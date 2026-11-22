// File: /home/kami/net-guard/src/protocols/itch_mdp.cpp

#include "net_guard/protocols/itch_mdp.hpp"

namespace net_guard::protocols {

static inline uint32_t read_be32(const uint8_t* p) noexcept {
    return (static_cast<uint32_t>(p[0]) << 24) |
           (static_cast<uint32_t>(p[1]) << 16) |
           (static_cast<uint32_t>(p[2]) << 8)  |
           static_cast<uint32_t>(p[3]);
}

GuardStatus ItchMdpValidator::validate_order(
    std::span<const uint8_t> payload,
    const PreTradeRiskLimits& limits) noexcept
{
    if (payload.size() < MIN_ITCH_ORDER_LEN) {
        return GuardStatus::REJECT_MALFORMED_FRAME;
    }

    const uint8_t msg_type = payload[0];
    // ITCH 'A' (Add Order) or 'F' (Add Order w/ MPID) or OUCH 'O' (Enter Order)
    if (msg_type != 'A' && msg_type != 'F' && msg_type != 'O') {
        return GuardStatus::PASS;
    }

    // Shares @ offset 20 (4 bytes BE), Price @ offset 32 (4 bytes BE, 1e-4 scaled)
    const uint32_t shares = read_be32(&payload[20]);
    const uint32_t price  = read_be32(&payload[32]);

    if (shares == 0 || shares > limits.max_shares) {
        return GuardStatus::REJECT_ITCH_FAT_FINGER_QTY;
    }
    if (price < limits.min_price_ticks || price > limits.max_price_ticks) {
        return GuardStatus::REJECT_ITCH_PRICE_COLLAR;
    }

    return GuardStatus::PASS;
}

} // namespace net_guard::protocols