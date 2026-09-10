#include "crypto_cipher.h"
#include "util/exception.h"

#include <climits>
#include <cstring>
#include <openssl/evp.h>

namespace fakelua::crypto {

// ─────────────────────────────────────────────────────────────────────────────
// RC4 stream cipher — EVP_rc4() (variable key length, no IV)
// ─────────────────────────────────────────────────────────────────────────────

std::vector<uint8_t> rc4(const uint8_t *key, size_t key_len,
                         const uint8_t *data, size_t data_len) {
    if (key_len == 0) ThrowFakeluaException("rc4: empty key");
    if (key_len > static_cast<size_t>(INT_MAX))
        ThrowFakeluaException("rc4: key too long");

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) ThrowFakeluaException("rc4: failed to create EVP_CIPHER_CTX");

    if (EVP_EncryptInit_ex(ctx, EVP_rc4(), nullptr, nullptr, nullptr) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("rc4: EVP_EncryptInit_ex (cipher) failed");
    }
    if (EVP_CIPHER_CTX_set_key_length(ctx, static_cast<int>(key_len)) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("rc4: EVP_CIPHER_CTX_set_key_length failed");
    }
    if (EVP_EncryptInit_ex(ctx, nullptr, nullptr, key, nullptr) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("rc4: EVP_EncryptInit_ex (key) failed");
    }

    std::vector<uint8_t> out(data_len);
    int outlen = 0;
    if (data_len > 0) {
        if (EVP_EncryptUpdate(ctx, out.data(), &outlen, data, static_cast<int>(data_len)) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            ThrowFakeluaException("rc4: EVP_EncryptUpdate failed");
        }
    }
    int final_len = 0;
    EVP_EncryptFinal_ex(ctx, out.data() + outlen, &final_len);

    EVP_CIPHER_CTX_free(ctx);
    return out;
}

// ─────────────────────────────────────────────────────────────────────────────
// Blowfish block cipher (ECB mode, zero-padded) — EVP_bf_ecb()
// Block size = 8 bytes. Key length: 4–56 bytes (variable).
// ─────────────────────────────────────────────────────────────────────────────

std::vector<uint8_t> blowfish_encrypt(const uint8_t *key, size_t key_len,
                                      const uint8_t *data, size_t data_len) {
    if (key_len == 0) ThrowFakeluaException("blowfish_encrypt: empty key");
    if (key_len > static_cast<size_t>(INT_MAX))
        ThrowFakeluaException("blowfish_encrypt: key too long");

    size_t padded = (data_len + 7) & ~size_t(7);
    if (padded == 0) padded = 8;
    std::vector<uint8_t> padded_data(data, data + data_len);
    padded_data.resize(padded, 0);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) ThrowFakeluaException("blowfish_encrypt: failed to create EVP_CIPHER_CTX");

    if (EVP_EncryptInit_ex(ctx, EVP_bf_ecb(), nullptr, nullptr, nullptr) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("blowfish_encrypt: EVP_EncryptInit_ex (cipher) failed");
    }
    if (EVP_CIPHER_CTX_set_key_length(ctx, static_cast<int>(key_len)) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("blowfish_encrypt: EVP_CIPHER_CTX_set_key_length failed");
    }
    if (EVP_EncryptInit_ex(ctx, nullptr, nullptr, key, nullptr) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("blowfish_encrypt: EVP_EncryptInit_ex (key) failed");
    }
    if (EVP_CIPHER_CTX_set_padding(ctx, 0) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("blowfish_encrypt: failed to disable padding");
    }

    std::vector<uint8_t> out(padded);
    int outlen = 0;
    if (EVP_EncryptUpdate(ctx, out.data(), &outlen,
                          padded_data.data(), static_cast<int>(padded)) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("blowfish_encrypt: EVP_EncryptUpdate failed");
    }
    int final_len = 0;
    if (EVP_EncryptFinal_ex(ctx, out.data() + outlen, &final_len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("blowfish_encrypt: EVP_EncryptFinal_ex failed");
    }

    EVP_CIPHER_CTX_free(ctx);
    out.resize(outlen + final_len);
    return out;
}

