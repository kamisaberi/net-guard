// File: /home/kami/net-guard/tools/sentinel_plugin/net_guard_plugin.cpp

#include "sentinel/sdk/abi.hpp"
#include "net_guard/net_guard.hpp"
#include <algorithm>
#include <cstring>
#include <memory>
#include <span>
#include <string>

static const SentinelHostInterface *g_host = nullptr;
static std::unique_ptr<net_guard::pipeline::GuardEngine> g_guard_engine;

static int plugin_init(const SentinelHostInterface *host_iface)
{
    g_host = host_iface;
    g_guard_engine = std::make_unique<net_guard::pipeline::GuardEngine>();
    net_guard::core::TscClock::calibrate();

    if (g_host && g_host->log_message)
    {
        g_host->log_message(1, "NetGuard",
                            "5G UPF, O-RAN E2, IEC 61850 L2, FinTech ITCH & PQC Plugin Initialized (< 120 ns SLA).");
    }
    return 0;
}

static void plugin_shutdown()
{
    g_guard_engine.reset();
    g_host = nullptr;
}

static SentinelDissectorResult plugin_dissect(const SentinelRawPacket *packet)
{
    SentinelDissectorResult res{};
    res.verdict = SENTINEL_VERDICT_PASS;
    res.threat_score = 0;
    res.rule_id = 0;
    res.semantic_tags = 0;
    res.threat_name[0] = '\0';

    if (!packet || !packet->data || packet->length < 8 || !g_guard_engine)
    {
        return res;
    }

    net_guard::GuardStatus st = net_guard::GuardStatus::PASS;
    uint32_t matched_rule_id = 0;

    // -------------------------------------------------------------------------
    // BRANCH A: NON-IP LAYER-2 FRAMES (IEC 61850 GOOSE 0x88B8 / SV 0x88BA)
    // -------------------------------------------------------------------------
    if (packet->network_proto == net_guard::protocols::GooseSvL2Validator::ETHERTYPE_GOOSE ||
        packet->network_proto == net_guard::protocols::GooseSvL2Validator::ETHERTYPE_SV)
    {
        const uint8_t *pdu = packet->data;
        size_t pdu_len = packet->length;

        if (packet->length >= 26)
        {
            const uint16_t et12 = (static_cast<uint16_t>(packet->data[12]) << 8) | packet->data[13];
            if (et12 == packet->network_proto)
            {
                pdu = packet->data + 14;
                pdu_len = packet->length - 14;
            }
            else if (et12 == 0x8100 && packet->length >= 30)
            {
                pdu = packet->data + 18;
                pdu_len = packet->length - 18;
            }
        }

        st = g_guard_engine->process_goose_l2(std::span<const uint8_t>(pdu, pdu_len), packet->network_proto);
        matched_rule_id = packet->network_proto;
    }
    else
    {
        // ---------------------------------------------------------------------
        // BRANCH B: IPv4 TCP / UDP / SCTP FRAMES (or raw test payloads)
        // ---------------------------------------------------------------------
        const uint8_t *payload_ptr = packet->data;
        size_t payload_len = packet->length;
        uint16_t dst_port = 0;
        uint8_t l4_proto = packet->transport_proto;

        // Strip Ethernet + IPv4 + L4 header when full L2 frame is supplied
        if (packet->length >= 34 && packet->data[12] == 0x08 && packet->data[13] == 0x00)
        {
            const size_t ihl = static_cast<size_t>(packet->data[14] & 0x0F) * 4U;
            const size_t l4_off = 14U + ihl;
            l4_proto = packet->data[23];
            if (ihl >= 20 && l4_off + 8U <= packet->length)
            {
                dst_port = (static_cast<uint16_t>(packet->data[l4_off + 2]) << 8) |
                           packet->data[l4_off + 3];
                if (l4_proto == 17)
                {
                    payload_ptr = packet->data + l4_off + 8U;
                    payload_len = packet->length - (l4_off + 8U);
                }
                else if (l4_proto == 6 && l4_off + 20U <= packet->length)
                {
                    const size_t tcp_hlen = static_cast<size_t>((packet->data[l4_off + 12] >> 4) & 0x0F) * 4U;
                    const size_t off = l4_off + tcp_hlen;
                    if (tcp_hlen < 20 || off >= packet->length)
                    {
                        return res;
                    }
                    payload_ptr = packet->data + off;
                    payload_len = packet->length - off;
                }
                else if (l4_proto == 132 && l4_off + 12U < packet->length)
                {
                    // SCTP Common Header (12 bytes)
                    payload_ptr = packet->data + l4_off + 12U;
                    payload_len = packet->length - (l4_off + 12U);
                }
            }
        }

        std::span<const uint8_t> frame(payload_ptr, payload_len);

        const bool is_gtpu = (dst_port == 2152) ||
                             (dst_port == 0 && l4_proto == 17 && payload_len >= 8 &&
                              (payload_ptr[0] & 0xF0) == 0x30);

        const bool is_oran = (l4_proto == 132 && (dst_port == 36422 || dst_port == 0));

        const bool is_itch = (dst_port == 9001) ||
                             (dst_port == 0 && l4_proto == 6 && payload_len >= 36 &&
                              (payload_ptr[0] == 'A' || payload_ptr[0] == 'F' || payload_ptr[0] == 'O'));

        const bool is_pqc_tls = (dst_port == 443) ||
                                (dst_port == 0 && payload_len >= 12 &&
                                 payload_ptr[0] == 0x16 && payload_ptr[1] == 0x03 && payload_ptr[5] == 0x01);

        if (is_gtpu)
        {
            st = g_guard_engine->process_gtpu(frame);
            matched_rule_id = 0x2152;
        }
        else if (is_oran)
        {
            st = g_guard_engine->process_oran_e2(frame);
            matched_rule_id = 0x3642;
        }
        else if (is_itch)
        {
            st = g_guard_engine->process_itch_order(frame);
            matched_rule_id = 0x4954;
        }
        else if (is_pqc_tls)
        {
            st = g_guard_engine->process_pqc_tls(frame);
            matched_rule_id = 0x0443;
        }
    }

    if (st != net_guard::GuardStatus::PASS)
    {
        res.verdict = SENTINEL_VERDICT_KERNEL_DROP;
        res.threat_score = 990;
        res.rule_id = matched_rule_id;
        std::strncpy(res.threat_name, net_guard::status_to_string(st).data(), sizeof(res.threat_name) - 1);

        if (g_host)
        {
            if (g_host->log_message)
            {
                std::string msg = "NET-GUARD TRIPWIRE: Dropped frame (" +
                                  std::string(res.threat_name) + ")";
                g_host->log_message(3, "NetGuard", msg.c_str());
            }
            if (g_host->emit_metric_counter)
            {
                g_host->emit_metric_counter("net_guard_wire_drops", 1);
            }
        }
        return res;
    }

    res.threat_score = static_cast<uint32_t>(std::min<size_t>(packet->length * 2U, 200U));
    return res;
}

static const SentinelPluginDescriptor g_descriptor = {
    .magic = SENTINEL_SDK_MAGIC,
    .sdk_version_major = SENTINEL_SDK_VERSION_MAJOR,
    .sdk_version_minor = SENTINEL_SDK_VERSION_MINOR,
    .tier = SENTINEL_TIER_NATIVE_CPP,
    .plugin_id = "org.aryorithm.plugin.net_guard",
    .plugin_name = "5G UPF, O-RAN E2, IEC 61850 L2, FinTech ITCH & PQC Defense Engine",
    .plugin_version = "1.0.0",
    .target_protocol = "GTPU_GOOSE_ORAN_ITCH_PQC",
    .default_port = 2152,
    .reserved = 0,
    .init_fn = plugin_init,
    .shutdown_fn = plugin_shutdown,
    .dissect_fn = plugin_dissect};

extern "C" ARY_SDK_API const SentinelPluginDescriptor *sentinel_plugin_get_descriptor(void)
{
    return &g_descriptor;
}