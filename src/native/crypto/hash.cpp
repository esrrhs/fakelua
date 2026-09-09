#include "hash.h"
#include "native/native_common.h"
#include "util/exception.h"

#include <climits>
#include <cstring>
#include <string>
#include <openssl/evp.h>

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
    if (!ctx) ThrowFakeluaException("md5: failed to create EVP_MD_CTX");
    if (EVP_DigestInit_ex(ctx, EVP_md5(), nullptr) != 1) {
        EVP_MD_CTX_free(ctx);
        ThrowFakeluaException("md5: EVP_DigestInit_ex failed");
    }
    if (EVP_DigestUpdate(ctx, data, len) != 1) {
        EVP_MD_CTX_free(ctx);
        ThrowFakeluaException("md5: EVP_DigestUpdate failed");
    }
    unsigned int outlen = 0;
    if (EVP_DigestFinal_ex(ctx, out.data(), &outlen) != 1) {
        EVP_MD_CTX_free(ctx);
        ThrowFakeluaException("md5: EVP_DigestFinal_ex failed");
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
    if (!ctx) ThrowFakeluaException("sha1: failed to create EVP_MD_CTX");
    if (EVP_DigestInit_ex(ctx, EVP_sha1(), nullptr) != 1) {
        EVP_MD_CTX_free(ctx);
        ThrowFakeluaException("sha1: EVP_DigestInit_ex failed");
    }
    if (EVP_DigestUpdate(ctx, data, len) != 1) {
        EVP_MD_CTX_free(ctx);
        ThrowFakeluaException("sha1: EVP_DigestUpdate failed");
    }
    unsigned int outlen = 0;
    if (EVP_DigestFinal_ex(ctx, out.data(), &outlen) != 1) {
        EVP_MD_CTX_free(ctx);
        ThrowFakeluaException("sha1: EVP_DigestFinal_ex failed");
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
    if (!ctx) ThrowFakeluaException("sha256: failed to create EVP_MD_CTX");
    if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1) {
        EVP_MD_CTX_free(ctx);
        ThrowFakeluaException("sha256: EVP_DigestInit_ex failed");
    }
    if (EVP_DigestUpdate(ctx, data, len) != 1) {
        EVP_MD_CTX_free(ctx);
        ThrowFakeluaException("sha256: EVP_DigestUpdate failed");
    }
    unsigned int outlen = 0;
    if (EVP_DigestFinal_ex(ctx, out.data(), &outlen) != 1) {
        EVP_MD_CTX_free(ctx);
        ThrowFakeluaException("sha256: EVP_DigestFinal_ex failed");
    }
    EVP_MD_CTX_free(ctx);
    return out;
}

// ─────────────────────────────────────────────────────────────────────────────
// RC4 stream cipher — migrated to EVP_rc4() (no legacy provider required)
// ─────────────────────────────────────────────────────────────────────────────

std::vector<uint8_t> rc4(const uint8_t *key, size_t key_len,
                         const uint8_t *data, size_t data_len) {
    if (key_len == 0) ThrowFakeluaException("rc4: empty key");
    // EVP_rc4 accepts key length up to EVP_MAX_KEY_LENGTH; guard against overflow
    // when converting size_t to int (used internally by EVP_CIPHER_CTX_set_key_length).
    if (key_len > static_cast<size_t>(INT_MAX))
        ThrowFakeluaException("rc4: key too long");

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) ThrowFakeluaException("rc4: failed to create EVP_CIPHER_CTX");

    // EVP_rc4() uses a 128-bit key by default; EVP_CIPHER_CTX_set_key_length
    // lets us use any length from 1 to 256 bytes.
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
    // RC4 is a stream cipher; Final produces no additional bytes.
    int final_len = 0;
    EVP_EncryptFinal_ex(ctx, out.data() + outlen, &final_len);

    EVP_CIPHER_CTX_free(ctx);
    return out;
}

// ─────────────────────────────────────────────────────────────────────────────
// Blowfish block cipher (ECB mode, zero-padded) — migrated to EVP_bf_ecb()
// Block size = 8 bytes. Data zero-padded to a multiple of 8.
// ─────────────────────────────────────────────────────────────────────────────

std::vector<uint8_t> blowfish_encrypt(const uint8_t *key, size_t key_len,
                                      const uint8_t *data, size_t data_len) {
    if (key_len == 0) ThrowFakeluaException("blowfish_encrypt: empty key");
    if (key_len > static_cast<size_t>(INT_MAX))
        ThrowFakeluaException("blowfish_encrypt: key too long");

    // Zero-pad data to multiple of 8 (Blowfish block size)
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
    // Disable EVP padding: we handle zero-padding ourselves so each input chunk
    // is already an exact multiple of the block size.
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
// DES block cipher (ECB mode, zero-padded) — migrated to EVP_des_ecb()
// Block size = 8 bytes. Only the first 8 bytes of key are used.
// ─────────────────────────────────────────────────────────────────────────────

std::vector<uint8_t> des_encrypt(const uint8_t *key, size_t key_len,
                                 const uint8_t *data, size_t data_len) {
    if (key_len < 8) ThrowFakeluaException("des_encrypt: key must be at least 8 bytes");

    // Zero-pad to multiple of 8
    size_t padded = (data_len + 7) & ~size_t(7);
    if (padded == 0) padded = 8;
    std::vector<uint8_t> padded_data(data, data + data_len);
    padded_data.resize(padded, 0);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) ThrowFakeluaException("des_encrypt: failed to create EVP_CIPHER_CTX");

    // EVP_des_ecb uses exactly 8 bytes of key (56-bit effective key).
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
// Triple DES (3DES / DES-EDE) block cipher (ECB mode, zero-padded)
// Migrated to EVP_des_ede3_ecb() which handles EDE internally.
// Block size = 8 bytes. Key = exactly 24 bytes (three 8-byte sub-keys).
// ─────────────────────────────────────────────────────────────────────────────

std::vector<uint8_t> triple_des_encrypt(const uint8_t *key, size_t key_len,
                                        const uint8_t *data, size_t data_len) {
    if (key_len < 24) ThrowFakeluaException("triple_des_encrypt: key must be at least 24 bytes");

    // Zero-pad to multiple of 8
    size_t padded = (data_len + 7) & ~size_t(7);
    if (padded == 0) padded = 8;
    std::vector<uint8_t> padded_data(data, data + data_len);
    padded_data.resize(padded, 0);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) ThrowFakeluaException("triple_des_encrypt: failed to create EVP_CIPHER_CTX");

    // EVP_des_ede3_ecb uses 24 bytes of key (encrypt key1, decrypt key2, encrypt key3).
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