std::vector<uint8_t> blowfish_decrypt(const uint8_t *key, size_t key_len,
                                      const uint8_t *data, size_t data_len) {
    if (key_len == 0) ThrowFakeluaException("blowfish_decrypt: empty key");
    if (key_len > static_cast<size_t>(INT_MAX))
        ThrowFakeluaException("blowfish_decrypt: key too long");
    if (data_len % 8 != 0)
        ThrowFakeluaException("blowfish_decrypt: length must be a multiple of 8");

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) ThrowFakeluaException("blowfish_decrypt: failed to create EVP_CIPHER_CTX");

    if (EVP_DecryptInit_ex(ctx, EVP_bf_ecb(), nullptr, nullptr, nullptr) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("blowfish_decrypt: EVP_DecryptInit_ex (cipher) failed");
    }
    if (EVP_CIPHER_CTX_set_key_length(ctx, static_cast<int>(key_len)) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("blowfish_decrypt: EVP_CIPHER_CTX_set_key_length failed");
    }
    if (EVP_DecryptInit_ex(ctx, nullptr, nullptr, key, nullptr) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("blowfish_decrypt: EVP_DecryptInit_ex (key) failed");
    }
    if (EVP_CIPHER_CTX_set_padding(ctx, 0) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("blowfish_decrypt: failed to disable padding");
    }

    std::vector<uint8_t> out(data_len);
    int outlen = 0;
    if (EVP_DecryptUpdate(ctx, out.data(), &outlen,
                          data, static_cast<int>(data_len)) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("blowfish_decrypt: EVP_DecryptUpdate failed");
    }
    int final_len = 0;
    if (EVP_DecryptFinal_ex(ctx, out.data() + outlen, &final_len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("blowfish_decrypt: EVP_DecryptFinal_ex failed");
    }

    EVP_CIPHER_CTX_free(ctx);
    out.resize(outlen + final_len);
    return out;
}

// ─────────────────────────────────────────────────────────────────────────────
// DES block cipher (ECB mode, zero-padded) — EVP_des_ecb()
// Only the first 8 bytes of key are used (56-bit effective key).
// ─────────────────────────────────────────────────────────────────────────────

std::vector<uint8_t> des_encrypt(const uint8_t *key, size_t key_len,
                                 const uint8_t *data, size_t data_len) {
    if (key_len < 8) ThrowFakeluaException("des_encrypt: key must be at least 8 bytes");

    size_t padded = (data_len + 7) & ~size_t(7);
    if (padded == 0) padded = 8;
    std::vector<uint8_t> padded_data(data, data + data_len);
    padded_data.resize(padded, 0);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) ThrowFakeluaException("des_encrypt: failed to create EVP_CIPHER_CTX");

    if (EVP_EncryptInit_ex(ctx, EVP_des_ecb(), nullptr, key, nullptr) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("des_encrypt: EVP_EncryptInit_ex failed");
    }
    if (EVP_CIPHER_CTX_set_padding(ctx, 0) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("des_encrypt: failed to disable padding");
    }

    std::vector<uint8_t> out(padded);
    int outlen = 0;
    if (EVP_EncryptUpdate(ctx, out.data(), &outlen,
                          padded_data.data(), static_cast<int>(padded)) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("des_encrypt: EVP_EncryptUpdate failed");
    }
    int final_len = 0;
    if (EVP_EncryptFinal_ex(ctx, out.data() + outlen, &final_len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("des_encrypt: EVP_EncryptFinal_ex failed");
    }

    EVP_CIPHER_CTX_free(ctx);
    out.resize(outlen + final_len);
    return out;
}

