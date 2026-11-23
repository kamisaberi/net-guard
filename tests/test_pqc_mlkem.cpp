// File: /home/kami/net-guard/tests/test_pqc_mlkem.cpp

#include "net_guard/crypto/pqc_kem_engine.hpp"
#include <cassert>
#include <iostream>
#include <vector>

int main()
{
    std::cout << "[TEST] Native OpenSSL 3.5 FIPS 203 ML-KEM-768 Encap/Decap & Implicit Rejection...\n";

    net_guard::crypto::PqcKemEngine kem("ML-KEM-768");
    assert(kem.generate_keypair());

    auto enc = kem.encapsulate();
    assert(enc.ok);
    assert(enc.ciphertext.size() == 1088);
    assert(enc.shared_secret.size() == 32);

    std::vector<uint8_t> dec_ss;
    double decap_us = 0.0;
    assert(kem.decapsulate(enc.ciphertext, dec_ss, &decap_us));
    assert(dec_ss == enc.shared_secret);

    // Flip 1 bit in ciphertext -> FIPS 203 IND-CCA2 implicit rejection must yield a different secret
    std::vector<uint8_t> tampered_ct = enc.ciphertext;
    tampered_ct[0] ^= 0x01;
    std::vector<uint8_t> bad_ss;
    assert(kem.decapsulate(tampered_ct, bad_ss));
    assert(bad_ss != enc.shared_secret);

    std::cout << "  [+] ML-KEM-768 Ciphertext: " << enc.ciphertext.size()
              << " B | Secret: " << enc.shared_secret.size()
              << " B | Encap: " << enc.latency_us << " us | Decap: " << decap_us << " us -> PASSED\n";
    return 0;
}