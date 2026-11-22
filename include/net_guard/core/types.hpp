// File: /home/kami/net-guard/include/net_guard/core/types.hpp

#pragma once

#include <cstdint>
#include <string_view>

namespace net_guard {

enum class GuardStatus : uint8_t {
    PASS = 0,
    REJECT_MALFORMED_FRAME,
    REJECT_GTPU_UNAUTHORIZED_TEID,
    REJECT_GTPU_SLICE_HOPPING,
    REJECT_GOOSE_SPOOFED_STNUM,
    REJECT_GOOSE_INVALID_APPID,
    REJECT_ORAN_ROGUE_XAPP_CONTROL,
    REJECT_ITCH_FAT_FINGER_QTY,
    REJECT_ITCH_PRICE_COLLAR,
    REJECT_PQC_CLASSICAL_DOWNGRADE,
    SLICE_AIRGAP_TRIPPED
};

[[nodiscard]] constexpr std::string_view status_to_string(GuardStatus st) noexcept {
    switch (st) {
        case GuardStatus::PASS:                           return "PASS";
        case GuardStatus::REJECT_MALFORMED_FRAME:         return "REJECT_MALFORMED_FRAME";
        case GuardStatus::REJECT_GTPU_UNAUTHORIZED_TEID:  return "REJECT_GTPU_UNAUTHORIZED_TEID";
        case GuardStatus::REJECT_GTPU_SLICE_HOPPING:      return "REJECT_GTPU_SLICE_HOPPING";
        case GuardStatus::REJECT_GOOSE_SPOOFED_STNUM:     return "REJECT_GOOSE_SPOOFED_STNUM";
        case GuardStatus::REJECT_GOOSE_INVALID_APPID:     return "REJECT_GOOSE_INVALID_APPID";
        case GuardStatus::REJECT_ORAN_ROGUE_XAPP_CONTROL: return "REJECT_ORAN_ROGUE_XAPP_CONTROL";
        case GuardStatus::REJECT_ITCH_FAT_FINGER_QTY:     return "REJECT_ITCH_FAT_FINGER_QTY";
        case GuardStatus::REJECT_ITCH_PRICE_COLLAR:       return "REJECT_ITCH_PRICE_COLLAR";
        case GuardStatus::REJECT_PQC_CLASSICAL_DOWNGRADE: return "REJECT_PQC_CLASSICAL_DOWNGRADE";
        case GuardStatus::SLICE_AIRGAP_TRIPPED:           return "SLICE_AIRGAP_TRIPPED";
    }
    return "UNKNOWN";
}

} // namespace net_guard