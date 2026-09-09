#pragma once

// aes.h — AES symmetric encryption (128/192/256-bit) with ECB, CBC, CTR modes
// implemented via OpenSSL EVP API.
//
// Key length is determined by key_size:
//   AES_128 → key must be 16 bytes
//   AES_192 → key must be 24 bytes
//   AES_256 → key must be 32 bytes
//
// The Lua binding currently only exposes AES-128 (16-byte key).

#include <array>
#include <cstdint>
#include <vector>

namespace fakelua::crypto {

// ── AES block size ──
static constexpr int AES_BLOCK_SIZE = 16;

// ── Key sizes ──
enum class AesKeySize {
    AES_128 = 16,
    AES_192 = 24,
    AES_256 = 32,
};

// ── ECB mode (electronic codebook) ──
// Encrypt/decrypt exactly one 16-byte block (in[16] → out[16]).
// Padding is disabled internally; the caller is responsible for block alignment.
// key must point to at least static_cast<int>(key_size) bytes.
void aes_encrypt_ecb(const uint8_t in[16], uint8_t out[16],
                     const uint8_t *key, AesKeySize key_size);
void aes_decrypt_ecb(const uint8_t in[16], uint8_t out[16],
                     const uint8_t *key, AesKeySize key_size);

// ── CBC mode (cipher block chaining) ──
// Encrypt with PKCS#7 padding. Output length = ceil((len+1)/16)*16 (1–16 bytes added).
// Decrypt removes PKCS#7 padding. Ciphertext length must be a multiple of 16.
// key must point to at least static_cast<int>(key_size) bytes.
// iv must be exactly 16 bytes.
std::vector<uint8_t> aes_encrypt_cbc(const uint8_t *data, size_t len,
                                     const uint8_t *key, AesKeySize key_size,
                                     const uint8_t iv[AES_BLOCK_SIZE]);
std::vector<uint8_t> aes_decrypt_cbc(const uint8_t *data, size_t len,
                                     const uint8_t *key, AesKeySize key_size,
                                     const uint8_t iv[AES_BLOCK_SIZE]);

// ── CTR mode (counter) ──
// Stream cipher mode: no padding. Output length = input length.
// iv[0..7] = nonce; iv[8..15] are ignored. Counter starts at 0.
// key must point to at least static_cast<int>(key_size) bytes.
std::vector<uint8_t> aes_encrypt_ctr(const uint8_t *data, size_t len,
                                     const uint8_t *key, AesKeySize key_size,
                                     const uint8_t iv[AES_BLOCK_SIZE]);
std::vector<uint8_t> aes_decrypt_ctr(const uint8_t *data, size_t len,
                                     const uint8_t *key, AesKeySize key_size,
                                     const uint8_t iv[AES_BLOCK_SIZE]);

}  // namespace fakelua::crypto
