#include "native/crypto/crypto_aes.h"
#include "util/exception.h"

#include <cstring>
#include <openssl/evp.h>

namespace fakelua::crypto {

// Internal helper: select cipher by key size

static const EVP_CIPHER *SelectEcbCipher(AesKeySize key_size, EVP_CIPHER_CTX *ctx) {
    switch (key_size) {
        case AesKeySize::AES_128:
            return EVP_aes_128_ecb();
        case AesKeySize::AES_192:
            return EVP_aes_192_ecb();
        case AesKeySize::AES_256:
            return EVP_aes_256_ecb();
    }
    EVP_CIPHER_CTX_free(ctx);
    ThrowFakeluaException("aes: unsupported key size");
}

static const EVP_CIPHER *SelectCtrCipher(AesKeySize key_size, EVP_CIPHER_CTX *ctx) {
    switch (key_size) {
        case AesKeySize::AES_128:
            return EVP_aes_128_ctr();
        case AesKeySize::AES_192:
            return EVP_aes_192_ctr();
        case AesKeySize::AES_256:
            return EVP_aes_256_ctr();
    }
    EVP_CIPHER_CTX_free(ctx);
    ThrowFakeluaException("aes: unsupported key size");
}

static const EVP_CIPHER *SelectCbcCipher(AesKeySize key_size, EVP_CIPHER_CTX *ctx) {
    switch (key_size) {
        case AesKeySize::AES_128:
            return EVP_aes_128_cbc();
        case AesKeySize::AES_192:
            return EVP_aes_192_cbc();
        case AesKeySize::AES_256:
            return EVP_aes_256_cbc();
    }
    EVP_CIPHER_CTX_free(ctx);
    ThrowFakeluaException("aes: unsupported key size");
}

// ECB mode (electronic codebook) — single 16-byte block per call
// The Lua binding iterates blocks externally; padding MUST be disabled so
// EVP_EncryptFinal_ex / EVP_DecryptFinal_ex write nothing beyond the block.

void AesEncryptEcb(const uint8_t in[16], uint8_t out[16], const uint8_t *key, AesKeySize key_size) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) ThrowFakeluaException("aes_encrypt_ecb: failed to create EVP_CIPHER_CTX");

    const EVP_CIPHER *cipher = SelectEcbCipher(key_size, ctx);

    if (EVP_EncryptInit_ex(ctx, cipher, nullptr, key, nullptr) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("aes_encrypt_ecb: failed to initialize");
    }

    // Disable padding: we handle one exact block; enabling padding would cause
    // EVP_EncryptFinal_ex to write a 17th byte past the 16-byte out[] buffer.
    if (EVP_CIPHER_CTX_set_padding(ctx, 0) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("aes_encrypt_ecb: failed to disable padding");
    }

    int outlen = 0;
    if (EVP_EncryptUpdate(ctx, out, &outlen, in, 16) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("aes_encrypt_ecb: EVP_EncryptUpdate failed");
    }
    int final_len = 0;
    if (EVP_EncryptFinal_ex(ctx, out + outlen, &final_len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("aes_encrypt_ecb: EVP_EncryptFinal_ex failed");
    }

    EVP_CIPHER_CTX_free(ctx);
}

void AesDecryptEcb(const uint8_t in[16], uint8_t out[16], const uint8_t *key, AesKeySize key_size) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) ThrowFakeluaException("aes_decrypt_ecb: failed to create EVP_CIPHER_CTX");

    const EVP_CIPHER *cipher = SelectEcbCipher(key_size, ctx);

    if (EVP_DecryptInit_ex(ctx, cipher, nullptr, key, nullptr) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("aes_decrypt_ecb: failed to initialize");
    }

    // Disable padding — same reasoning as encrypt path.
    if (EVP_CIPHER_CTX_set_padding(ctx, 0) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("aes_decrypt_ecb: failed to disable padding");
    }

    int outlen = 0;
    if (EVP_DecryptUpdate(ctx, out, &outlen, in, 16) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("aes_decrypt_ecb: EVP_DecryptUpdate failed");
    }
    int final_len = 0;
    if (EVP_DecryptFinal_ex(ctx, out + outlen, &final_len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("aes_decrypt_ecb: EVP_DecryptFinal_ex failed");
    }

    EVP_CIPHER_CTX_free(ctx);
}

// CBC mode (cipher block chaining) — PKCS#7 padding, full buffer per call

