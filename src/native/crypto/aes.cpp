#include "native/crypto/aes.h"
#include "util/exception.h"

#include <cstring>
#include <limits>
#include <openssl/evp.h>

namespace fakelua::crypto {

// ─────────────────────────────────────────────────────────────────────────────
// ECB mode (electronic codebook)
// ─────────────────────────────────────────────────────────────────────────────

void aes_encrypt_ecb(const uint8_t in[16], uint8_t out[16],
                     const uint8_t key[AES_BLOCK_SIZE], AesKeySize key_size) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) ThrowFakeluaException("Failed to create EVP_CIPHER_CTX");

    const EVP_CIPHER *cipher = nullptr;
    switch (key_size) {
        case AesKeySize::AES_128: cipher = EVP_aes_128_ecb(); break;
        case AesKeySize::AES_192: cipher = EVP_aes_192_ecb(); break;
        case AesKeySize::AES_256: cipher = EVP_aes_256_ecb(); break;
    }
    if (!cipher) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("Unsupported AES key size");
    }

    if (EVP_EncryptInit_ex(ctx, cipher, nullptr, key, nullptr) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("Failed to initialize AES-ECB encryption");
    }

    // ECB operates on a single 16-byte block: disable padding so
    // EncryptFinal writes nothing beyond the one block.
    EVP_CIPHER_CTX_set_padding(ctx, 0);

    int outlen = 0;
    if (EVP_EncryptUpdate(ctx, out, &outlen, in, 16) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("Failed to encrypt AES-ECB");
    }
    int final_len = 0;
    if (EVP_EncryptFinal_ex(ctx, out + outlen, &final_len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("Failed to finalize AES-ECB encryption");
    }
    outlen += final_len;

    EVP_CIPHER_CTX_free(ctx);
}

void aes_decrypt_ecb(const uint8_t in[16], uint8_t out[16],
                     const uint8_t key[AES_BLOCK_SIZE], AesKeySize key_size) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) ThrowFakeluaException("Failed to create EVP_CIPHER_CTX");

    const EVP_CIPHER *cipher = nullptr;
    switch (key_size) {
        case AesKeySize::AES_128: cipher = EVP_aes_128_ecb(); break;
        case AesKeySize::AES_192: cipher = EVP_aes_192_ecb(); break;
        case AesKeySize::AES_256: cipher = EVP_aes_256_ecb(); break;
    }
    if (!cipher) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("Unsupported AES key size");
    }

    if (EVP_DecryptInit_ex(ctx, cipher, nullptr, key, nullptr) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("Failed to initialize AES-ECB decryption");
    }

    // ECB operates on a single 16-byte block: disable padding so
    // DecryptFinal writes nothing beyond the one block.
    EVP_CIPHER_CTX_set_padding(ctx, 0);

    int outlen = 0;
    if (EVP_DecryptUpdate(ctx, out, &outlen, in, 16) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("Failed to decrypt AES-ECB");
    }
    int final_len = 0;
    if (EVP_DecryptFinal_ex(ctx, out + outlen, &final_len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("Failed to finalize AES-ECB decryption");
    }
    outlen += final_len;

    EVP_CIPHER_CTX_free(ctx);
}

// ─────────────────────────────────────────────────────────────────────────────
// CBC mode (cipher block chaining)
// ─────────────────────────────────────────────────────────────────────────────

