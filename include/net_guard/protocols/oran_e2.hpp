// File: /home/kami/net-guard/include/net_guard/protocols/oran_e2.hpp

#pragma once

#include "net_guard/core/types.hpp"
#include <cstdint>
#include <span>

namespace net_guard::protocols {

class OranE2Validator {
public:
    static constexpr size_t MIN_E2AP_LEN = 8;

    [[nodiscard]] static GuardStatus validate_e2ap(
        std::span<const uint8_t> payload,
        bool allow_ric_control = false) noexcept;
};

} // namespace net_guard::protocols