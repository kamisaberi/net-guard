// File: /home/kami/net-guard/src/protocols/oran_e2.cpp

#include "net_guard/protocols/oran_e2.hpp"

namespace net_guard::protocols {

GuardStatus OranE2Validator::validate_e2ap(
    std::span<const uint8_t> payload,
    bool allow_ric_control) noexcept
{
    if (payload.size() < MIN_E2AP_LEN) {
        return GuardStatus::REJECT_MALFORMED_FRAME;
    }

    // E2AP PDU Choice (0 = InitiatingMessage) & ProcedureCode
    // ProcedureCode 4 = RICcontrol, 5 = RICindication, 10 = E2nodeConfigurationUpdate
    const uint8_t pdu_choice = payload[0];
    const uint8_t proc_code  = payload[1];

    if (pdu_choice == 0x00 && (proc_code == 4 || proc_code == 10) && !allow_ric_control) {
        return GuardStatus::REJECT_ORAN_ROGUE_XAPP_CONTROL;
    }

    return GuardStatus::PASS;
}

} // namespace net_guard::protocols