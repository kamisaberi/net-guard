// File: /home/kami/net-guard/include/net_guard/crypto/pqc_kem_engine.hpp

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace net_guard::crypto {

struct KemEncapResult {
    bool ok{false};
    std::vector<uint8_t> ciphertext;
    std::vector<uint8_t> shared_secret;
    double latency_us{0.0};
};

class PqcKemEngine {
public:
    explicit PqcKemEngine(std::string algorithm = "ML-KEM-768");
    ~PqcKemEngine();

    PqcKemEngine(const PqcKemEngine&) = delete;
    PqcKemEngine& operator=(const PqcKemEngine&) = delete;

    /// Generates a native OpenSSL 3.5 FIPS 203 ML-KEM keypair
    [[nodiscard]] bool generate_keypair();

    /// Encapsulates a fresh 32-byte quantum-resistant shared secret using the public key
    [[nodiscard]] KemEncapResult encapsulate() const;

    /// Decapsulates ciphertext using the private key (FIPS 203 implicit rejection on tamper)
    [[nodiscard]] bool decapsulate(
        const std::vector<uint8_t>& ciphertext,
        std::vector<uint8_t>& out_shared_secret,
        double* out_latency_us = nullptr) const;

    [[nodiscard]] const std::string& algorithm() const noexcept { return algorithm_; }

private:
    std::string algorithm_;
    void* pkey_{nullptr}; // Opaque EVP_PKEY*
};

} // namespace net_guard::crypto