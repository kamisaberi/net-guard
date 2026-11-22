// File: /home/kami/net-guard/include/net_guard/protocols/gtpu_pfcp.hpp

#pragma once

#include "net_guard/core/types.hpp"
#include <cstdint>
#include <span>

namespace net_guard::protocols {

struct SlicePolicy {
    uint32_t urllc_teid_min{0x00000100};
    uint32_t urllc_teid_max{0x000001FF};
    uint32_t embb_teid_min{0x00000200};
    uint32_t embb_teid_max{0x000002FF};
};

class GtpuPfcpValidator {
public:
    static constexpr size_t MIN_GTPU_HDR_LEN = 8;

    [[nodiscard]] static GuardStatus validate_gtpu(
        std::span<const uint8_t> udp_payload,
        const SlicePolicy& policy = {}) noexcept;
};

} // namespace net_guard::protocols