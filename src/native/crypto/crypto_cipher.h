#pragma once

// crypto_cipher.h — Symmetric stream and block cipher functions.
// All implementations use the OpenSSL EVP high-level API.
// No deprecated low-level APIs (RC4_set_key, BF_set_key, DES_ecb_encrypt) are used.

#include <cstdint>
#include <string>
#include <vector>

namespace fakelua::crypto {

// ── RC4 stream cipher ──
// RC4 is symmetric: encrypt and decrypt are the same operation (XOR keystream).
// Returns output of same length as input.
// key_len must satisfy 1 <= key_len <= INT_MAX.
std::vector<uint8_t> rc4(const uint8_t *key, size_t key_len,
                         const uint8_t *data, size_t data_len);
inline std::vector<uint8_t> rc4(const std::string &key, const std::string &data) {
    return rc4(reinterpret_cast<const uint8_t *>(key.data()), key.size(),
               reinterpret_cast<const uint8_t *>(data.data()), data.size());
}

// ── Blowfish block cipher (ECB mode, zero-padded) ──
// Block size = 8 bytes. Data is zero-padded to a multiple of 8.
// Key length: 4–56 bytes (32–448 bits).
std::vector<uint8_t> blowfish_encrypt(const uint8_t *key, size_t key_len,
                                      const uint8_t *data, size_t data_len);
std::vector<uint8_t> blowfish_decrypt(const uint8_t *key, size_t key_len,
                                      const uint8_t *data, size_t data_len);

// ── DES block cipher (ECB mode, zero-padded) ──
// Block size = 8 bytes. Only the first 8 bytes of key are used (56-bit effective key).
// Data is zero-padded to a multiple of 8.
std::vector<uint8_t> des_encrypt(const uint8_t *key, size_t key_len,
                                 const uint8_t *data, size_t data_len);
std::vector<uint8_t> des_decrypt(const uint8_t *key, size_t key_len,
                                 const uint8_t *data, size_t data_len);

// ── Triple DES (DES-EDE3) block cipher (ECB mode, zero-padded) ──
// Block size = 8 bytes. Key = 24 bytes (three 8-byte sub-keys). Data zero-padded.
// EDE order: encrypt with key1, decrypt with key2, encrypt with key3.
std::vector<uint8_t> triple_des_encrypt(const uint8_t *key, size_t key_len,
                                        const uint8_t *data, size_t data_len);
std::vector<uint8_t> triple_des_decrypt(const uint8_t *key, size_t key_len,
                                        const uint8_t *data, size_t data_len);

}  // namespace fakelua::crypto
