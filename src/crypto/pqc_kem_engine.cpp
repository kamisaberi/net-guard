// File: /home/kami/net-guard/src/crypto/pqc_kem_engine.cpp

#include "net_guard/crypto/pqc_kem_engine.hpp"
#include <chrono>
#include <openssl/evp.h>

namespace net_guard::crypto {

PqcKemEngine::PqcKemEngine(std::string algorithm)
    : algorithm_(std::move(algorithm)) {}

PqcKemEngine::~PqcKemEngine() {
    if (pkey_) {
        EVP_PKEY_free(static_cast<EVP_PKEY*>(pkey_));
        pkey_ = nullptr;
    }
}

bool PqcKemEngine::generate_keypair() {
    if (pkey_) {
        EVP_PKEY_free(static_cast<EVP_PKEY*>(pkey_));
        pkey_ = nullptr;
    }
    EVP_PKEY* k = EVP_PKEY_Q_keygen(nullptr, nullptr, algorithm_.c_str());
    if (!k) return false;
    pkey_ = k;
    return true;
}

KemEncapResult PqcKemEngine::encapsulate() const {
    KemEncapResult res;
    if (!pkey_) return res;

    auto t0 = std::chrono::steady_clock::now();
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_from_pkey(nullptr, static_cast<EVP_PKEY*>(pkey_), nullptr);
    if (!ctx) return res;

    size_t ct_len = 0, ss_len = 0;
    if (EVP_PKEY_encapsulate_init(ctx, nullptr) == 1 &&
        EVP_PKEY_encapsulate(ctx, nullptr, &ct_len, nullptr, &ss_len) == 1) {
        res.ciphertext.resize(ct_len);
        res.shared_secret.resize(ss_len);
        if (EVP_PKEY_encapsulate(ctx, res.ciphertext.data(), &ct_len, res.shared_secret.data(), &ss_len) == 1) {
            res.ciphertext.resize(ct_len);
            res.shared_secret.resize(ss_len);
            res.ok = true;
        }
    }

    EVP_PKEY_CTX_free(ctx);
    auto t1 = std::chrono::steady_clock::now();
    res.latency_us = static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count()) / 1000.0;
    return res;
}

bool PqcKemEngine::decapsulate(
    const std::vector<uint8_t>& ciphertext,
    std::vector<uint8_t>& out_shared_secret,
    double* out_latency_us) const
{
    if (!pkey_ || ciphertext.empty()) return false;

    auto t0 = std::chrono::steady_clock::now();
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_from_pkey(nullptr, static_cast<EVP_PKEY*>(pkey_), nullptr);
    if (!ctx) return false;

    bool ok = false;
    size_t ss_len = 0;
    if (EVP_PKEY_decapsulate_init(ctx, nullptr) == 1 &&
        EVP_PKEY_decapsulate(ctx, nullptr, &ss_len, ciphertext.data(), ciphertext.size()) == 1) {
        out_shared_secret.resize(ss_len);
        if (EVP_PKEY_decapsulate(ctx, out_shared_secret.data(), &ss_len, ciphertext.data(), ciphertext.size()) == 1) {
            out_shared_secret.resize(ss_len);
            ok = true;
        }
    }

    EVP_PKEY_CTX_free(ctx);
    auto t1 = std::chrono::steady_clock::now();
    if (out_latency_us) {
        *out_latency_us = static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count()) / 1000.0;
    }
    return ok;
}

} // namespace net_guard::crypto