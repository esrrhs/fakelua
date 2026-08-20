#pragma once

// hash.md5, hash.sha1, hash.sha256 — self-contained hash algorithms (no OpenSSL).
// Modeled after Go's crypto library: each returns raw bytes; Lua bindings return
// hex strings. Reusable by any native module (e.g. mysql uses sha1 for auth).

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace fakelua::crypto {

// ── MD5: 128-bit (16-byte) digest ──
std::array<uint8_t, 16> md5(const uint8_t *data, size_t len);
inline std::array<uint8_t, 16> md5(const std::string &data) {
    return md5(reinterpret_cast<const uint8_t *>(data.data()), data.size());
}

// ── SHA1: 160-bit (20-byte) digest ──
std::array<uint8_t, 20> sha1(const uint8_t *data, size_t len);
inline std::array<uint8_t, 20> sha1(const std::string &data) {
    return sha1(reinterpret_cast<const uint8_t *>(data.data()), data.size());
}

// ── SHA256: 256-bit (32-byte) digest ──
std::array<uint8_t, 32> sha256(const uint8_t *data, size_t len);
inline std::array<uint8_t, 32> sha256(const std::string &data) {
    return sha256(reinterpret_cast<const uint8_t *>(data.data()), data.size());
}

// ── Hex encoding helper ──
std::string to_hex(const uint8_t *data, size_t len);

// ── Base64 encoding/decoding (RFC 4648) ──
std::string base64_encode(const uint8_t *data, size_t len);
std::string base64_decode(const uint8_t *data, size_t len);

// ── RC4 stream cipher ──
// RC4 is symmetric: encrypt and decrypt are the same operation (XOR keystream).
// Returns output of same length as input.
std::vector<uint8_t> rc4(const uint8_t *key, size_t key_len,
                         const uint8_t *data, size_t data_len);
inline std::vector<uint8_t> rc4(const std::string &key, const std::string &data) {
    return rc4(reinterpret_cast<const uint8_t *>(key.data()), key.size(),
               reinterpret_cast<const uint8_t *>(data.data()), data.size());
}

// ── Blowfish block cipher (ECB mode, zero-padded) ──
// Block size = 8 bytes. Data is zero-padded to a multiple of 8.
std::vector<uint8_t> blowfish_encrypt(const uint8_t *key, size_t key_len,
                                      const uint8_t *data, size_t data_len);
std::vector<uint8_t> blowfish_decrypt(const uint8_t *key, size_t key_len,
                                      const uint8_t *data, size_t data_len);

// ── DES block cipher (ECB mode, zero-padded) ──
// Block size = 8 bytes. Key = 8 bytes. Data is zero-padded to a multiple of 8.
std::vector<uint8_t> des_encrypt(const uint8_t *key, size_t key_len,
                                 const uint8_t *data, size_t data_len);
std::vector<uint8_t> des_decrypt(const uint8_t *key, size_t key_len,
                                 const uint8_t *data, size_t data_len);

// ── 3DES (Triple DES) block cipher ──
// Block size = 8 bytes. Key = 24 bytes (or 16 for two-key 3DES). Data zero-padded.
std::vector<uint8_t> triple_des_encrypt(const uint8_t *key, size_t key_len,
                                        const uint8_t *data, size_t data_len);
std::vector<uint8_t> triple_des_decrypt(const uint8_t *key, size_t key_len,
                                        const uint8_t *data, size_t data_len);

}  // namespace fakelua::crypto
