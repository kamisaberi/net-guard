// File: /home/kami/net-guard/include/net_guard/pipeline/guard_engine.hpp

#pragma once

#include "net_guard/core/types.hpp"
#include "net_guard/protocols/gtpu_pfcp.hpp"
#include "net_guard/protocols/goose_sv_l2.hpp"
#include "net_guard/protocols/itch_mdp.hpp"
#include "net_guard/protocols/oran_e2.hpp"
#include "net_guard/protocols/pqc_tls.hpp"
#include "net_guard/defense/slice_airgap.hpp"
#include <span>

namespace net_guard::pipeline {

class GuardEngine {
public:
    [[nodiscard]] GuardStatus process_gtpu(
        std::span<const uint8_t> udp_payload,
        const protocols::SlicePolicy& policy = {}) noexcept;

    [[nodiscard]] GuardStatus process_goose_l2(
        std::span<const uint8_t> l2_payload,
        uint16_t ethertype = protocols::GooseSvL2Validator::ETHERTYPE_GOOSE) noexcept;

    [[nodiscard]] GuardStatus process_itch_order(
        std::span<const uint8_t> payload,
        const protocols::PreTradeRiskLimits& limits = {}) noexcept;

    [[nodiscard]] GuardStatus process_oran_e2(
        std::span<const uint8_t> payload,
        bool allow_ric_control = false) noexcept;

    [[nodiscard]] GuardStatus process_pqc_tls(
        std::span<const uint8_t> tls_record,
        bool require_pqc = true) noexcept;

    [[nodiscard]] bool is_airgap_tripped() const noexcept { return airgap_.is_tripped(); }
    void reset_airgap() noexcept {
        airgap_.reset();
        goose_validator_.reset();
    }

private:
    protocols::GooseSvL2Validator goose_validator_;
    defense::SliceAirGap airgap_;
};

} // namespace net_guard::pipeline