std::vector<uint8_t> AesEncryptCbc(const uint8_t *data, size_t len, const uint8_t *key, AesKeySize key_size, const uint8_t iv[AES_BLOCK_SIZE]) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) ThrowFakeluaException("aes_encrypt_cbc: failed to create EVP_CIPHER_CTX");

    const EVP_CIPHER *cipher = SelectCbcCipher(key_size, ctx);

    if (EVP_EncryptInit_ex(ctx, cipher, nullptr, key, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("aes_encrypt_cbc: failed to initialize");
    }
    // Padding is ON by default (PKCS#7); no need to call set_padding(ctx, 1).

    std::vector<uint8_t> result(len + AES_BLOCK_SIZE);
    int outlen = 0;
    if (EVP_EncryptUpdate(ctx, result.data(), &outlen, data, static_cast<int>(len)) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("aes_encrypt_cbc: EVP_EncryptUpdate failed");
    }
    int final_len = 0;
    if (EVP_EncryptFinal_ex(ctx, result.data() + outlen, &final_len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("aes_encrypt_cbc: EVP_EncryptFinal_ex failed");
    }
    outlen += final_len;

    result.resize(outlen);
    EVP_CIPHER_CTX_free(ctx);
    return result;
}

std::vector<uint8_t> AesDecryptCbc(const uint8_t *data, size_t len, const uint8_t *key, AesKeySize key_size, const uint8_t iv[AES_BLOCK_SIZE]) {
    if (len == 0 || len % AES_BLOCK_SIZE != 0) {
        ThrowFakeluaException("aes_decrypt_cbc: ciphertext length must be a multiple of 16");
    }

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) ThrowFakeluaException("aes_decrypt_cbc: failed to create EVP_CIPHER_CTX");

    const EVP_CIPHER *cipher = SelectCbcCipher(key_size, ctx);

    if (EVP_DecryptInit_ex(ctx, cipher, nullptr, key, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("aes_decrypt_cbc: failed to initialize");
    }
    // Padding is ON by default (PKCS#7).

    std::vector<uint8_t> result(len);
    int outlen = 0;
    if (EVP_DecryptUpdate(ctx, result.data(), &outlen, data, static_cast<int>(len)) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("aes_decrypt_cbc: EVP_DecryptUpdate failed");
    }
    int final_len = 0;
    if (EVP_DecryptFinal_ex(ctx, result.data() + outlen, &final_len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("aes_decrypt_cbc: EVP_DecryptFinal_ex failed — bad padding or corrupt data");
    }
    outlen += final_len;

    result.resize(outlen);
    EVP_CIPHER_CTX_free(ctx);
    return result;
}

// CTR mode — OpenSSL AES-CTR: the 16-byte IV is the initial counter block.
// Matches crypto.encrypt("aes-*-ctr", ...). Output length equals input length.

std::vector<uint8_t> AesEncryptCtr(const uint8_t *data, size_t len, const uint8_t *key, AesKeySize key_size, const uint8_t iv[AES_BLOCK_SIZE]) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) ThrowFakeluaException("aes_encrypt_ctr: failed to create EVP_CIPHER_CTX");
    const EVP_CIPHER *cipher = SelectCtrCipher(key_size, ctx);
    if (EVP_EncryptInit_ex(ctx, cipher, nullptr, key, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("aes_encrypt_ctr: failed to initialize");
    }
    EVP_CIPHER_CTX_set_padding(ctx, 0);
    std::vector<uint8_t> result(len + AES_BLOCK_SIZE);
    int outlen = 0;
    if (len > 0 && EVP_EncryptUpdate(ctx, result.data(), &outlen, data, static_cast<int>(len)) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("aes_encrypt_ctr: EVP_EncryptUpdate failed");
    }
    int final_len = 0;
    if (EVP_EncryptFinal_ex(ctx, result.data() + outlen, &final_len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("aes_encrypt_ctr: EVP_EncryptFinal_ex failed");
    }
    result.resize(static_cast<size_t>(outlen + final_len));
    EVP_CIPHER_CTX_free(ctx);
    return result;
}

std::vector<uint8_t> AesDecryptCtr(const uint8_t *data, size_t len, const uint8_t *key, AesKeySize key_size, const uint8_t iv[AES_BLOCK_SIZE]) {
    return AesEncryptCtr(data, len, key, key_size, iv);
}

}// namespace fakelua::crypto