std::vector<uint8_t> aes_encrypt_cbc(const uint8_t *data, size_t len,
                                     const uint8_t key[AES_BLOCK_SIZE], AesKeySize key_size,
                                     const uint8_t iv[AES_BLOCK_SIZE]) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) ThrowFakeluaException("Failed to create EVP_CIPHER_CTX");

    const EVP_CIPHER *cipher = nullptr;
    switch (key_size) {
        case AesKeySize::AES_128: cipher = EVP_aes_128_cbc(); break;
        case AesKeySize::AES_192: cipher = EVP_aes_192_cbc(); break;
        case AesKeySize::AES_256: cipher = EVP_aes_256_cbc(); break;
    }
    if (!cipher) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("Unsupported AES key size");
    }

    if (EVP_EncryptInit_ex(ctx, cipher, nullptr, key, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("Failed to initialize AES-CBC encryption");
    }

    // Enable padding (PKCS#7)
    EVP_CIPHER_CTX_set_padding(ctx, 1);

    int outlen = 0;
    std::vector<uint8_t> result(len + AES_BLOCK_SIZE); // Upper bound

    if (EVP_EncryptUpdate(ctx, result.data(), &outlen, data, len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("Failed to encrypt AES-CBC");
    }
    int final_len = 0;
    if (EVP_EncryptFinal_ex(ctx, result.data() + outlen, &final_len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("Failed to finalize AES-CBC encryption");
    }
    outlen += final_len;

    result.resize(outlen);
    EVP_CIPHER_CTX_free(ctx);
    return result;
}

std::vector<uint8_t> aes_decrypt_cbc(const uint8_t *data, size_t len,
                                     const uint8_t key[AES_BLOCK_SIZE], AesKeySize key_size,
                                     const uint8_t iv[AES_BLOCK_SIZE]) {
    if (len == 0 || len % AES_BLOCK_SIZE != 0) {
        ThrowFakeluaException("aes_decrypt_cbc: ciphertext length must be a multiple of 16");
    }

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) ThrowFakeluaException("Failed to create EVP_CIPHER_CTX");

    const EVP_CIPHER *cipher = nullptr;
    switch (key_size) {
        case AesKeySize::AES_128: cipher = EVP_aes_128_cbc(); break;
        case AesKeySize::AES_192: cipher = EVP_aes_192_cbc(); break;
        case AesKeySize::AES_256: cipher = EVP_aes_256_cbc(); break;
    }
    if (!cipher) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("Unsupported AES key size");
    }

    if (EVP_DecryptInit_ex(ctx, cipher, nullptr, key, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("Failed to initialize AES-CBC decryption");
    }

    // Enable padding (PKCS#7)
    EVP_CIPHER_CTX_set_padding(ctx, 1);

    int outlen = 0;
    std::vector<uint8_t> result(len); // Same size as input for CBC with padding

    if (EVP_DecryptUpdate(ctx, result.data(), &outlen, data, len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("Failed to decrypt AES-CBC");
    }
    int final_len = 0;
    if (EVP_DecryptFinal_ex(ctx, result.data() + outlen, &final_len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("Failed to finalize AES-CBC decryption");
    }
    outlen += final_len;

    result.resize(outlen);
    EVP_CIPHER_CTX_free(ctx);
    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
// CTR mode (counter)
// ─────────────────────────────────────────────────────────────────────────────

// CTR mode: nonce (first 8 bytes) + counter (last 8 bytes, big-endian, starting from 0)
// This matches Python's AES.MODE_CTR convention.
static void increment_counter(uint8_t counter[8]) {
    for (int i = 7; i >= 0; --i) {
        if (++counter[i] != 0) break;
    }
}

std::vector<uint8_t> aes_encrypt_ctr(const uint8_t *data, size_t len,
                                     const uint8_t key[AES_BLOCK_SIZE], AesKeySize key_size,
                                     const uint8_t iv[16]) {
    // Extract nonce (first 8 bytes of IV)
    uint8_t nonce[8];
    std::memcpy(nonce, iv, 8);

    // Initialize counter (last 8 bytes, big-endian, starting from 0)
    uint8_t counter[8] = {0};

    std::vector<uint8_t> result(len);
    size_t pos = 0;

    while (pos < len) {
        // Build counter block: nonce || counter (16 bytes total)
        uint8_t counter_block[16];
        std::memcpy(counter_block, nonce, 8);
        std::memcpy(counter_block + 8, counter, 8);

        // Encrypt counter block with AES-ECB to get keystream
        uint8_t keystream[16];
        aes_encrypt_ecb(counter_block, keystream, key, key_size);

        // XOR keystream with input to get output
        size_t chunk = std::min((size_t)AES_BLOCK_SIZE, len - pos);
        for (size_t i = 0; i < chunk; ++i) {
            result[pos + i] = data[pos + i] ^ keystream[i];
        }
        pos += chunk;

        // Increment counter for next block
        increment_counter(counter);
    }

    return result;
}

std::vector<uint8_t> aes_decrypt_ctr(const uint8_t *data, size_t len,
                                     const uint8_t key[AES_BLOCK_SIZE], AesKeySize key_size,
                                     const uint8_t iv[16]) {
    // CTR mode encryption and decryption are the same operation
    return aes_encrypt_ctr(data, len, key, key_size, iv);
}

}  // namespace fakelua::crypto