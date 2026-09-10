#pragma once

// crypto_digest.h — Cryptographic hash/digest functions and encoding helpers.
// All digest functions use the OpenSSL EVP high-level API.

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace fakelua::crypto {

// ── Hex encoding helper ──
std::string to_hex(const uint8_t *data, size_t len);

// ── Base64 encoding/decoding (RFC 4648) ──
std::string base64_encode(const uint8_t *data, size_t len);
std::string base64_decode(const uint8_t *data, size_t len);

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

}  // namespace fakelua::crypto
