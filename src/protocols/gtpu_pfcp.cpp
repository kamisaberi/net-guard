// File: /home/kami/net-guard/src/protocols/gtpu_pfcp.cpp

#include "net_guard/protocols/gtpu_pfcp.hpp"

namespace net_guard::protocols {

static inline uint32_t read_be32(const uint8_t* p) noexcept {
    return (static_cast<uint32_t>(p[0]) << 24) |
           (static_cast<uint32_t>(p[1]) << 16) |
           (static_cast<uint32_t>(p[2]) << 8)  |
           static_cast<uint32_t>(p[3]);
}

static inline uint16_t read_be16(const uint8_t* p) noexcept {
    return (static_cast<uint16_t>(p[0]) << 8) | p[1];
}

GuardStatus GtpuPfcpValidator::validate_gtpu(
    std::span<const uint8_t> udp_payload,
    const SlicePolicy& policy) noexcept
{
    if (udp_payload.size() < MIN_GTPU_HDR_LEN) {
        return GuardStatus::REJECT_MALFORMED_FRAME;
    }

    const uint8_t flags = udp_payload[0];
    const uint8_t version = (flags >> 5) & 0x07;
    const uint8_t pt = (flags >> 4) & 0x01;
    const uint8_t msg_type = udp_payload[1];

    // GTPv1 (version == 1, PT == 1)
    if (version != 1 || pt != 1) {
        return GuardStatus::REJECT_MALFORMED_FRAME;
    }

    // Echo Request/Response pass
    if (msg_type == 1 || msg_type == 2) {
        return GuardStatus::PASS;
    }

    const uint32_t teid = read_be32(&udp_payload[4]);
    const bool is_urllc = (teid >= policy.urllc_teid_min && teid <= policy.urllc_teid_max);
    const bool is_embb  = (teid >= policy.embb_teid_min && teid <= policy.embb_teid_max);

    if (!is_urllc && !is_embb) {
        return GuardStatus::REJECT_GTPU_UNAUTHORIZED_TEID;
    }

    // Compute GTP-U header length (optional E/S/PN flags add 4 bytes)
    size_t inner_off = 8;
    if ((flags & 0x07) != 0) {
        inner_off += 4;
    }

    // Decapsulate inner IPv4 packet inside G-PDU (msg_type == 0xFF)
    if (msg_type == 0xFF && udp_payload.size() >= inner_off + 24) {
        const uint8_t* ip = &udp_payload[inner_off];
        if ((ip[0] >> 4) == 4) {
            const size_t ihl = static_cast<size_t>(ip[0] & 0x0F) * 4U;
            const uint8_t proto = ip[9];
            if (ihl >= 20 && udp_payload.size() >= inner_off + ihl + 4 && (proto == 6 || proto == 17)) {
                const uint16_t dst_port = read_be16(ip + ihl + 2);
                const bool is_ot_port = (dst_port == 502 || dst_port == 102 || dst_port == 2404);

                // Strictly block eMBB slice-hopping into OT/ICS control ports (matches configs/5g_slices.yaml)
                if (is_embb && is_ot_port) {
                    return GuardStatus::REJECT_GTPU_SLICE_HOPPING;
                }
            }
        }
    }

    return GuardStatus::PASS;
}

} // namespace net_guard::protocols