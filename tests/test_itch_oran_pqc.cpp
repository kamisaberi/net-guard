// File: /home/kami/net-guard/tests/test_itch_oran_pqc.cpp

#include "net_guard/pipeline/guard_engine.hpp"
#include <cassert>
#include <iostream>
#include <vector>

int main()
{
    std::cout << "[TEST] ITCH Pre-Trade Collar, O-RAN E2 & PQC TLS 1.3... ";
    net_guard::pipeline::GuardEngine engine;

    // 1. ITCH valid order (100 shares @ $100.00)
    std::vector<uint8_t> itch(36, 0);
    itch[0] = 'A';
    itch[23] = 100;
    itch[33] = 0x0F; itch[34] = 0x42; itch[35] = 0x40;
    assert(engine.process_itch_order(itch) == net_guard::GuardStatus::PASS);

    // 2. ITCH fat-finger (500,000 shares)
    itch[21] = 0x07; itch[22] = 0xA1; itch[23] = 0x20;
    assert(engine.process_itch_order(itch) == net_guard::GuardStatus::REJECT_ITCH_FAT_FINGER_QTY);

    // 3. O-RAN E2 Indication (PASS) vs Rogue Control (DROP)
    engine.reset_airgap();
    assert(engine.process_oran_e2(std::vector<uint8_t>{0x00, 0x05, 0, 8, 0, 0, 0, 0}) == net_guard::GuardStatus::PASS);
    assert(engine.process_oran_e2(std::vector<uint8_t>{0x00, 0x04, 0, 8, 0, 0, 0, 0}) == net_guard::GuardStatus::REJECT_ORAN_ROGUE_XAPP_CONTROL);

    // 4. TLS 1.3 X25519MLKEM768 (0x11EC) vs Classical Downgrade
    engine.reset_airgap();
    std::vector<uint8_t> pqc_ok = {0x16, 0x03, 0x03, 0x00, 0x10, 0x01, 0x00, 0x00, 0x0C, 0x11, 0xEC, 0x00};
    assert(engine.process_pqc_tls(pqc_ok) == net_guard::GuardStatus::PASS);
    std::vector<uint8_t> pqc_bad = {0x16, 0x03, 0x03, 0x00, 0x10, 0x01, 0x00, 0x00, 0x0C, 0x00, 0x1D, 0x00};
    assert(engine.process_pqc_tls(pqc_bad) == net_guard::GuardStatus::REJECT_PQC_CLASSICAL_DOWNGRADE);

    std::cout << "PASSED\n";
    return 0;
}