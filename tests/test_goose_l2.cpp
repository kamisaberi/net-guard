// File: /home/kami/net-guard/tests/test_goose_l2.cpp

#include "net_guard/pipeline/guard_engine.hpp"
#include <cassert>
#include <iostream>
#include <vector>

int main()
{
    std::cout << "[TEST] Non-IP Layer-2 IEC 61850 GOOSE & SV Anti-Spoof... ";
    net_guard::pipeline::GuardEngine engine;

    std::vector<uint8_t> g1(16, 0);
    g1[0] = 0x00; g1[1] = 0x01; g1[3] = 16; // APPID 0x0001
    g1[11] = 10; g1[15] = 1;                // stNum=10, sqNum=1
    assert(engine.process_goose_l2(g1, 0x88B8) == net_guard::GuardStatus::PASS);

    // Valid monotonic increment
    g1[15] = 2;
    assert(engine.process_goose_l2(g1, 0x88B8) == net_guard::GuardStatus::PASS);

    // Spoofed stNum regression (10 -> 4)
    g1[11] = 4; g1[15] = 1;
    assert(engine.process_goose_l2(g1, 0x88B8) == net_guard::GuardStatus::REJECT_GOOSE_SPOOFED_STNUM);

    // Invalid GOOSE APPID (> 0x3FFF)
    engine.reset_airgap();
    g1[0] = 0x90; g1[1] = 0x00;
    assert(engine.process_goose_l2(g1, 0x88B8) == net_guard::GuardStatus::REJECT_GOOSE_INVALID_APPID);

    std::cout << "PASSED\n";
    return 0;
}