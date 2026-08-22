#pragma once

// aes.h — AES symmetric encryption (128/192/256-bit) with ECB, CBC, CTR modes.
// Self-contained: no OpenSSL. Based on NIST FIPS PUB 197.
// Useful for encrypting game server packets.

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
// Encrypt/decrypt a 16-byte block. Not recommended for repeated use with same key.
void aes_encrypt_ecb(const uint8_t in[16], uint8_t out[16],
                     const uint8_t key[AES_BLOCK_SIZE], AesKeySize key_size);
void aes_decrypt_ecb(const uint8_t in[16], uint8_t out[16],
                     const uint8_t key[AES_BLOCK_SIZE], AesKeySize key_size);

// ── CBC mode (cipher block chaining) ──
// Encrypt with PKCS#7 padding. Output length = input length + padding (1-16 bytes).
std::vector<uint8_t> aes_encrypt_cbc(const uint8_t *data, size_t len,
                                     const uint8_t key[AES_BLOCK_SIZE], AesKeySize key_size,
                                     const uint8_t iv[AES_BLOCK_SIZE]);
std::vector<uint8_t> aes_decrypt_cbc(const uint8_t *data, size_t len,
                                     const uint8_t key[AES_BLOCK_SIZE], AesKeySize key_size,
                                     const uint8_t iv[AES_BLOCK_SIZE]);

// ── CTR mode (counter) ──
// Stream cipher mode: no padding needed. Output length = input length.
std::vector<uint8_t> aes_encrypt_ctr(const uint8_t *data, size_t len,
                                     const uint8_t key[AES_BLOCK_SIZE], AesKeySize key_size,
                                     const uint8_t iv[AES_BLOCK_SIZE]);
std::vector<uint8_t> aes_decrypt_ctr(const uint8_t *data, size_t len,
                                     const uint8_t key[AES_BLOCK_SIZE], AesKeySize key_size,
                                     const uint8_t iv[AES_BLOCK_SIZE]);

}  // namespace fakelua::crypto
