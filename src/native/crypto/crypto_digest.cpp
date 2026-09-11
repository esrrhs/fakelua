#include "crypto_digest.h"
#include "native/native_common.h"
#include "util/exception.h"

#include <boost/algorithm/hex.hpp>
#include <cstring>
#include <iterator>
#include <openssl/evp.h>
#include <string>

namespace fakelua::crypto {

// Hex encoding (lowercase, via Boost.Algorithm)

std::string ToHex(const uint8_t *data, size_t len) {
    std::string out;
    out.reserve(len * 2);
    boost::algorithm::hex_lower(data, data + len, std::back_inserter(out));
    return out;
}

// Base64 encoding/decoding (RFC 4648)

static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string Base64Encode(const uint8_t *data, size_t len) {
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

static int Base64DecodeChar(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

std::string Base64Decode(const uint8_t *data, size_t len) {
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
            n[j] = Base64DecodeChar(static_cast<char>(bytes[i + j]));
            if (n[j] < 0) {
                ThrowFakeluaException("crypto.base64_decode: invalid character");
            }
        }

        uint32_t val = (static_cast<uint32_t>(n[0]) << 18) | (static_cast<uint32_t>(n[1]) << 12) | (static_cast<uint32_t>(n[2]) << 6) | static_cast<uint32_t>(n[3]);

        out.push_back(static_cast<char>((val >> 16) & 0xFF));
        if (i + 2 < effective_len) out.push_back(static_cast<char>((val >> 8) & 0xFF));
        if (i + 3 < effective_len) out.push_back(static_cast<char>(val & 0xFF));
    }

    return out;
}

// MD5

std::array<uint8_t, 16> Md5(const uint8_t *data, size_t len) {
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

// SHA1

std::array<uint8_t, 20> Sha1(const uint8_t *data, size_t len) {
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

// SHA256

std::array<uint8_t, 32> Sha256(const uint8_t *data, size_t len) {
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

}// namespace fakelua::crypto
