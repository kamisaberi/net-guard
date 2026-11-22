// File: /home/kami/net-guard/tests/test_sentinel_plugin.cpp

#include "sentinel/sdk/abi.hpp"
#include <cassert>
#include <dlfcn.h>
#include <filesystem>
#include <iostream>
#include <vector>

static int g_logs = 0;
static int g_metrics = 0;

static void mock_log(int level, const char *sender, const char *msg)
{
    std::cout << "  [HOST_LOG level=" << level << " sender=" << sender << "] " << msg << "\n";
    g_logs++;
}

static void mock_metric(const char *name, uint64_t delta)
{
    std::cout << "  [HOST_METRIC] " << name << " (+" << delta << ")\n";
    g_metrics += static_cast<int>(delta);
}

int main()
{
    std::cout << "[TEST] blackbox-sentinel Native Net-Guard Plugin ABI Integration...\n";

    const char *plugin_path = std::filesystem::exists("./net_guard_dissector_plugin.so")
                                  ? "./net_guard_dissector_plugin.so"
                                  : "/usr/local/lib/blackbox/net_guard_dissector_plugin.so";

    void *handle = dlopen(plugin_path, RTLD_NOW | RTLD_LOCAL);
    if (!handle)
    {
        std::cerr << "[-] dlopen failed: " << dlerror() << "\n";
        return 1;
    }

    using GetDescFn = const SentinelPluginDescriptor *(*)(void);
    auto get_desc = reinterpret_cast<GetDescFn>(dlsym(handle, "sentinel_plugin_get_descriptor"));
    assert(get_desc != nullptr);
    const SentinelPluginDescriptor *desc = get_desc();
    assert(desc && desc->magic == SENTINEL_SDK_MAGIC);
    assert(std::string(desc->plugin_id) == "org.aryorithm.plugin.net_guard");

    SentinelHostInterface host{};
    host.engine_version = 1;
    host.log_message = mock_log;
    host.emit_metric_counter = mock_metric;
    assert(desc->init_fn(&host) == 0);
    assert(g_logs >= 1);

    // 1. Valid Non-IP L2 GOOSE frame (0x88B8) -> PASS
    std::vector<uint8_t> goose_ok(16, 0);
    goose_ok[1] = 0x01; goose_ok[3] = 16; goose_ok[11] = 1; goose_ok[15] = 1;
    SentinelRawPacket p_goose{};
    p_goose.data = goose_ok.data();
    p_goose.length = goose_ok.size();
    p_goose.network_proto = 0x88B8;
    p_goose.transport_proto = 0;
    assert(desc->dissect_fn(&p_goose).verdict == SENTINEL_VERDICT_PASS);

    // 2. eMBB GTP-U Slice-Hopping to Modbus 502 -> KERNEL_DROP
    std::vector<uint8_t> embb_hop(32, 0);
    embb_hop[0] = 0x30; embb_hop[1] = 0xFF; embb_hop[3] = 24;
    embb_hop[6] = 0x02; embb_hop[7] = 0x00;
    embb_hop[8] = 0x45; embb_hop[17] = 6;
    embb_hop[30] = 0x01; embb_hop[31] = 0xF6;
    SentinelRawPacket p_hop{};
    p_hop.data = embb_hop.data();
    p_hop.length = embb_hop.size();
    p_hop.network_proto = 0x0800;
    p_hop.transport_proto = 17;

    SentinelDissectorResult r2 = desc->dissect_fn(&p_hop);
    assert(r2.verdict == SENTINEL_VERDICT_KERNEL_DROP);
    assert(r2.rule_id == 0x2152);
    assert(g_metrics >= 1);

    desc->shutdown_fn();
    dlclose(handle);
    std::cout << ">>> RESULT: SUCCESS (Net-Guard ABI Plugin + Non-IP L2 Verified) <<<\n";
    return 0;
}