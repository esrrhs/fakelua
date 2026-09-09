#include "hash.h"
#include "hash.h"
#include "native/native_common.h"
#include "util/exception.h"

#include <cstring>
#include <string>
#include <openssl/evp.h>
#include <openssl/rc4.h>
#include <openssl/blowfish.h>
#include <openssl/des.h>

namespace fakelua::crypto {

// ─────────────────────────────────────────────────────────────────────────────
// Hex encoding
// ─────────────────────────────────────────────────────────────────────────────

std::string to_hex(const uint8_t *data, size_t len) {
    static const char digits[] = "0123456789abcdef";
    std::string out;
    out.reserve(len * 2);
    for (size_t i = 0; i < len; ++i) {
        out.push_back(digits[(data[i] >> 4) & 0xF]);
        out.push_back(digits[data[i] & 0xF]);
    }
    return out;
}

// ─────────────────────────────────────────────────────────────────────────────
// Base64 encoding/decoding (RFC 4648)
// ─────────────────────────────────────────────────────────────────────────────

static const char base64_chars[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string base64_encode(const uint8_t *data, size_t len) {
    std::string out;
    out.reserve(((len + 2) / 3) * 4);

    for (size_t i = 0; i < len; i += 3) {
        uint32_t n = static_cast<uint32_t>(data[i]) << 16;
        if (i + 1 < len) n |= static_cast<uint32_t>(data[i + 1]) << 8;
        if (i + 2 < len) n |= static_cast<uint32_t>(data[i + 2]);

        out.push_back(base64_chars[(n >> 18) & 0x3F]);
        out.push_back(base64_chars[(n >> 12) & 0x3F]);
        out.push_back((i + 1 < len) ? base64_chars[(n >> 6) & 0x3F] : '=');
        out.push_back((i + 2 < len) ? base64_chars[n & 0x3F] : '=');
    }

    return out;
}

static int base64_decode_char(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

std::string base64_decode(const uint8_t *data, size_t len) {
    std::string filtered;
    filtered.reserve(len);
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = data[i];
        if (c == ' ' || c == '\n' || c == '\r' || c == '\t') continue;
        filtered.push_back(static_cast<char>(c));
    }

    const uint8_t *bytes = reinterpret_cast<const uint8_t *>(filtered.data());
    size_t nlen = filtered.size();

    size_t effective_len = nlen;
    while (effective_len > 0 && bytes[effective_len - 1] == '=') {
        effective_len--;
    }
    if (effective_len % 4 == 1) {
        ThrowFakeluaException("crypto.base64_decode: invalid input");
    }

    std::string out;
    out.reserve((effective_len * 3) / 4);

    for (size_t i = 0; i < effective_len; i += 4) {
        int n[4] = {0, 0, 0, 0};
        for (int j = 0; j < 4 && i + j < effective_len; ++j) {
            n[j] = base64_decode_char(static_cast<char>(bytes[i + j]));
            if (n[j] < 0) {
                ThrowFakeluaException("crypto.base64_decode: invalid character");
            }
        }

        uint32_t val = (static_cast<uint32_t>(n[0]) << 18) |
                       (static_cast<uint32_t>(n[1]) << 12) |
                       (static_cast<uint32_t>(n[2]) << 6) |
                       static_cast<uint32_t>(n[3]);

        out.push_back(static_cast<char>((val >> 16) & 0xFF));
        if (i + 2 < effective_len) out.push_back(static_cast<char>((val >> 8) & 0xFF));
        if (i + 3 < effective_len) out.push_back(static_cast<char>(val & 0xFF));
    }

    return out;
}

// ─────────────────────────────────────────────────────────────────────────────
// MD5
// ─────────────────────────────────────────────────────────────────────────────

std::array<uint8_t, 16> md5(const uint8_t *data, size_t len) {
    std::array<uint8_t, 16> out;
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (!ctx) ThrowFakeluaException("Failed to create EVP_MD_CTX");
    if (EVP_DigestInit_ex(ctx, EVP_md5(), nullptr) != 1) {
        EVP_MD_CTX_free(ctx);
        ThrowFakeluaException("Failed to initialize MD5");
    }
    if (EVP_DigestUpdate(ctx, data, len) != 1) {
        EVP_MD_CTX_free(ctx);
        ThrowFakeluaException("Failed to update MD5");
    }
    unsigned int outlen = 0;
    if (EVP_DigestFinal_ex(ctx, out.data(), &outlen) != 1) {
        EVP_MD_CTX_free(ctx);
        ThrowFakeluaException("Failed to finalize MD5");
    }
    EVP_MD_CTX_free(ctx);
    return out;
}

// ─────────────────────────────────────────────────────────────────────────────
// SHA1
// ─────────────────────────────────────────────────────────────────────────────

std::array<uint8_t, 20> sha1(const uint8_t *data, size_t len) {
    std::array<uint8_t, 20> out;
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (!ctx) ThrowFakeluaException("Failed to create EVP_MD_CTX");
    if (EVP_DigestInit_ex(ctx, EVP_sha1(), nullptr) != 1) {
        EVP_MD_CTX_free(ctx);
        ThrowFakeluaException("Failed to initialize SHA1");
    }
    if (EVP_DigestUpdate(ctx, data, len) != 1) {
        EVP_MD_CTX_free(ctx);
        ThrowFakeluaException("Failed to update SHA1");
    }
    unsigned int outlen = 0;
    if (EVP_DigestFinal_ex(ctx, out.data(), &outlen) != 1) {
        EVP_MD_CTX_free(ctx);
        ThrowFakeluaException("Failed to finalize SHA1");
    }
    EVP_MD_CTX_free(ctx);
    return out;
}

// ─────────────────────────────────────────────────────────────────────────────
// SHA256
// ─────────────────────────────────────────────────────────────────────────────

std::array<uint8_t, 32> sha256(const uint8_t *data, size_t len) {
    std::array<uint8_t, 32> out;
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (!ctx) ThrowFakeluaException("Failed to create EVP_MD_CTX");
    if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1) {
        EVP_MD_CTX_free(ctx);
        ThrowFakeluaException("Failed to initialize SHA256");
    }
    if (EVP_DigestUpdate(ctx, data, len) != 1) {
        EVP_MD_CTX_free(ctx);
        ThrowFakeluaException("Failed to update SHA256");
    }
    unsigned int outlen = 0;
    if (EVP_DigestFinal_ex(ctx, out.data(), &outlen) != 1) {
        EVP_MD_CTX_free(ctx);
        ThrowFakeluaException("Failed to finalize SHA256");
    }
    EVP_MD_CTX_free(ctx);
    return out;
}

// ─────────────────────────────────────────────────────────────────────────────
// RC4 stream cipher
// ─────────────────────────────────────────────────────────────────────────────

std::vector<uint8_t> rc4(const uint8_t *key, size_t key_len,
                         const uint8_t *data, size_t data_len) {
    if (key_len == 0) ThrowFakeluaException("rc4: empty key");
    RC4_KEY rc4_key;
    RC4_set_key(&rc4_key, key_len, key);
    std::vector<uint8_t> out(data_len);
    RC4(&rc4_key, data_len, data, out.data());
    return out;
}

// ────────────────……………………………………………………………………
// Blowfish block cipher (ECB mode, zero-padded)
// ────────────────……………………………………………………………………

std::vector<uint8_t> blowfish_encrypt(const uint8_t *key, size_t key_len,
                                      const uint8_t *data, size_t data_len) {
    if (key_len == 0) ThrowFakeluaException("blowfish_encrypt: empty key");
    BF_KEY blowfish_key;
    BF_set_key(&blowfish_key, key_len, key);

    // Zero-pad data to multiple of 8
    size_t padded = (data_len + 7) & ~size_t(7);
    std::vector<uint8_t> padded_data(data, data + data_len);
    padded_data.resize(padded, 0);

    std::vector<uint8_t> out(padded);
    for (size_t i = 0; i < padded; i += 8) {
        BF_LONG block[2];
        block[0] = (static_cast<BF_LONG>(padded_data[i]) << 24) |
                   (static_cast<BF_LONG>(padded_data[i + 1]) << 16) |
                   (static_cast<BF_LONG>(padded_data[i + 2]) << 8) |
                   static_cast<BF_LONG>(padded_data[i + 3]);
        block[1] = (static_cast<BF_LONG>(padded_data[i + 4]) << 24) |
                   (static_cast<BF_LONG>(padded_data[i + 5]) << 16) |
                   (static_cast<BF_LONG>(padded_data[i + 6]) << 8) |
                   static_cast<BF_LONG>(padded_data[i + 7]);
        BF_encrypt(block, &blowfish_key);
        out[i] = static_cast<uint8_t>((block[0] >> 24) & 0xFF);
        out[i + 1] = static_cast<uint8_t>((block[0] >> 16) & 0xFF);
        out[i + 2] = static_cast<uint8_t>((block[0] >> 8) & 0xFF);
        out[i + 3] = static_cast<uint8_t>(block[0] & 0xFF);
        out[i + 4] = static_cast<uint8_t>((block[1] >> 24) & 0xFF);
        out[i + 5] = static_cast<uint8_t>((block[1] >> 16) & 0xFF);
        out[i + 6] = static_cast<uint8_t>((block[1] >> 8) & 0xFF);
        out[i + 7] = static_cast<uint8_t>(block[1] & 0xFF);
    }
    return out;
}

std::vector<uint8_t> blowfish_decrypt(const uint8_t *key, size_t key_len,
                                      const uint8_t *data, size_t data_len) {
    if (key_len == 0) ThrowFakeluaException("blowfish_decrypt: empty key");
    if (data_len % 8 != 0) ThrowFakeluaException("blowfish_decrypt: length must be a multiple of 8");
    BF_KEY blowfish_key;
    BF_set_key(&blowfish_key, key_len, key);

    std::vector<uint8_t> out(data_len);
    for (size_t i = 0; i < data_len; i += 8) {
        BF_LONG block[2];
        block[0] = (static_cast<BF_LONG>(data[i]) << 24) |
                   (static_cast<BF_LONG>(data[i + 1]) << 16) |
                   (static_cast<BF_LONG>(data[i + 2]) << 8) |
                   static_cast<BF_LONG>(data[i + 3]);
        block[1] = (static_cast<BF_LONG>(data[i + 4]) << 24) |
                   (static_cast<BF_LONG>(data[i + 5]) << 16) |
                   (static_cast<BF_LONG>(data[i + 6]) << 8) |
                   static_cast<BF_LONG>(data[i + 7]);
        BF_decrypt(block, &blowfish_key);
        out[i] = static_cast<uint8_t>((block[0] >> 24) & 0xFF);
        out[i + 1] = static_cast<uint8_t>((block[0] >> 16) & 0xFF);
        out[i + 2] = static_cast<uint8_t>((block[0] >> 8) & 0xFF);
        out[i + 3] = static_cast<uint8_t>(block[0] & 0xFF);
        out[i + 4] = static_cast<uint8_t>((block[1] >> 24) & 0xFF);
        out[i + 5] = static_cast<uint8_t>((block[1] >> 16) & 0xFF);
        out[i + 6] = static_cast<uint8_t>((block[1] >> 8) & 0xFF);
        out[i + 7] = static_cast<uint8_t>(block[1] & 0xFF);
    }
    return out;
}

// ─────────────────────────────────────────────────────────────────────────────
// DES block cipher (ECB mode, zero-padded)
// ─────────────────────────────────────────────────────────────────────────────

std::vector<uint8_t> des_encrypt(const uint8_t *key, size_t key_len,
                                 const uint8_t *data, size_t data_len) {
    if (key_len < 8) ThrowFakeluaException("des_encrypt: key must be at least 8 bytes");

    // Use only the first 8 bytes of the key
    DES_key_schedule schedule;
    DES_cblock key_block;
    std::memcpy(key_block, key, 8);
    DES_set_key_unchecked(&key_block, &schedule);

    // Zero-pad to multiple of 8
    size_t padded = (data_len + 7) & ~size_t(7);
    std::vector<uint8_t> padded_data(data, data + data_len);
    padded_data.resize(padded, 0);

    std::vector<uint8_t> out(padded);
    for (size_t i = 0; i < padded; i += 8) {
        DES_cblock block;
        std::memcpy(block, padded_data.data() + i, 8);
        DES_ecb_encrypt(&block, &block, &schedule, DES_ENCRYPT);
        std::memcpy(out.data() + i, block, 8);
    }
    return out;
}

std::vector<uint8_t> des_decrypt(const uint8_t *key, size_t key_len,
                                 const uint8_t *data, size_t data_len) {
    if (key_len < 8) ThrowFakeluaException("des_decrypt: key must be at least 8 bytes");
    if (data_len % 8 != 0) ThrowFakeluaException("des_decrypt: length must be a multiple of 8");

    // Use only the first 8 bytes of the key
    DES_key_schedule schedule;
    DES_cblock key_block;
    std::memcpy(key_block, key, 8);
    DES_set_key_unchecked(&key_block, &schedule);

    std::vector<uint8_t> out(data_len);
    for (size_t i = 0; i < data_len; i += 8) {
        DES_cblock block;
        std::memcpy(block, data + i, 8);
        DES_ecb_encrypt(&block, &block, &schedule, DES_DECRYPT);
        std::memcpy(out.data() + i, block, 8);
    }
    return out;
}

// ────────────────……………………………………………………………………
// Triple DES (3DES) block cipher (ECB mode, zero-padded)
// ────────────────……………………………………………………………………

std::vector<uint8_t> triple_des_encrypt(const uint8_t *key, size_t key_len,
                                        const uint8_t *data, size_t data_len) {
    if (key_len < 24) ThrowFakeluaException("triple_des_encrypt: key must be at least 24 bytes");

    // Use only the first 24 bytes of the key (three 8-byte keys)
    DES_key_schedule schedule1, schedule2, schedule3;
    DES_cblock key1, key2, key3;
    std::memcpy(key1, key, 8);
    std::memcpy(key2, key + 8, 8);
    std::memcpy(key3, key + 16, 8);
    DES_set_key_unchecked(&key1, &schedule1);
    DES_set_key_unchecked(&key2, &schedule2);
    DES_set_key_unchecked(&key3, &schedule3);

    // Zero-pad to multiple of 8
    size_t padded = (data_len + 7) & ~size_t(7);
    std::vector<uint8_t> padded_data(data, data + data_len);
    padded_data.resize(padded, 0);

    std::vector<uint8_t> out(padded);
    for (size_t i = 0; i < padded; i += 8) {
        DES_cblock block;
        std::memcpy(block, padded_data.data() + i, 8);
        // 3DES encrypt: encrypt with key1, decrypt with key2, encrypt with key3
        DES_ecb_encrypt(&block, &block, &schedule1, DES_ENCRYPT);
        DES_ecb_encrypt(&block, &block, &schedule2, DES_DECRYPT);
        DES_ecb_encrypt(&block, &block, &schedule3, DES_ENCRYPT);
        std::memcpy(out.data() + i, block, 8);
    }
    return out;
}

std::vector<uint8_t> triple_des_decrypt(const uint8_t *key, size_t key_len,
                                        const uint8_t *data, size_t data_len) {
    if (key_len < 24) ThrowFakeluaException("triple_des_decrypt: key must be at least 24 bytes");
    if (data_len % 8 != 0) ThrowFakeluaException("triple_des_decrypt: length must be a multiple of 8");

    // Use only the first 24 bytes of the key (three 8-byte keys)
    DES_key_schedule schedule1, schedule2, schedule3;
    DES_cblock key1, key2, key3;
    std::memcpy(key1, key, 8);
    std::memcpy(key2, key + 8, 8);
    std::memcpy(key3, key + 16, 8);
    DES_set_key_unchecked(&key1, &schedule1);
    DES_set_key_unchecked(&key2, &schedule2);
    DES_set_key_unchecked(&key3, &schedule3);

    std::vector<uint8_t> out(data_len);
    for (size_t i = 0; i < data_len; i += 8) {
        DES_cblock block;
        std::memcpy(block, data + i, 8);
        // 3DES decrypt: decrypt with key3, encrypt with key2, decrypt with key1
        DES_ecb_encrypt(&block, &block, &schedule3, DES_DECRYPT);
        DES_ecb_encrypt(&block, &block, &schedule2, DES_ENCRYPT);
        DES_ecb_encrypt(&block, &block, &schedule1, DES_DECRYPT);
        std::memcpy(out.data() + i, block, 8);
    }
    return out;
}

}  // namespace fakelua::crypto