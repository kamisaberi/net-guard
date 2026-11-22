// File: /home/kami/net-guard/src/protocols/goose_sv_l2.cpp

#include "net_guard/protocols/goose_sv_l2.hpp"

namespace net_guard::protocols {

static inline uint16_t read_be16(const uint8_t* p) noexcept {
    return (static_cast<uint16_t>(p[0]) << 8) | p[1];
}

static inline uint32_t read_be32(const uint8_t* p) noexcept {
    return (static_cast<uint32_t>(p[0]) << 24) |
           (static_cast<uint32_t>(p[1]) << 16) |
           (static_cast<uint32_t>(p[2]) << 8)  |
           static_cast<uint32_t>(p[3]);
}

GuardStatus GooseSvL2Validator::validate_pdu(
    std::span<const uint8_t> l2_payload,
    uint16_t ethertype) noexcept
{
    if (l2_payload.size() < MIN_GOOSE_PDU_LEN) {
        return GuardStatus::REJECT_MALFORMED_FRAME;
    }

    const uint16_t appid  = read_be16(&l2_payload[0]);
    const uint16_t apdu_len = read_be16(&l2_payload[2]);

    if (apdu_len < 8 || apdu_len > l2_payload.size()) {
        return GuardStatus::REJECT_MALFORMED_FRAME;
    }

    // IEC 61850 standard APPID ranges: GOOSE 0x0000..0x3FFF, SV 0x4000..0x7FFF
    if (ethertype == ETHERTYPE_GOOSE && appid > 0x3FFF) {
        return GuardStatus::REJECT_GOOSE_INVALID_APPID;
    }
    if (ethertype == ETHERTYPE_SV && (appid < 0x4000 || appid > 0x7FFF)) {
        return GuardStatus::REJECT_GOOSE_INVALID_APPID;
    }

    // Compact GOOSE header counters at bytes [8..15] (stNum, sqNum) if 16+ bytes
    if (l2_payload.size() >= 16) {
        const uint32_t st_num = read_be32(&l2_payload[8]);
        const uint32_t sq_num = read_be32(&l2_payload[12]);

        if (!initialized_) {
            last_st_num_ = st_num;
            last_sq_num_ = sq_num;
            initialized_ = true;
            return GuardStatus::PASS;
        }

        // Detect forged / replayed GOOSE state number regression or massive jump (> 64)
        if (st_num < last_st_num_ || (st_num == last_st_num_ && sq_num <= last_sq_num_)) {
            return GuardStatus::REJECT_GOOSE_SPOOFED_STNUM;
        }
        if (st_num > last_st_num_ + 64U) {
            return GuardStatus::REJECT_GOOSE_SPOOFED_STNUM;
        }

        last_st_num_ = st_num;
        last_sq_num_ = sq_num;
    }

    return GuardStatus::PASS;
}

} // namespace net_guard::protocols