// File: /home/kami/net-guard/benchmarks/bench_latency.cpp

#include "net_guard/net_guard.hpp"
#include <algorithm>
#include <iostream>
#include <numeric>
#include <vector>

int main()
{
    net_guard::core::TscClock::calibrate();
    net_guard::pipeline::GuardEngine engine;

    constexpr size_t ITERATIONS = 100'000;
    std::vector<double> samples_ns;
    samples_ns.reserve(ITERATIONS);

    std::vector<uint8_t> gtpu(32, 0);
    gtpu[0] = 0x30; gtpu[1] = 0xFF; gtpu[3] = 24;
    gtpu[6] = 0x01; gtpu[7] = 0x00;
    gtpu[8] = 0x45; gtpu[17] = 6;
    gtpu[30] = 0x01; gtpu[31] = 0xF6;

    for (size_t i = 0; i < ITERATIONS; ++i)
    {
        const uint64_t t0 = net_guard::core::TscClock::rdtsc();
        const auto st = engine.process_gtpu(gtpu);
        const uint64_t t1 = net_guard::core::TscClock::rdtsc();
        if (st == net_guard::GuardStatus::PASS)
        {
            samples_ns.push_back(net_guard::core::TscClock::cycles_to_nanoseconds(t1 - t0));
        }
    }

    std::sort(samples_ns.begin(), samples_ns.end());
    const double sum = std::accumulate(samples_ns.begin(), samples_ns.end(), 0.0);
    const double mean = sum / static_cast<double>(samples_ns.size());
    const double p99 = samples_ns[(samples_ns.size() * 99) / 100];

    std::cout << "================================================================================\n";
    std::cout << "  NET-GUARD HARDWARE LATENCY BENCHMARK (100,000 5G GTP-U FRAMES)\n";
    std::cout << "================================================================================\n";
    std::cout << "  TSC Frequency : " << net_guard::core::TscClock::ghz() << " GHz\n";
    std::cout << "  Mean Latency  : " << mean << " ns\n";
    std::cout << "  P99 Latency   : " << p99 << " ns\n";
    std::cout << "  SLA Target    : < 120.0 ns -> " << (mean < 120.0 ? "PASS" : "FAIL") << "\n";
    std::cout << "================================================================================\n";
    return (mean < 120.0) ? 0 : 1;
}