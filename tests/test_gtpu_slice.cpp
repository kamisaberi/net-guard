// File: /home/kami/net-guard/tests/test_gtpu_slice.cpp

#include "net_guard/pipeline/guard_engine.hpp"
#include <cassert>
#include <iostream>
#include <vector>

int main()
{
    std::cout << "[TEST] 3GPP 5G GTP-U Slice-Hopping & TEID Enforcement... ";
    net_guard::pipeline::GuardEngine engine;

    // 1. URLLC TEID 0x00000100 -> Modbus 502 (PASS)
    std::vector<uint8_t> urllc(32, 0);
    urllc[0] = 0x30; urllc[1] = 0xFF; urllc[3] = 24;
    urllc[6] = 0x01; urllc[7] = 0x00;
    urllc[8] = 0x45; urllc[17] = 6;
    urllc[30] = 0x01; urllc[31] = 0xF6;
    assert(engine.process_gtpu(urllc) == net_guard::GuardStatus::PASS);

    // 2. eMBB TEID 0x00000200 -> HTTPS 443 (PASS)
    std::vector<uint8_t> embb_ok(32, 0);
    embb_ok[0] = 0x30; embb_ok[1] = 0xFF; embb_ok[3] = 24;
    embb_ok[6] = 0x02; embb_ok[7] = 0x00;
    embb_ok[8] = 0x45; embb_ok[17] = 6;
    embb_ok[30] = 0x01; embb_ok[31] = 0xBB;
    assert(engine.process_gtpu(embb_ok) == net_guard::GuardStatus::PASS);

    // 3. eMBB TEID 0x00000200 hopping to Modbus 502 -> DROP
    std::vector<uint8_t> embb_hop = urllc;
    embb_hop[6] = 0x02; embb_hop[7] = 0x00;
    assert(engine.process_gtpu(embb_hop) == net_guard::GuardStatus::REJECT_GTPU_SLICE_HOPPING);

    // 4. Rogue TEID 0x00009999 -> DROP
    engine.reset_airgap();
    std::vector<uint8_t> rogue = urllc;
    rogue[6] = 0x99; rogue[7] = 0x99;
    assert(engine.process_gtpu(rogue) == net_guard::GuardStatus::REJECT_GTPU_UNAUTHORIZED_TEID);

    std::cout << "PASSED\n";
    return 0;
}