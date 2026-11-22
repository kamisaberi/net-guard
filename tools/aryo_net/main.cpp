// File: /home/kami/net-guard/tools/aryo_net/main.cpp

#include "net_guard/net_guard.hpp"
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

static void print_help()
{
    std::cout << R"(================================================================================
ARYO NET: 5G UPF, O-RAN, RAW L2 GOOSE, FINTECH ITCH & PQC DEFENSE (net-guard)
================================================================================
Usage:
  aryo net <action>

Actions:
  bench           Run 100,000-sample GTP-U slice decapsulation benchmark (< 120 ns)
  verify-gtpu     Verify 3GPP GTP-U URLLC vs eMBB slice-hopping enforcement
  verify-goose    Verify Non-IP L2 IEC 61850 GOOSE (0x88B8) stNum anti-spoof
  verify-itch     Verify NASDAQ ITCH pre-trade fat-finger order drop
  verify-oran     Verify O-RAN E2AP rogue xApp control blocking
  verify-pqc      Verify TLS 1.3 ML-KEM post-quantum enforcement
  airgap-status   Inspect real-time slice air-gap latch state
  telco-up        Deploy telco_5g_fintech_grid simulation mesh
  telco-down      Teardown telco_5g_fintech_grid simulation mesh
  telco-status    Show running telco_5g_fintech_grid containers
  --plugin-desc   Print single-line extension metadata for master CLI
)" << std::endl;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        print_help();
        return 0;
    }

    const std::string action = argv[1];

    if (action == "--plugin-desc")
    {
        std::cout << "[ext. net]              aryo net (bench, verify-gtpu, verify-goose, verify-itch, telco-up) [5G UPF, Raw L2 & FinTech Guard]";
        return 0;
    }

    if (action == "-h" || action == "--help" || action == "help")
    {
        print_help();
        return 0;
    }

    if (action == "telco-up")
    {
        return std::system("python3 /home/kami/sentinel-matrix/tools/scenario_engine.py load /home/kami/net-guard/scenarios/telco_5g_fintech_grid");
    }
    if (action == "telco-down")
    {
        return std::system("python3 /home/kami/sentinel-matrix/tools/scenario_engine.py down --name telco_5g_fintech_grid");
    }
    if (action == "telco-status")
    {
        return std::system("python3 /home/kami/sentinel-matrix/tools/scenario_engine.py status --name telco_5g_fintech_grid");
    }

    net_guard::core::TscClock::calibrate();
    net_guard::pipeline::GuardEngine engine;

    if (action == "bench")
    {
        constexpr size_t ITERATIONS = 100'000;
        std::vector<double> lat_ns;
        lat_ns.reserve(ITERATIONS);

        // Valid URLLC GTP-U G-PDU (TEID 0x00000100) carrying inner IPv4 TCP dst port 502
        std::vector<uint8_t> gtpu(32, 0);
        gtpu[0] = 0x30; gtpu[1] = 0xFF; gtpu[3] = 24;
        gtpu[6] = 0x01; gtpu[7] = 0x00; // TEID = 0x00000100 (URLLC)
        gtpu[8] = 0x45; gtpu[17] = 6;   // Inner IPv4 TCP
        gtpu[30] = 0x01; gtpu[31] = 0xF6; // Inner dst port 502

        for (size_t i = 0; i < ITERATIONS; ++i)
        {
            const uint64_t t0 = net_guard::core::TscClock::rdtsc();
            const auto st = engine.process_gtpu(gtpu);
            const uint64_t t1 = net_guard::core::TscClock::rdtsc();
            if (st == net_guard::GuardStatus::PASS)
            {
                lat_ns.push_back(net_guard::core::TscClock::cycles_to_nanoseconds(t1 - t0));
            }
        }
        double total = 0.0;
        for (double v : lat_ns) total += v;
        std::cout << "[aryo-net] Mean Latency (100k GTP-U frames): " << (total / lat_ns.size()) << " ns\n";
        return 0;
    }

    if (action == "verify-gtpu")
    {
        std::vector<uint8_t> embb_hop(32, 0);
        embb_hop[0] = 0x30; embb_hop[1] = 0xFF; embb_hop[3] = 24;
        embb_hop[6] = 0x02; embb_hop[7] = 0x00; // TEID = 0x00000200 (eMBB)
        embb_hop[8] = 0x45; embb_hop[17] = 6;
        embb_hop[30] = 0x01; embb_hop[31] = 0xF6; // Hopping to Modbus 502!
        const auto st = engine.process_gtpu(embb_hop);
        std::cout << "[aryo-net] eMBB Slice-Hopping Result: " << net_guard::status_to_string(st) << "\n";
        return (st == net_guard::GuardStatus::REJECT_GTPU_SLICE_HOPPING) ? 0 : 1;
    }

    if (action == "verify-goose")
    {
        std::vector<uint8_t> g1(16, 0);
        g1[1] = 0x01; g1[3] = 16; g1[11] = 5; g1[15] = 1; // stNum=5, sqNum=1
        auto st1 = engine.process_goose_l2(g1, 0x88B8);
        std::vector<uint8_t> g_spoof(16, 0);
        g_spoof[1] = 0x01; g_spoof[3] = 16; g_spoof[11] = 3; g_spoof[15] = 1; // stNum regression!
        auto st2 = engine.process_goose_l2(g_spoof, 0x88B8);
        std::cout << "[aryo-net] GOOSE Nominal: " << net_guard::status_to_string(st1)
                  << " | Spoofed stNum: " << net_guard::status_to_string(st2) << "\n";
        return (st1 == net_guard::GuardStatus::PASS && st2 == net_guard::GuardStatus::REJECT_GOOSE_SPOOFED_STNUM) ? 0 : 1;
    }

    if (action == "verify-itch")
    {
        std::vector<uint8_t> itch(36, 0);
        itch[0] = 'A';
        itch[21] = 0x07; itch[22] = 0xA1; itch[23] = 0x20; // 500,000 shares
        itch[33] = 0x0F; itch[34] = 0x42; itch[35] = 0x40; // $100.00
        const auto st = engine.process_itch_order(itch);
        std::cout << "[aryo-net] ITCH Fat-Finger (500k shares): " << net_guard::status_to_string(st) << "\n";
        return (st == net_guard::GuardStatus::REJECT_ITCH_FAT_FINGER_QTY) ? 0 : 1;
    }

    if (action == "verify-oran")
    {
        const std::vector<uint8_t> e2_ctrl = {0x00, 0x04, 0x00, 0x10, 0x01, 0x02, 0x03, 0x04};
        const auto st = engine.process_oran_e2(e2_ctrl);
        std::cout << "[aryo-net] O-RAN Rogue xApp Control: " << net_guard::status_to_string(st) << "\n";
        return (st == net_guard::GuardStatus::REJECT_ORAN_ROGUE_XAPP_CONTROL) ? 0 : 1;
    }

    if (action == "verify-pqc")
    {
        std::vector<uint8_t> ch(16, 0);
        ch[0] = 0x16; ch[1] = 0x03; ch[2] = 0x03; ch[5] = 0x01; // No ML-KEM group
        const auto st = engine.process_pqc_tls(ch);
        std::cout << "[aryo-net] TLS Classical Downgrade: " << net_guard::status_to_string(st) << "\n";
        return (st == net_guard::GuardStatus::REJECT_PQC_CLASSICAL_DOWNGRADE) ? 0 : 1;
    }

    if (action == "airgap-status")
    {
        std::cout << "[aryo-net] Slice Air-Gap Tripped: " << (engine.is_airgap_tripped() ? "YES" : "NO") << "\n";
        return 0;
    }

    std::cerr << "[-] Unknown action: " << action << "\n";
    return 1;
}