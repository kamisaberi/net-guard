// File: /home/kami/net-guard/include/net_guard/protocols/itch_mdp.hpp

#pragma once

#include "net_guard/core/types.hpp"
#include <cstdint>
#include <span>

namespace net_guard::protocols {

struct PreTradeRiskLimits {
    uint32_t max_shares{10000};
    uint32_t min_price_ticks{10000};   // $1.0000 (1e-4 scale)
    uint32_t max_price_ticks{50000000}; // $5000.0000
};

class ItchMdpValidator {
public:
    // NASDAQ ITCH 5.0 'A' (Add Order) / OUCH 'O' (Enter Order) minimum frame
    static constexpr size_t MIN_ITCH_ORDER_LEN = 36;

    [[nodiscard]] static GuardStatus validate_order(
        std::span<const uint8_t> payload,
        const PreTradeRiskLimits& limits = {}) noexcept;
};

} // namespace net_guard::protocols