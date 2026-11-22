// File: /home/kami/net-guard/src/protocols/pqc_tls.cpp

#include "net_guard/protocols/pqc_tls.hpp"

namespace net_guard::protocols {

GuardStatus PqcTlsValidator::validate_client_hello(
    std::span<const uint8_t> tls_record,
    bool require_pqc) noexcept
{
    // TLS Record Header: 0x16 (Handshake), TLS 1.x (0x03 0x01..0x03), HandshakeType 0x01 (ClientHello)
    if (tls_record.size() < 12 || tls_record[0] != 0x16 || tls_record[1] != 0x03) {
        return GuardStatus::REJECT_MALFORMED_FRAME;
    }
    if (tls_record[5] != 0x01) {
        return GuardStatus::PASS;
    }
    if (!require_pqc) {
        return GuardStatus::PASS;
    }

    // Scan record for recognized NIST ML-KEM / X25519MLKEM768 NamedGroup big-endian tokens
    for (size_t i = 6; i + 1 < tls_record.size(); ++i) {
        const uint16_t grp = (static_cast<uint16_t>(tls_record[i]) << 8) | tls_record[i + 1];
        if (grp == GROUP_MLKEM768 || grp == GROUP_X25519_MLKEM768 || grp == GROUP_MLKEM1024) {
            return GuardStatus::PASS;
        }
    }

    return GuardStatus::REJECT_PQC_CLASSICAL_DOWNGRADE;
}

} // namespace net_guard::protocols