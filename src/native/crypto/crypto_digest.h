#pragma once

// crypto_digest.h — Cryptographic hash/digest functions and encoding helpers.
// All digest functions use the OpenSSL EVP high-level API.

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace fakelua::crypto {

// Hex encoding helper
std::string ToHex(const uint8_t *data, size_t len);

// Base64 encoding/decoding (RFC 4648)
std::string Base64Encode(const uint8_t *data, size_t len);
std::string Base64Decode(const uint8_t *data, size_t len);

// MD5: 128-bit (16-byte) digest
std::array<uint8_t, 16> Md5(const uint8_t *data, size_t len);

inline std::array<uint8_t, 16> Md5(const std::string &data) {
    return Md5(reinterpret_cast<const uint8_t *>(data.data()), data.size());
}

// SHA1: 160-bit (20-byte) digest
std::array<uint8_t, 20> Sha1(const uint8_t *data, size_t len);

inline std::array<uint8_t, 20> Sha1(const std::string &data) {
    return Sha1(reinterpret_cast<const uint8_t *>(data.data()), data.size());
}

// SHA256: 256-bit (32-byte) digest
std::array<uint8_t, 32> Sha256(const uint8_t *data, size_t len);

inline std::array<uint8_t, 32> Sha256(const std::string &data) {
    return Sha256(reinterpret_cast<const uint8_t *>(data.data()), data.size());
}

}// namespace fakelua::crypto