std::vector<uint8_t> des_decrypt(const uint8_t *key, size_t key_len,
                                 const uint8_t *data, size_t data_len) {
    if (key_len < 8) ThrowFakeluaException("des_decrypt: key must be at least 8 bytes");
    if (data_len % 8 != 0) ThrowFakeluaException("des_decrypt: length must be a multiple of 8");

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) ThrowFakeluaException("des_decrypt: failed to create EVP_CIPHER_CTX");

    if (EVP_DecryptInit_ex(ctx, EVP_des_ecb(), nullptr, key, nullptr) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("des_decrypt: EVP_DecryptInit_ex failed");
    }
    if (EVP_CIPHER_CTX_set_padding(ctx, 0) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("des_decrypt: failed to disable padding");
    }

    std::vector<uint8_t> out(data_len);
    int outlen = 0;
    if (EVP_DecryptUpdate(ctx, out.data(), &outlen,
                          data, static_cast<int>(data_len)) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("des_decrypt: EVP_DecryptUpdate failed");
    }
    int final_len = 0;
    if (EVP_DecryptFinal_ex(ctx, out.data() + outlen, &final_len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("des_decrypt: EVP_DecryptFinal_ex failed");
    }

    EVP_CIPHER_CTX_free(ctx);
    out.resize(outlen + final_len);
    return out;
}

// ─────────────────────────────────────────────────────────────────────────────
// Triple DES (DES-EDE3) block cipher (ECB mode, zero-padded) — EVP_des_ede3_ecb()
// EDE is handled internally by OpenSSL. Key = 24 bytes.
// ─────────────────────────────────────────────────────────────────────────────

std::vector<uint8_t> triple_des_encrypt(const uint8_t *key, size_t key_len,
                                        const uint8_t *data, size_t data_len) {
    if (key_len < 24) ThrowFakeluaException("triple_des_encrypt: key must be at least 24 bytes");

    size_t padded = (data_len + 7) & ~size_t(7);
    if (padded == 0) padded = 8;
    std::vector<uint8_t> padded_data(data, data + data_len);
    padded_data.resize(padded, 0);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) ThrowFakeluaException("triple_des_encrypt: failed to create EVP_CIPHER_CTX");

    if (EVP_EncryptInit_ex(ctx, EVP_des_ede3_ecb(), nullptr, key, nullptr) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("triple_des_encrypt: EVP_EncryptInit_ex failed");
    }
    if (EVP_CIPHER_CTX_set_padding(ctx, 0) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("triple_des_encrypt: failed to disable padding");
    }

    std::vector<uint8_t> out(padded);
    int outlen = 0;
    if (EVP_EncryptUpdate(ctx, out.data(), &outlen,
                          padded_data.data(), static_cast<int>(padded)) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("triple_des_encrypt: EVP_EncryptUpdate failed");
    }
    int final_len = 0;
    if (EVP_EncryptFinal_ex(ctx, out.data() + outlen, &final_len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("triple_des_encrypt: EVP_EncryptFinal_ex failed");
    }

    EVP_CIPHER_CTX_free(ctx);
    out.resize(outlen + final_len);
    return out;
}

std::vector<uint8_t> triple_des_decrypt(const uint8_t *key, size_t key_len,
                                        const uint8_t *data, size_t data_len) {
    if (key_len < 24) ThrowFakeluaException("triple_des_decrypt: key must be at least 24 bytes");
    if (data_len % 8 != 0) ThrowFakeluaException("triple_des_decrypt: length must be a multiple of 8");

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) ThrowFakeluaException("triple_des_decrypt: failed to create EVP_CIPHER_CTX");

    if (EVP_DecryptInit_ex(ctx, EVP_des_ede3_ecb(), nullptr, key, nullptr) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("triple_des_decrypt: EVP_DecryptInit_ex failed");
    }
    if (EVP_CIPHER_CTX_set_padding(ctx, 0) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("triple_des_decrypt: failed to disable padding");
    }

    std::vector<uint8_t> out(data_len);
    int outlen = 0;
    if (EVP_DecryptUpdate(ctx, out.data(), &outlen,
                          data, static_cast<int>(data_len)) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("triple_des_decrypt: EVP_DecryptUpdate failed");
    }
    int final_len = 0;
    if (EVP_DecryptFinal_ex(ctx, out.data() + outlen, &final_len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException("triple_des_decrypt: EVP_DecryptFinal_ex failed");
    }

    EVP_CIPHER_CTX_free(ctx);
    out.resize(outlen + final_len);
    return out;
}

}  // namespace fakelua::crypto
