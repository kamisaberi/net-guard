// File: /home/kami/net-guard/src/pipeline/guard_engine.cpp

#include "net_guard/pipeline/guard_engine.hpp"

namespace net_guard::pipeline {

GuardStatus GuardEngine::process_gtpu(
    std::span<const uint8_t> udp_payload,
    const protocols::SlicePolicy& policy) noexcept
{
    if (airgap_.is_tripped()) return GuardStatus::SLICE_AIRGAP_TRIPPED;
    const GuardStatus st = protocols::GtpuPfcpValidator::validate_gtpu(udp_payload, policy);
    if (st != GuardStatus::PASS) airgap_.trip();
    return st;
}

GuardStatus GuardEngine::process_goose_l2(
    std::span<const uint8_t> l2_payload,
    uint16_t ethertype) noexcept
{
    if (airgap_.is_tripped()) return GuardStatus::SLICE_AIRGAP_TRIPPED;
    const GuardStatus st = goose_validator_.validate_pdu(l2_payload, ethertype);
    if (st != GuardStatus::PASS) airgap_.trip();
    return st;
}

GuardStatus GuardEngine::process_itch_order(
    std::span<const uint8_t> payload,
    const protocols::PreTradeRiskLimits& limits) noexcept
{
    if (airgap_.is_tripped()) return GuardStatus::SLICE_AIRGAP_TRIPPED;
    const GuardStatus st = protocols::ItchMdpValidator::validate_order(payload, limits);
    if (st != GuardStatus::PASS) airgap_.trip();
    return st;
}

GuardStatus GuardEngine::process_oran_e2(
    std::span<const uint8_t> payload,
    bool allow_ric_control) noexcept
{
    if (airgap_.is_tripped()) return GuardStatus::SLICE_AIRGAP_TRIPPED;
    const GuardStatus st = protocols::OranE2Validator::validate_e2ap(payload, allow_ric_control);
    if (st != GuardStatus::PASS) airgap_.trip();
    return st;
}

GuardStatus GuardEngine::process_pqc_tls(
    std::span<const uint8_t> tls_record,
    bool require_pqc) noexcept
{
    if (airgap_.is_tripped()) return GuardStatus::SLICE_AIRGAP_TRIPPED;
    const GuardStatus st = protocols::PqcTlsValidator::validate_client_hello(tls_record, require_pqc);
    if (st != GuardStatus::PASS) airgap_.trip();
    return st;
}

} // namespace net_guard::pipeline