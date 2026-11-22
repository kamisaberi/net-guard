// File: /home/kami/net-guard/include/net_guard/protocols/goose_sv_l2.hpp

#pragma once

#include "net_guard/core/types.hpp"
#include <cstdint>
#include <span>

namespace net_guard::protocols {

class GooseSvL2Validator {
public:
    static constexpr uint16_t ETHERTYPE_GOOSE = 0x88B8;
    static constexpr uint16_t ETHERTYPE_SV    = 0x88BA;
    static constexpr size_t MIN_GOOSE_PDU_LEN = 12;

    [[nodiscard]] GuardStatus validate_pdu(
        std::span<const uint8_t> l2_payload,
        uint16_t ethertype = ETHERTYPE_GOOSE) noexcept;

    void reset() noexcept { last_st_num_ = 0; last_sq_num_ = 0; initialized_ = false; }

private:
    uint32_t last_st_num_{0};
    uint32_t last_sq_num_{0};
    bool initialized_{false};
};

} // namespace net_guard::protocols