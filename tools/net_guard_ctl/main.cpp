// File: /home/kami/net-guard/tools/net_guard_ctl/main.cpp

#include "net_guard/net_guard.hpp"
#include <iostream>
#include <vector>

int main()
{
    net_guard::core::TscClock::calibrate();
    net_guard::pipeline::GuardEngine engine;

    std::vector<uint8_t> gtpu(32, 0);
    gtpu[0] = 0x30; gtpu[1] = 0xFF; gtpu[3] = 24;
    gtpu[6] = 0x01; gtpu[7] = 0x00;
    gtpu[8] = 0x45; gtpu[17] = 6;
    gtpu[30] = 0x01; gtpu[31] = 0xF6;

    const auto st = engine.process_gtpu(gtpu);
    std::cout << "[net-guard-ctl] TSC Clock : " << net_guard::core::TscClock::ghz() << " GHz\n";
    std::cout << "[net-guard-ctl] Self-Test : " << net_guard::status_to_string(st) << "\n";
    return (st == net_guard::GuardStatus::PASS) ? 0 : 1;
}