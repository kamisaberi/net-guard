// File: /home/kami/net-guard/include/net_guard/protocols/pqc_tls.hpp

#pragma once

#include "net_guard/core/types.hpp"
#include <cstdint>
#include <span>

namespace net_guard::protocols {

class PqcTlsValidator {
public:
    // Recognized NIST ML-KEM / Hybrid NamedGroup IANA identifiers
    static constexpr uint16_t GROUP_MLKEM768        = 0x0201;
    static constexpr uint16_t GROUP_X25519_MLKEM768 = 0x11EC;
    static constexpr uint16_t GROUP_MLKEM1024       = 0x0202;

    [[nodiscard]] static GuardStatus validate_client_hello(
        std::span<const uint8_t> tls_record,
        bool require_pqc = true) noexcept;
};

} // namespace net_guard::protocols