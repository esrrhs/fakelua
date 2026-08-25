#include "native/crypto/hash.h"
#include "native/native_common.h"
#include "util/exception.h"

#include <cstring>

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

static uint32_t md5_left_rotate(uint32_t x, int n) {
    return (x << n) | (x >> (32 - n));
}

std::array<uint8_t, 16> md5(const uint8_t *data, size_t len) {
    // Per-round shift amounts
    static const uint32_t s[64] = {
        7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,
        5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,
        4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,
        6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,
    };

    // Binary integer part of sines of integers (radians) as constants
    static const uint32_t k[64] = {
        0xd76aa478u, 0xe8c7b756u, 0x242070dbu, 0xc1bdceeeu,
        0xf57c0fafu, 0x4787c62au, 0xa8304613u, 0xfd469501u,
        0x698098d8u, 0x8b44f7afu, 0xffff5bb1u, 0x895cd7beu,
        0x6b901122u, 0xfd987193u, 0xa679438eu, 0x49b40821u,
        0xf61e2562u, 0xc040b340u, 0x265e5a51u, 0xe9b6c7aau,
        0xd62f105du, 0x02441453u, 0xd8a1e681u, 0xe7d3fbc8u,
        0x21e1cde6u, 0xc33707d6u, 0xf4d50d87u, 0x455a14edu,
        0xa9e3e905u, 0xfcefa3f8u, 0x676f02d9u, 0x8d2a4c8au,
        0xfffa3942u, 0x8771f681u, 0x6d9d6122u, 0xfde5380cu,
        0xa4beea44u, 0x4bdecfa9u, 0xf6bb4b60u, 0xbebfbc70u,
        0x289b7ec6u, 0xeaa127fau, 0xd4ef3085u, 0x04881d05u,
        0xd9d4d039u, 0xe6db99e5u, 0x1fa27cf8u, 0xc4ac5665u,
        0xf4292244u, 0x432aff97u, 0xab9423a7u, 0xfc93a039u,
        0x655b59c3u, 0x8f0ccc92u, 0xffeff47du, 0x85845dd1u,
        0x6fa87e4fu, 0xfe2ce6e0u, 0xa3014314u, 0x4e0811a1u,
        0xf7537e82u, 0xbd3af235u, 0x2ad7d2bbu, 0xeb86d391u,
    };

    uint32_t a0 = 0x67452301u;
    uint32_t b0 = 0xefcdab89u;
    uint32_t c0 = 0x98badcfeu;
    uint32_t d0 = 0x10325476u;

    // Pre-processing: pad to ≡ 448 bits mod 512 bits
    uint64_t total_bits = static_cast<uint64_t>(len) * 8;
    std::vector<uint8_t> msg(data, data + len);
    msg.push_back(0x80);
    while ((msg.size() % 64) != 56) {
        msg.push_back(0x00);
    }
    // Append length as 64-bit little-endian
    for (int i = 0; i < 8; ++i) {
        msg.push_back(static_cast<uint8_t>((total_bits >> (8 * i)) & 0xFF));
    }

    // Process each 64-byte block
    for (size_t offset = 0; offset < msg.size(); offset += 64) {
        uint32_t m[16];
        for (int i = 0; i < 16; ++i) {
            m[i] = static_cast<uint32_t>(msg[offset + i * 4]) |
                   (static_cast<uint32_t>(msg[offset + i * 4 + 1]) << 8) |
                   (static_cast<uint32_t>(msg[offset + i * 4 + 2]) << 16) |
                   (static_cast<uint32_t>(msg[offset + i * 4 + 3]) << 24);
        }

        uint32_t a = a0, b = b0, c = c0, d = d0;
        for (int i = 0; i < 64; ++i) {
            uint32_t f, g;
            if (i < 16) {
                f = (b & c) | ((~b) & d);
                g = i;
            } else if (i < 32) {
                f = (d & b) | ((~d) & c);
                g = (5 * i + 1) % 16;
            } else if (i < 48) {
                f = b ^ c ^ d;
                g = (3 * i + 5) % 16;
            } else {
                f = c ^ (b | (~d));
                g = (7 * i) % 16;
            }
            f = f + a + k[i] + m[g];
            a = d;
            d = c;
            c = b;
            b = b + md5_left_rotate(f, s[i]);
        }
        a0 += a;
        b0 += b;
        c0 += c;
        d0 += d;
    }

    std::array<uint8_t, 16> out{};
    auto put32 = [&](int idx, uint32_t v) {
        out[idx]     = static_cast<uint8_t>(v & 0xFF);
        out[idx + 1] = static_cast<uint8_t>((v >> 8) & 0xFF);
        out[idx + 2] = static_cast<uint8_t>((v >> 16) & 0xFF);
        out[idx + 3] = static_cast<uint8_t>((v >> 24) & 0xFF);
    };
    put32(0, a0);
    put32(4, b0);
    put32(8, c0);
    put32(12, d0);
    return out;
}

// ─────────────────────────────────────────────────────────────────────────────
// SHA1
// ─────────────────────────────────────────────────────────────────────────────

static uint32_t sha1_left_rotate(uint32_t x, int n) {
    return (x << n) | (x >> (32 - n));
}

std::array<uint8_t, 20> sha1(const uint8_t *data, size_t len) {
    uint32_t h0 = 0x67452301u;
    uint32_t h1 = 0xEFCDAB89u;
    uint32_t h2 = 0x98BADCFEu;
    uint32_t h3 = 0x10325476u;
    uint32_t h4 = 0xC3D2E1F0u;

    uint64_t total_bits = static_cast<uint64_t>(len) * 8;
    std::vector<uint8_t> msg(data, data + len);
    msg.push_back(0x80);
    while ((msg.size() % 64) != 56) {
        msg.push_back(0x00);
    }
    for (int i = 7; i >= 0; --i) {
        msg.push_back(static_cast<uint8_t>((total_bits >> (8 * i)) & 0xFF));
    }

    for (size_t offset = 0; offset < msg.size(); offset += 64) {
        uint32_t w[80];
        for (int i = 0; i < 16; ++i) {
            w[i] = (static_cast<uint32_t>(msg[offset + i * 4]) << 24) |
                   (static_cast<uint32_t>(msg[offset + i * 4 + 1]) << 16) |
                   (static_cast<uint32_t>(msg[offset + i * 4 + 2]) << 8) |
                   (static_cast<uint32_t>(msg[offset + i * 4 + 3]));
        }
        for (int i = 16; i < 80; ++i) {
            w[i] = sha1_left_rotate(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
        }

        uint32_t a = h0, b = h1, c = h2, d = h3, e = h4;
        for (int i = 0; i < 80; ++i) {
            uint32_t f, k;
            if (i < 20) {
                f = (b & c) | ((~b) & d);
                k = 0x5A827999u;
            } else if (i < 40) {
                f = b ^ c ^ d;
                k = 0x6ED9EBA1u;
            } else if (i < 60) {
                f = (b & c) | (b & d) | (c & d);
                k = 0x8F1BBCDCu;
            } else {
                f = b ^ c ^ d;
                k = 0xCA62C1D6u;
            }
            uint32_t temp = sha1_left_rotate(a, 5) + f + e + k + w[i];
            e = d;
            d = c;
            c = sha1_left_rotate(b, 30);
            b = a;
            a = temp;
        }
        h0 += a;
        h1 += b;
        h2 += c;
        h3 += d;
        h4 += e;
    }

    std::array<uint8_t, 20> out{};
    auto put32 = [&](int idx, uint32_t v) {
        out[idx]     = static_cast<uint8_t>((v >> 24) & 0xFF);
        out[idx + 1] = static_cast<uint8_t>((v >> 16) & 0xFF);
        out[idx + 2] = static_cast<uint8_t>((v >> 8) & 0xFF);
        out[idx + 3] = static_cast<uint8_t>(v & 0xFF);
    };
    put32(0, h0);
    put32(4, h1);
    put32(8, h2);
    put32(12, h3);
    put32(16, h4);
    return out;
}

// ─────────────────────────────────────────────────────────────────────────────
// SHA256
// ─────────────────────────────────────────────────────────────────────────────

static const uint32_t sha256_k[64] = {
    0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u,
    0x3956c25bu, 0x59f111f1u, 0x923f82a4u, 0xab1c5ed5u,
    0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u,
    0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u,
    0xe49b69c1u, 0xefbe4786u, 0x0fc19dc6u, 0x240ca1ccu,
    0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau,
    0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u,
    0xc6e00bf3u, 0xd5a79147u, 0x06ca6351u, 0x14292967u,
    0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u,
    0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u,
    0xa2bfe8a1u, 0xa81a664bu, 0xc24b8b70u, 0xc76c51a3u,
    0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
    0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u,
    0x391c0cb3u, 0x4ed8aa4au, 0x5b9cca4fu, 0x682e6ff3u,
    0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u,
    0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u,
};

// SHA256 rotation helpers
static uint32_t sha256_rotr(uint32_t x, int n) {
    return (x >> n) | (x << (32 - n));
}

// SHA256 round-function mixers (EP0/EP1 = uppercase Sigma in FIPS 180-4)
static uint32_t sha256_ep0(uint32_t x) {
    return sha256_rotr(x, 2) ^ sha256_rotr(x, 13) ^ sha256_rotr(x, 22);
}
static uint32_t sha256_ep1(uint32_t x) {
    return sha256_rotr(x, 6) ^ sha256_rotr(x, 11) ^ sha256_rotr(x, 25);
}

// SHA256 message-schedule mixers (SIG0/SIG1 = lowercase sigma in FIPS 180-4)
static uint32_t sha256_sig0(uint32_t x) {
    return sha256_rotr(x, 7) ^ sha256_rotr(x, 18) ^ (x >> 3);
}
static uint32_t sha256_sig1(uint32_t x) {
    return sha256_rotr(x, 17) ^ sha256_rotr(x, 19) ^ (x >> 10);
}

// Choice and majority
static uint32_t sha256_ch(uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) ^ ((~x) & z);
}
static uint32_t sha256_maj(uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) ^ (x & z) ^ (y & z);
}

std::array<uint8_t, 32> sha256(const uint8_t *data, size_t len) {
    uint32_t h[8] = {
        0x6a09e667u, 0xbb67ae85u, 0x3c6ef372u, 0xa54ff53au,
        0x510e527fu, 0x9b05688cu, 0x1f83d9abu, 0x5be0cd19u,
    };

    // Pad message: append 0x80, then zeros until length ≡ 56 (mod 64),
    // then append original length in bits as 64-bit big-endian.
    uint64_t total_bits = static_cast<uint64_t>(len) * 8;
    std::vector<uint8_t> msg(data, data + len);
    msg.push_back(0x80);
    while ((msg.size() % 64) != 56) {
        msg.push_back(0x00);
    }
    for (int i = 7; i >= 0; --i) {
        msg.push_back(static_cast<uint8_t>((total_bits >> (8 * i)) & 0xFF));
    }

    // Process each 64-byte block
    for (size_t offset = 0; offset < msg.size(); offset += 64) {
        uint32_t w[64];
        for (int i = 0; i < 16; ++i) {
            w[i] = (static_cast<uint32_t>(msg[offset + i * 4]) << 24) |
                   (static_cast<uint32_t>(msg[offset + i * 4 + 1]) << 16) |
                   (static_cast<uint32_t>(msg[offset + i * 4 + 2]) << 8) |
                   (static_cast<uint32_t>(msg[offset + i * 4 + 3]));
        }
        for (int i = 16; i < 64; ++i) {
            w[i] = sha256_sig1(w[i - 2]) + w[i - 7] + sha256_sig0(w[i - 15]) + w[i - 16];
        }

        uint32_t a = h[0], b = h[1], c = h[2], d = h[3];
        uint32_t e = h[4], f = h[5], g = h[6], hh = h[7];

        for (int i = 0; i < 64; ++i) {
            uint32_t t1 = hh + sha256_ep1(e) + sha256_ch(e, f, g) + sha256_k[i] + w[i];
            uint32_t t2 = sha256_ep0(a) + sha256_maj(a, b, c);
            hh = g;
            g = f;
            f = e;
            e = d + t1;
            d = c;
            c = b;
            b = a;
            a = t1 + t2;
        }

        h[0] += a; h[1] += b; h[2] += c; h[3] += d;
        h[4] += e; h[5] += f; h[6] += g; h[7] += hh;
    }

    // Produce 32-byte big-endian output
    std::array<uint8_t, 32> out{};
    for (int i = 0; i < 8; ++i) {
        out[i * 4]     = static_cast<uint8_t>((h[i] >> 24) & 0xFF);
        out[i * 4 + 1] = static_cast<uint8_t>((h[i] >> 16) & 0xFF);
        out[i * 4 + 2] = static_cast<uint8_t>((h[i] >> 8) & 0xFF);
        out[i * 4 + 3] = static_cast<uint8_t>(h[i] & 0xFF);
    }
    return out;
}

// ─────────────────────────────────────────────────────────────────────────────
// RC4 stream cipher
// ─────────────────────────────────────────────────────────────────────────────

std::vector<uint8_t> rc4(const uint8_t *key, size_t key_len,
                         const uint8_t *data, size_t data_len) {
    if (key_len == 0) ThrowFakeluaException("rc4: empty key");
    // KSA: Key Scheduling Algorithm
    uint8_t state[256];
    for (int i = 0; i < 256; ++i) state[i] = static_cast<uint8_t>(i);
    int j = 0;
    for (int i = 0; i < 256; ++i) {
        j = (j + state[i] + key[i % key_len]) & 0xFF;
        uint8_t t = state[i];
        state[i] = state[j];
        state[j] = t;
    }

    // PRGA: Pseudo-Random Generation Algorithm
    std::vector<uint8_t> out(data_len);
    int i = 0, k = 0;
    for (size_t idx = 0; idx < data_len; ++idx) {
        i = (i + 1) & 0xFF;
        k = (k + state[i]) & 0xFF;
        uint8_t t = state[i];
        state[i] = state[k];
        state[k] = t;
        out[idx] = data[idx] ^ state[(state[i] + state[k]) & 0xFF];
    }
    return out;
}

// ─────────────────────────────────────────────────────────────────────────────
// Blowfish block cipher
// ─────────────────────────────────────────────────────────────────────────────

static const uint32_t blowfish_p[18] = {
    0x243F6A88u,0x85A308D3u,0x13198A2Eu,0x03707344u,0xA4093822u,0x299F31D0u,0x082EFA98u,
    0xEC4E6C89u,0x452821E6u,0x38D01377u,0xBE5466CFu,0x34E90C6Cu,0xC0AC29B7u,0xC97C50DDu,
    0x3F84D5B5u,0xB5470917u,0x9216D5D9u,0x8979FB1Bu
};

static const uint32_t blowfish_s[4][256] = { {
    0xD1310BA6u,0x98DFB5ACu,0x2FFD72DBu,0xD01ADFB7u,0xB8E1AFEDu,0x6A267E96u,0xBA7C9045u,0xF12C7F99u,
    0x24A19947u,0xB3916CF7u,0x0801F2E2u,0x858EFC16u,0x636920D8u,0x71574E69u,0xA458FEA3u,0xF4933D7Eu,
    0x0D95748Fu,0x728EB658u,0x718BCD58u,0x82154AEEu,0x7B54A41Du,0xC25A59B5u,0x9C30D539u,0x2AF26013u,
    0xC5D1B023u,0x286085F0u,0xCA417918u,0xB8DB38EFu,0x8E79DCB0u,0x603A180Eu,0x6C9E0E8Bu,0xB01E8A3Eu,
    0xD71577C1u,0xBD314B27u,0x78AF2FDAu,0x55605C60u,0xE65525F3u,0xAA55AB94u,0x57489862u,0x63E81440u,
    0x55CA396Au,0x2AAB10B6u,0xB4CC5C34u,0x1141E8CEu,0xA15486AFu,0x7C72E993u,0xB3EE1411u,0x636FBC2Au,
    0x2BA9C55Du,0x741831F6u,0xCE5C3E16u,0x9B87931Eu,0xAFD6BA33u,0x6C24CF5Cu,0x7A325381u,0x28958677u,
    0x3B8F4898u,0x6B4BB9AFu,0xC4BFE81Bu,0x66282193u,0x61D809CCu,0xFB21A991u,0x487CAC60u,0x5DEC8032u,
    0xEF845D5Du,0xE98575B1u,0xDC262302u,0xEB651B88u,0x23893E81u,0xD396ACC5u,0x0F6D6FF3u,0x83F44239u,
    0x2E0B4482u,0xA4842004u,0x69C8F04Au,0x9E1F9B5Eu,0x21C66842u,0xF6E96C9Au,0x670C9C61u,0xABD388F0u,
    0x6A51A0D2u,0xD8542F68u,0x960FA728u,0xAB5133A3u,0x6EEF0B6Cu,0x137A3BE4u,0xBA3BF050u,0x7EFB2A98u,
    0xA1F1651Du,0x39AF0176u,0x66CA593Eu,0x82430E88u,0x8CEE8619u,0x456F9FB4u,0x7D84A5C3u,0x3B8B5EBEu,
    0xE06F75D8u,0x85C12073u,0x401A449Fu,0x56C16AA6u,0x4ED3AA62u,0x363F7706u,0x1BFEDF72u,0x429B023Du,
    0x37D0D724u,0xD00A1248u,0xDB0FEAD3u,0x49F1C09Bu,0x075372C9u,0x80991B7Bu,0x25D479D8u,0xF6E8DEF7u,
    0xE3FE501Au,0xB6794C3Bu,0x976CE0BDu,0x04C006BAu,0xC1A94FB6u,0x409F60C4u,0x5E5C9EC2u,0x196A2463u,
    0x68FB6FAFu,0x3E6C53B5u,0x1339B2EBu,0x3B52EC6Fu,0x6DFC511Fu,0x9B30952Cu,0xCC814544u,0xAF5EBD09u,
    0xBEE3D004u,0xDE334AFDu,0x660F2807u,0x192E4BB3u,0xC0CBA857u,0x45C8740Fu,0xD20B5F39u,0xB9D3FBDBu,
    0x5579C0BDu,0x1A60320Au,0xD6A100C6u,0x402C7279u,0x679F25FEu,0xFB1FA3CCu,0x8EA5E9F8u,0xDB3222F8u,
    0x3C7516DFu,0xFD616B15u,0x2F501EC8u,0xAD0552ABu,0x323DB5FAu,0xFD238760u,0x53317B48u,0x3E00DF82u,
    0x9E5C57BBu,0xCA6F8CA0u,0x1A87562Eu,0xDF1769DBu,0xD542A8F6u,0x287EFFC3u,0xAC6732C6u,0x8C4F5573u,
    0x695B27B0u,0xBBCA58C8u,0xE1FFA35Du,0xB8F011A0u,0x10FA3D98u,0xFD2183B8u,0x4AFCB56Cu,0x2DD1D35Bu,
    0x9A53E479u,0xB6F84565u,0xD28E49BCu,0x4BFB9790u,0xE1DDF2DAu,0xA4CB7E33u,0x62FB1341u,0xCEE4C6E8u,
    0xEF20CADAu,0x36774C01u,0xD07E9EFEu,0x2BF11FB4u,0x95DBDA4Du,0xAE909198u,0xEAAD8E71u,0x6B93D5A0u,
    0xD08ED1D0u,0xAFC725E0u,0x8E3C5B2Fu,0x8E7594B7u,0x8FF6E2FBu,0xF2122B64u,0x8888B812u,0x900DF01Cu,
    0x4FAD5EA0u,0x688FC31Cu,0xD1CFF191u,0xB3A8C1ADu,0x2F2F2218u,0xBE0E1777u,0xEA752DFEu,0x8B021FA1u,
    0xE5A0CC0Fu,0xB56F74E8u,0x18ACF3D6u,0xCE89E299u,0xB4A84FE0u,0xFD13E0B7u,0x7CC43B81u,0xD2ADA8D9u,
    0x165FA266u,0x80957705u,0x93CC7314u,0x211A1477u,0xE6AD2065u,0x77B5FA86u,0xC75442F5u,0xFB9D35CFu,
    0xEBCDAF0Cu,0x7B3E89A0u,0xD6411BD3u,0xAE1E7E49u,0x00250E2Du,0x2071B35Eu,0x226800BBu,0x57B8E0AFu,
    0x2464369Bu,0xF009B91Eu,0x5563911Du,0x59DFA6AAu,0x78C14389u,0xD95A537Fu,0x207D5BA2u,0x02E5B9C5u,
    0x83260376u,0x6295CFA9u,0x11C81968u,0x4E734A41u,0xB3472DCAu,0x7B14A94Au,0x1B510052u,0x9A532915u,
    0xD60F573Fu,0xBC9BC6E4u,0x2B60A476u,0x81E67400u,0x08BA6FB5u,0x571BE91Fu,0xF296EC6Bu,0x2A0DD915u,
    0xB6636521u,0xE7B9F9B6u,0xFF34052Eu,0xC5855664u,0x53B02D5Du,0xA99F8FA1u,0x08BA4799u,0x6E85076Au,
},{
    0x4B7A70E9u,0xB5B32944u,0xDB75092Eu,0xC4192623u,0xAD6EA6B0u,0x49A7DF7Du,0x9CEE60B8u,0x8FEDB266u,
    0xECAA8C71u,0x699A17FFu,0x5664526Cu,0xC2B19EE1u,0x193602A5u,0x75094C29u,0xA0591340u,0xE4183A3Eu,
    0x3F54989Au,0x5B429D65u,0x6B8FE4D6u,0x99F73FD6u,0xA1D29C07u,0xEFE830F5u,0x4D2D38E6u,0xF0255DC1u,
    0x4CDD2086u,0x8470EB26u,0x6382E9C6u,0x021ECC5Eu,0x09686B3Fu,0x3EBAEFC9u,0x3C971814u,0x6B6A70A1u,
    0x687F3584u,0x52A0E286u,0xB79C5305u,0xAA500737u,0x3E07841Cu,0x7FDEAE5Cu,0x8E7D44ECu,0x5716F2B8u,
    0xB03ADA37u,0xF0500C0Du,0xF01C1F04u,0x0200B3FFu,0xAE0CF51Au,0x3CB574B2u,0x25837A58u,0xDC0921BDu,
    0xD19113F9u,0x7CA92FF6u,0x94324773u,0x22F54701u,0x3AE5E581u,0x37C2DADCu,0xC8B57634u,0x9AF3DDA7u,
    0xA9446146u,0x0FD0030Eu,0xECC8C73Eu,0xA4751E41u,0xE238CD99u,0x3BEA0E2Fu,0x3280BBA1u,0x183EB331u,
    0x4E548B38u,0x4F6DB908u,0x6F420D03u,0xF60A04BFu,0x2CB81290u,0x24977C79u,0x5679B072u,0xBCAF89AFu,
    0xDE9A771Fu,0xD9930810u,0xB38BAE12u,0xDCCF3F2Eu,0x5512721Fu,0x2E6B7124u,0x501ADDE6u,0x9F84CD87u,
    0x7A584718u,0x7408DA17u,0xBC9F9ABCu,0xE94B7D8Cu,0xEC7AEC3Au,0xDB851DFAu,0x63094366u,0xC464C3D2u,
    0xEF1C1847u,0x3215D908u,0xDD433B37u,0x24C2BA16u,0x12A14D43u,0x2A65C451u,0x50940002u,0x133AE4DDu,
    0x71DFF89Eu,0x10314E55u,0x81AC77D6u,0x5F11199Bu,0x043556F1u,0xD7A3C76Bu,0x3C11183Bu,0x5924A509u,
    0xF28FE6EDu,0x97F1FBFAu,0x9EBABF2Cu,0x1E153C6Eu,0x86E34570u,0xEAE96FB1u,0x860E5E0Au,0x5A3E2AB3u,
    0x771FE71Cu,0x4E3D06FAu,0x2965DCB9u,0x99E71D0Fu,0x803E89D6u,0x5266C825u,0x2E4CC978u,0x9C10B36Au,
    0xC6150EBAu,0x94E2EA78u,0xA5FC3C53u,0x1E0A2DF4u,0xF2F74EA7u,0x361D2B3Du,0x1939260Fu,0x19C27960u,
    0x5223A708u,0xF71312B6u,0xEBADFE6Eu,0xEAC31F66u,0xE3BC4595u,0xA67BC883u,0xB17F37D1u,0x018CFF28u,
    0xC332DDEFu,0xBE6C5AA5u,0x65582185u,0x68AB9802u,0xEECEA50Fu,0xDB2F953Bu,0x2AEF7DADu,0x5B6E2F84u,
    0x1521B628u,0x29076170u,0xECDD4775u,0x619F1510u,0x13CCA830u,0xEB61BD96u,0x0334FE1Eu,0xAA0363CFu,
    0xB5735C90u,0x4C70A239u,0xD59E9E0Bu,0xCBAADE14u,0xEECC86BCu,0x60622CA7u,0x9CAB5CABu,0xB2F3846Eu,
    0x648B1EAFu,0x19BDF0CAu,0xA02369B9u,0x655ABB50u,0x40685A32u,0x3C2AB4B3u,0x319EE9D5u,0xC021B8F7u,
    0x9B540B19u,0x875FA099u,0x95F7997Eu,0x623D7DA8u,0xF837889Au,0x97E32D77u,0x11ED935Fu,0x16681281u,
    0x0E358829u,0xC7E61FD6u,0x96DEDFA1u,0x7858BA99u,0x57F584A5u,0x1B227263u,0x9B83C3FFu,0x1AC24696u,
    0xCDB30AEBu,0x532E3054u,0x8FD948E4u,0x6DBC3128u,0x58EBF2EFu,0x34C6FFEAu,0xFE28ED61u,0xEE7C3C73u,
    0x5D4A14D9u,0xE864B7E3u,0x42105D14u,0x203E13E0u,0x45EEE2B6u,0xA3AAABEAu,0xDB6C4F15u,0xFACB4FD0u,
    0xC742F442u,0xEF6ABBB5u,0x654F3B1Du,0x41CD2105u,0xD81E799Eu,0x86854DC7u,0xE44B476Au,0x3D816250u,
    0xCF62A1F2u,0x5B8D2646u,0xFC8883A0u,0xC1C7B6A3u,0x7F1524C3u,0x69CB7492u,0x47848A0Bu,0x5692B285u,
    0x095BBF00u,0xAD19489Du,0x1462B174u,0x23820E00u,0x58428D2Au,0x0C55F5EAu,0x1DADF43Eu,0x233F7061u,
    0x3372F092u,0x8D937E41u,0xD65FECF1u,0x6C223BDBu,0x7CDE3759u,0xCBEE7460u,0x4085F2A7u,0xCE77326Eu,
    0xA6078084u,0x19F8509Eu,0xE8EFD855u,0x61D99735u,0xA969A7AAu,0xC50C06C2u,0x5A04ABFCu,0x800BCADCu,
    0x9E447A2Eu,0xC3453484u,0xFDD56705u,0x0E1E9EC9u,0xDB73DBD3u,0x105588CDu,0x675FDA79u,0xE3674340u,
    0xC5C43465u,0x713E38D8u,0x3D28F89Eu,0xF16DFF20u,0x153E21E7u,0x8FB03D4Au,0xE6E39F2Bu,0xDB83ADF7u,
},{
    0xE93D5A68u,0x948140F7u,0xF64C261Cu,0x94692934u,0x411520F7u,0x7602D4F7u,0xBCF46B2Eu,0xD4A20068u,
    0xD4082471u,0x3320F46Au,0x43B7D4B7u,0x500061AFu,0x1E39F62Eu,0x97244546u,0x14214F74u,0xBF8B8840u,
    0x4D95FC1Du,0x96B591AFu,0x70F4DDD3u,0x66A02F45u,0xBFBC09ECu,0x03BD9785u,0x7FAC6DD0u,0x31CB8504u,
    0x96EB27B3u,0x55FD3941u,0xDA2547E6u,0xABCA0A9Au,0x28507825u,0x530429F4u,0x0A2C86DAu,0xE9B66DFBu,
    0x68DC1462u,0xD7486900u,0x680EC0A4u,0x27A18DEEu,0x4F3FFEA2u,0xE887AD8Cu,0xB58CE006u,0x7AF4D6B6u,
    0xAACE1E7Cu,0xD3375FECu,0xCE78A399u,0x406B2A42u,0x20FE9E35u,0xD9F385B9u,0xEE39D7ABu,0x3B124E8Bu,
    0x1DC9FAF7u,0x4B6D1856u,0x26A36631u,0xEAE397B2u,0x3A6EFA74u,0xDD5B4332u,0x6841E7F7u,0xCA7820FBu,
    0xFB0AF54Eu,0xD8FEB397u,0x454056ACu,0xBA489527u,0x55533A3Au,0x20838D87u,0xFE6BA9B7u,0xD096954Bu,
    0x55A867BCu,0xA1159A58u,0xCCA92963u,0x99E1DB33u,0xA62A4A56u,0x3F3125F9u,0x5EF47E1Cu,0x9029317Cu,
    0xFDF8E802u,0x04272F70u,0x80BB155Cu,0x05282CE3u,0x95C11548u,0xE4C66D22u,0x48C1133Fu,0xC70F86DCu,
    0x07F9C9EEu,0x41041F0Fu,0x404779A4u,0x5D886E17u,0x325F51EBu,0xD59BC0D1u,0xF2BCC18Fu,0x41113564u,
    0x257B7834u,0x602A9C60u,0xDFF8E8A3u,0x1F636C1Bu,0x0E12B4C2u,0x02E1329Eu,0xAF664FD1u,0xCAD18115u,
    0x6B2395E0u,0x333E92E1u,0x3B240B62u,0xEEBEB922u,0x85B2A20Eu,0xE6BA0D99u,0xDE720C8Cu,0x2DA2F728u,
    0xD0127845u,0x95B794FDu,0x647D0862u,0xE7CCF5F0u,0x5449A36Fu,0x877D48FAu,0xC39DFD27u,0xF33E8D1Eu,
    0x0A476341u,0x992EFF74u,0x3A6F6EABu,0xF4F8FD37u,0xA812DC60u,0xA1EBDDF8u,0x991BE14Cu,0xDB6E6B0Du,
    0xC67B5510u,0x6D672C37u,0x2765D43Bu,0xDCD0E804u,0xF1290DC7u,0xCC00FFA3u,0xB5390F92u,0x690FED0Bu,
    0x667B9FFBu,0xCEDB7D9Cu,0xA091CF0Bu,0xD9155EA3u,0xBB132F88u,0x515BAD24u,0x7B9479BFu,0x763BD6EBu,
    0x37392EB3u,0xCC115979u,0x8026E297u,0xF42E312Du,0x6842ADA7u,0xC66A2B3Bu,0x12754CCCu,0x782EF11Cu,
    0x6A124237u,0xB79251E7u,0x06A1BBE6u,0x4BFB6350u,0x1A6B1018u,0x11CAEDFAu,0x3D25BDD8u,0xE2E1C3C9u,
    0x44421659u,0x0A121386u,0xD90CEC6Eu,0xD5ABEA2Au,0x64AF674Eu,0xDA86A85Fu,0xBEBFE988u,0x64E4C3FEu,
    0x9DBC8057u,0xF0F7C086u,0x60787BF8u,0x6003604Du,0xD1FD8346u,0xF6381FB0u,0x7745AE04u,0xD736FCCCu,
    0x83426B33u,0xF01EAB71u,0xB0804187u,0x3C005E5Fu,0x77A057BEu,0xBDE8AE24u,0x55464299u,0xBF582E61u,
    0x4E58F48Fu,0xF2DDFDA2u,0xF474EF38u,0x8789BDC2u,0x5366F9C3u,0xC8B38E74u,0xB475F255u,0x46FCD9B9u,
    0x7AEB2661u,0x8B1DDF84u,0x846A0E79u,0x915F95E2u,0x466E598Eu,0x20B45770u,0x8CD55591u,0xC902DE4Cu,
    0xB90BACE1u,0xBB8205D0u,0x11A86248u,0x7574A99Eu,0xB77F19B6u,0xE0A9DC09u,0x662D09A1u,0xC4324633u,
    0xE85A1F02u,0x09F0BE8Cu,0x4A99A025u,0x1D6EFE10u,0x1AB93D1Du,0x0BA5A4DFu,0xA186F20Fu,0x2868F169u,
    0xDCB7DA83u,0x573906FEu,0xA1E2CE9Bu,0x4FCD7F52u,0x50115E01u,0xA70683FAu,0xA002B5C4u,0x0DE6D027u,
    0x9AF88C27u,0x773F8641u,0xC3604C06u,0x61A806B5u,0xF0177A28u,0xC0F586E0u,0x006058AAu,0x30DC7D62u,
    0x11E69ED7u,0x2338EA63u,0x53C2DD94u,0xC2C21634u,0xBBCBEE56u,0x90BCB6DEu,0xEBFC7DA1u,0xCE591D76u,
    0x6F05E409u,0x4B7C0188u,0x39720A3Du,0x7C927C24u,0x86E3725Fu,0x724D9DB9u,0x1AC15BB4u,0xD39EB8FCu,
    0xED545578u,0x08FCA5B5u,0xD83D7CD3u,0x4DAD0FC4u,0x1E50EF5Eu,0xB161E6F8u,0xA28514D9u,0x6C51133Cu,
    0x6FD5C7E7u,0x56E14EC4u,0x362ABFCEu,0xDDC6C837u,0xD79A3234u,0x92638212u,0x670EFA8Eu,0x406000E0u,
},{
    0x3A39CE37u,0xD3FAF5CFu,0xABC27737u,0x5AC52D1Bu,0x5CB0679Eu,0x4FA33742u,0xD3822740u,0x99BC9BBEu,
    0xD5118E9Du,0xBF0F7315u,0xD62D1C7Eu,0xC700C47Bu,0xB78C1B6Bu,0x21A19045u,0xB26EB1BEu,0x6A366EB4u,
    0x5748AB2Fu,0xBC946E79u,0xC6A376D2u,0x6549C2C8u,0x530FF8EEu,0x468DDE7Du,0xD5730A1Du,0x4CD04DC6u,
    0x2939BBDBu,0xA9BA4650u,0xAC9526E8u,0xBE5EE304u,0xA1FAD5F0u,0x6A2D519Au,0x63EF8CE2u,0x9A86EE22u,
    0xC089C2B8u,0x43242EF6u,0xA51E03AAu,0x9CF2D0A4u,0x83C061BAu,0x9BE96A4Du,0x8FE51550u,0xBA645BD6u,
    0x2826A2F9u,0xA73A3AE1u,0x4BA99586u,0xEF5562E9u,0xC72FEFD3u,0xF752F7DAu,0x3F046F69u,0x77FA0A59u,
    0x80E4A915u,0x87B08601u,0x9B09E6ADu,0x3B3EE593u,0xE990FD5Au,0x9E34D797u,0x2CF0B7D9u,0x022B8B51u,
    0x96D5AC3Au,0x017DA67Du,0xD1CF3ED6u,0x7C7D2D28u,0x1F9F25CFu,0xADF2B89Bu,0x5AD6B472u,0x5A88F54Cu,
    0xE029AC71u,0xE019A5E6u,0x47B0ACFDu,0xED93FA9Bu,0xE8D3C48Du,0x283B57CCu,0xF8D56629u,0x79132E28u,
    0x785F0191u,0xED756055u,0xF7960E44u,0xE3D35E8Cu,0x15056DD4u,0x88F46DBAu,0x03A16125u,0x0564F0BDu,
    0xC3EB9E15u,0x3C9057A2u,0x97271AECu,0xA93A072Au,0x1B3F6D9Bu,0x1E6321F5u,0xF59C66FBu,0x26DCF319u,
    0x7533D928u,0xB155FDF5u,0x03563482u,0x8ABA3CBBu,0x28517711u,0xC20AD9F8u,0xABCC5167u,0xCCAD925Fu,
    0x4DE81751u,0x3830DC8Eu,0x379D5862u,0x9320F991u,0xEA7A90C2u,0xFB3E7BCEu,0x5121CE64u,0x774FBE32u,
    0xA8B6E37Eu,0xC3293D46u,0x48DE5369u,0x6413E680u,0xA2AE0810u,0xDD6DB224u,0x69852DFDu,0x09072166u,
    0xB39A460Au,0x6445C0DDu,0x586CDECFu,0x1C20C8AEu,0x5BBEF7DDu,0x1B588D40u,0xCCD2017Fu,0x6BB4E3BBu,
    0xDDA26A7Eu,0x3A59FF45u,0x3E350A44u,0xBCB4CDD5u,0x72EACEA8u,0xFA6484BBu,0x8D6612AEu,0xBF3C6F47u,
    0xD29BE463u,0x542F5D9Eu,0xAEC2771Bu,0xF64E6370u,0x740E0D8Du,0xE75B1357u,0xF8721671u,0xAF537D5Du,
    0x4040CB08u,0x4EB4E2CCu,0x34D2466Au,0x0115AF84u,0xE1B00428u,0x95983A1Du,0x06B89FB4u,0xCE6EA048u,
    0x6F3F3B82u,0x3520AB82u,0x011A1D4Bu,0x277227F8u,0x611560B1u,0xE7933FDCu,0xBB3A792Bu,0x344525BDu,
    0xA08839E1u,0x51CE794Bu,0x2F32C9B7u,0xA01FBAC9u,0xE01CC87Eu,0xBCC7D1F6u,0xCF0111C3u,0xA1E8AAC7u,
    0x1A908749u,0xD44FBD9Au,0xD0DADECBu,0xD50ADA38u,0x0339C32Au,0xC6913667u,0x8DF9317Cu,0xE0B12B4Fu,
    0xF79E59B7u,0x43F5BB3Au,0xF2D519FFu,0x27D9459Cu,0xBF97222Cu,0x15E6FC2Au,0x0F91FC71u,0x9B941525u,
    0xFAE59361u,0xCEB69CEBu,0xC2A86459u,0x12BAA8D1u,0xB6C1075Eu,0xE3056A0Cu,0x10D25065u,0xCB03A442u,
    0xE0EC6E0Eu,0x1698DB3Bu,0x4C98A0BEu,0x3278E964u,0x9F1F9532u,0xE0D392DFu,0xD3A0342Bu,0x8971F21Eu,
    0x1B0A7441u,0x4BA3348Cu,0xC5BE7120u,0xC37632D8u,0xDF359F8Du,0x9B992F2Eu,0xE60B6F47u,0x0FE3F11Du,
    0xE54CDA54u,0x1EDAD891u,0xCE6279CFu,0xCD3E7E6Fu,0x1618B166u,0xFD2C1D05u,0x848FD2C5u,0xF6FB2299u,
    0xF523F357u,0xA6327623u,0x93A83531u,0x56CCCD02u,0xACF08162u,0x5A75EBB5u,0x6E163697u,0x88D273CCu,
    0xDE966292u,0x81B949D0u,0x4C50901Bu,0x71C65614u,0xE6C6C7BDu,0x327A140Au,0x45E1D006u,0xC3F27B9Au,
    0xC9AA53FDu,0x62A80F00u,0xBB25BFE2u,0x35BDD2F6u,0x71126905u,0xB2040222u,0xB6CBCF7Cu,0xCD769C2Bu,
    0x53113EC0u,0x1640E3D3u,0x38ABBD60u,0x2547ADF0u,0xBA38209Cu,0xF746CE76u,0x77AFA1C5u,0x20756060u,
    0x85CBFE4Eu,0x8AE88DD8u,0x7AAAF9B0u,0x4CF9AA7Eu,0x1948C25Cu,0x02FB8A8Cu,0x01C36AE4u,0xD6EBE1F9u,
    0x90D4F869u,0xA65CDEAu,0x3F09252Du,0xC208E69Fu,0xB74E6132u,0xCE77E25Bu,0x578FDFE3u,0x3AC372E6u
} };

// Blowfish F-function: looks up into the S-boxes
static uint32_t blowfish_f(uint32_t x, const uint32_t s[4][256]) {
    uint32_t t = s[0][(x >> 24) & 0xFF];
    t += s[1][(x >> 16) & 0xFF];
    t ^= s[2][(x >> 8) & 0xFF];
    t += s[3][x & 0xFF];
    return t;
}

// Encrypt a single 8-byte block (no mode)
// Matches B-Con's implementation exactly: 15 ITERATIONs with swap, then final round
static void blowfish_encrypt_block(const uint8_t in[8], uint8_t out[8],
                                   const uint32_t p[18], const uint32_t s[4][256]) {
    uint32_t l = (in[0] << 24) | (in[1] << 16) | (in[2] << 8) | in[3];
    uint32_t r = (in[4] << 24) | (in[5] << 16) | (in[6] << 8) | in[7];
    uint32_t t;

    // 15 ITERATIONs (each: l ^= p[i]; F(l,t); r ^= t; swap(l,r))
    for (int i = 0; i < 15; ++i) {
        l ^= p[i];
        t = blowfish_f(l, s);
        r ^= t;
        // swap l and r
        t = l;
        l = r;
        r = t;
    }
    // Final round (no swap)
    l ^= p[15];
    t = blowfish_f(l, s);
    r ^= t;
    r ^= p[16];
    l ^= p[17];

    out[0] = l >> 24; out[1] = l >> 16; out[2] = l >> 8; out[3] = l;
    out[4] = r >> 24; out[5] = r >> 16; out[6] = r >> 8; out[7] = r;
}

// Decrypt a single 8-byte block (no mode)
// Reverse of encrypt: undo final round, then 15 reverse ITERATIONs
static void blowfish_decrypt_block(const uint8_t in[8], uint8_t out[8],
                                   const uint32_t p[18], const uint32_t s[4][256]) {
    uint32_t l = (in[0] << 24) | (in[1] << 16) | (in[2] << 8) | in[3];
    uint32_t r = (in[4] << 24) | (in[5] << 16) | (in[6] << 8) | in[7];
    uint32_t t;

    // Undo final round
    l ^= p[17];
    r ^= p[16];
    t = blowfish_f(l, s);
    r ^= t;
    l ^= p[15];

    // Undo 15 ITERATIONs in reverse (each: swap; r ^= t; t = F(l); l ^= p[i])
    for (int i = 14; i >= 0; --i) {
        // swap l and r (undo the swap)
        t = l;
        l = r;
        r = t;
        t = blowfish_f(l, s);
        r ^= t;
        l ^= p[i];
    }

    out[0] = l >> 24; out[1] = l >> 16; out[2] = l >> 8; out[3] = l;
    out[4] = r >> 24; out[5] = r >> 16; out[6] = r >> 8; out[7] = r;
}

std::vector<uint8_t> blowfish_encrypt(const uint8_t *key, size_t key_len,
                                      const uint8_t *data, size_t data_len) {
    if (key_len == 0) ThrowFakeluaException("blowfish_encrypt: empty key");
    // Copy the constant init arrays
    uint32_t p[18];
    uint32_t s[4][256];
    memcpy(p, blowfish_p, sizeof(p));
    memcpy(s, blowfish_s, sizeof(s));

    // Combine the key with the P box
    for (int idx = 0, idx2 = 0; idx < 18; ++idx, idx2 += 4) {
        p[idx] ^= (key[idx2 % key_len] << 24) | (key[(idx2 + 1) % key_len] << 16) |
                  (key[(idx2 + 2) % key_len] << 8) | key[(idx2 + 3) % key_len];
    }

    // Re-calculate the P box via encryption
    uint8_t block[8] = {0};
    for (int idx = 0; idx < 18; idx += 2) {
        blowfish_encrypt_block(block, block, p, s);
        p[idx] = (block[0] << 24) | (block[1] << 16) | (block[2] << 8) | block[3];
        p[idx + 1] = (block[4] << 24) | (block[5] << 16) | (block[6] << 8) | block[7];
    }

    // Recalculate the S-boxes
    for (int idx = 0; idx < 4; ++idx) {
        for (int idx2 = 0; idx2 < 256; idx2 += 2) {
            blowfish_encrypt_block(block, block, p, s);
            s[idx][idx2] = (block[0] << 24) | (block[1] << 16) | (block[2] << 8) | block[3];
            s[idx][idx2 + 1] = (block[4] << 24) | (block[5] << 16) | (block[6] << 8) | block[7];
        }
    }

    // Zero-pad data to multiple of 8
    size_t padded = (data_len + 7) & ~size_t(7);
    std::vector<uint8_t> padded_data(data, data + data_len);
    padded_data.resize(padded, 0);

    std::vector<uint8_t> out(padded);
    for (size_t i = 0; i < padded; i += 8) {
        blowfish_encrypt_block(padded_data.data() + i, out.data() + i, p, s);
    }
    return out;
}

std::vector<uint8_t> blowfish_decrypt(const uint8_t *key, size_t key_len,
                                      const uint8_t *data, size_t data_len) {
    if (key_len == 0) ThrowFakeluaException("blowfish_decrypt: empty key");
    if (data_len % 8 != 0) ThrowFakeluaException("blowfish_decrypt: length must be a multiple of 8");
    // Same key setup as encrypt
    uint32_t p[18];
    uint32_t s[4][256];
    memcpy(p, blowfish_p, sizeof(p));
    memcpy(s, blowfish_s, sizeof(s));

    for (int idx = 0, idx2 = 0; idx < 18; ++idx, idx2 += 4) {
        p[idx] ^= (key[idx2 % key_len] << 24) | (key[(idx2 + 1) % key_len] << 16) |
                  (key[(idx2 + 2) % key_len] << 8) | key[(idx2 + 3) % key_len];
    }

    uint8_t block[8] = {0};
    for (int idx = 0; idx < 18; idx += 2) {
        blowfish_encrypt_block(block, block, p, s);
        p[idx] = (block[0] << 24) | (block[1] << 16) | (block[2] << 8) | block[3];
        p[idx + 1] = (block[4] << 24) | (block[5] << 16) | (block[6] << 8) | block[7];
    }

    for (int idx = 0; idx < 4; ++idx) {
        for (int idx2 = 0; idx2 < 256; idx2 += 2) {
            blowfish_encrypt_block(block, block, p, s);
            s[idx][idx2] = (block[0] << 24) | (block[1] << 16) | (block[2] << 8) | block[3];
            s[idx][idx2 + 1] = (block[4] << 24) | (block[5] << 16) | (block[6] << 8) | block[7];
        }
    }

    // Decrypt (data must be multiple of 8)
    std::vector<uint8_t> out(data_len);
    for (size_t i = 0; i < data_len; i += 8) {
        blowfish_decrypt_block(data + i, out.data() + i, p, s);
    }
    return out;
}

// ─────────────────────────────────────────────────────────────────────────────
// DES block cipher
// ─────────────────────────────────────────────────────────────────────────────

static const uint8_t des_sbox1[64] = {
    14,  4,  13,  1,   2, 15,  11,  8,   3, 10,   6, 12,   5,  9,   0,  7,
     0, 15,   7,  4,  14,  2,  13,  1,  10,  6,  12, 11,   9,  5,   3,  8,
     4,  1,  14,  8,  13,  6,   2, 11,  15, 12,   9,  7,   3, 10,   5,  0,
    15, 12,   8,  2,   4,  9,   1,  7,   5, 11,   3, 14,  10,  0,   6, 13
};
static const uint8_t des_sbox2[64] = {
    15,  1,   8, 14,   6, 11,   3,  4,   9,  7,   2, 13,  12,  0,   5, 10,
     3, 13,   4,  7,  15,  2,   8, 14,  12,  0,   1, 10,   6,  9,  11,  5,
     0, 14,   7, 11,  10,  4,  13,  1,   5,  8,  12,  6,   9,  3,   2, 15,
    13,  8,  10,  1,   3, 15,   4,  2,  11,  6,   7, 12,   0,  5,  14,  9
};
static const uint8_t des_sbox3[64] = {
    10,  0,   9, 14,   6,  3,  15,  5,   1, 13,  12,  7,  11,  4,   2,  8,
    13,  7,   0,  9,   3,  4,   6, 10,   2,  8,   5, 14,  12, 11,  15,  1,
    13,  6,   4,  9,   8, 15,   3,  0,  11,  1,   2, 12,   5, 10,  14,  7,
     1, 10,  13,  0,   6,  9,   8,  7,   4, 15,  14,  3,  11,  5,   2, 12
};
static const uint8_t des_sbox4[64] = {
     7, 13,  14,  3,   0,  6,   9, 10,   1,  2,   8,  5,  11, 12,   4, 15,
    13,  8,  11,  5,   6, 15,   0,  3,   4,  7,   2, 12,   1, 10,  14,  9,
    10,  6,   9,  0,  12, 11,   7, 13,  15,  1,   3, 14,   5,  2,   8,  4,
     3, 15,   0,  6,  10,  1,  13,  8,   9,  4,   5, 11,  12,  7,   2, 14
};
static const uint8_t des_sbox5[64] = {
     2, 12,   4,  1,   7, 10,  11,  6,   8,  5,   3, 15,  13,  0,  14,  9,
    14, 11,   2, 12,   4,  7,  13,  1,   5,  0,  15, 10,   3,  9,   8,  6,
     4,  2,   1, 11,  10, 13,   7,  8,  15,  9,  12,  5,   6,  3,   0, 14,
    11,  8,  12,  7,   1, 14,   2, 13,   6, 15,   0,  9,  10,  4,   5,  3
};
static const uint8_t des_sbox6[64] = {
    12,  1,  10, 15,   9,  2,   6,  8,   0, 13,   3,  4,  14,  7,   5, 11,
    10, 15,   4,  2,   7, 12,   9,  5,   6,  1,  13, 14,   0, 11,   3,  8,
     9, 14,  15,  5,   2,  8,  12,  3,   7,  0,   4, 10,   1, 13,  11,  6,
     4,  3,   2, 12,   9,  5,  15, 10,  11, 14,   1,  7,   6,  0,   8, 13
};
static const uint8_t des_sbox7[64] = {
     4, 11,   2, 14,  15,  0,   8, 13,   3, 12,   9,  7,   5, 10,   6,  1,
    13,  0,  11,  7,   4,  9,   1, 10,  14,  3,   5, 12,   2, 15,   8,  6,
     1,  4,  11, 13,  12,  3,   7, 14,  10, 15,   6,  8,   0,  5,   9,  2,
     6, 11,  13,  8,   1,  4,  10,  7,   9,  5,   0, 15,  14,  2,   3, 12
};
static const uint8_t des_sbox8[64] = {
    13,  2,   8,  4,   6, 15,  11,  1,  10,  9,   3, 14,   5,  0,  12,  7,
     1, 15,  13,  8,  10,  3,   7,  4,  12,  5,   6, 11,   0, 14,   9,  2,
     7, 11,   4,  1,   9, 12,  14,  2,   0,  6,  10, 13,  15,  3,   5,  8,
     2,  1,  14,  7,   4, 10,   8, 13,  15, 12,   9,  0,   3,  5,   6, 11
};

// DES helper macros and functions
static inline uint32_t des_bitnum(const uint8_t *a, int b, int c) {
    return ((((a[(b)/8] >> (7 - (b%8))) & 0x01)) << (c));
}
static inline uint32_t des_bitnum_int_r(uint32_t a, int b, int c) {
    return (((a >> (31 - (b))) & 0x00000001) << (c));
}
static inline uint32_t des_bitnum_int_l(uint32_t a, int b, int c) {
    return (((a << (b)) & 0x80000000) >> (c));
}
static inline uint32_t des_sbox_bit(uint32_t a) {
    return ((a & 0x20) | (((a) & 0x1f) >> 1) | (((a) & 0x01) << 4));
}

static void des_ip(uint32_t state[2], const uint8_t in[8]) {
    state[0] = des_bitnum(in,57,31) | des_bitnum(in,49,30) | des_bitnum(in,41,29) | des_bitnum(in,33,28) |
               des_bitnum(in,25,27) | des_bitnum(in,17,26) | des_bitnum(in,9,25) | des_bitnum(in,1,24) |
               des_bitnum(in,59,23) | des_bitnum(in,51,22) | des_bitnum(in,43,21) | des_bitnum(in,35,20) |
               des_bitnum(in,27,19) | des_bitnum(in,19,18) | des_bitnum(in,11,17) | des_bitnum(in,3,16) |
               des_bitnum(in,61,15) | des_bitnum(in,53,14) | des_bitnum(in,45,13) | des_bitnum(in,37,12) |
               des_bitnum(in,29,11) | des_bitnum(in,21,10) | des_bitnum(in,13,9) | des_bitnum(in,5,8) |
               des_bitnum(in,63,7) | des_bitnum(in,55,6) | des_bitnum(in,47,5) | des_bitnum(in,39,4) |
               des_bitnum(in,31,3) | des_bitnum(in,23,2) | des_bitnum(in,15,1) | des_bitnum(in,7,0);
    state[1] = des_bitnum(in,56,31) | des_bitnum(in,48,30) | des_bitnum(in,40,29) | des_bitnum(in,32,28) |
               des_bitnum(in,24,27) | des_bitnum(in,16,26) | des_bitnum(in,8,25) | des_bitnum(in,0,24) |
               des_bitnum(in,58,23) | des_bitnum(in,50,22) | des_bitnum(in,42,21) | des_bitnum(in,34,20) |
               des_bitnum(in,26,19) | des_bitnum(in,18,18) | des_bitnum(in,10,17) | des_bitnum(in,2,16) |
               des_bitnum(in,60,15) | des_bitnum(in,52,14) | des_bitnum(in,44,13) | des_bitnum(in,36,12) |
               des_bitnum(in,28,11) | des_bitnum(in,20,10) | des_bitnum(in,12,9) | des_bitnum(in,4,8) |
               des_bitnum(in,62,7) | des_bitnum(in,54,6) | des_bitnum(in,46,5) | des_bitnum(in,38,4) |
               des_bitnum(in,30,3) | des_bitnum(in,22,2) | des_bitnum(in,14,1) | des_bitnum(in,6,0);
}

static void des_inv_ip(uint32_t state[2], uint8_t out[8]) {
    out[0] = des_bitnum_int_r(state[1],7,7) | des_bitnum_int_r(state[0],7,6) |
             des_bitnum_int_r(state[1],15,5) | des_bitnum_int_r(state[0],15,4) |
             des_bitnum_int_r(state[1],23,3) | des_bitnum_int_r(state[0],23,2) |
             des_bitnum_int_r(state[1],31,1) | des_bitnum_int_r(state[0],31,0);
    out[1] = des_bitnum_int_r(state[1],6,7) | des_bitnum_int_r(state[0],6,6) |
             des_bitnum_int_r(state[1],14,5) | des_bitnum_int_r(state[0],14,4) |
             des_bitnum_int_r(state[1],22,3) | des_bitnum_int_r(state[0],22,2) |
             des_bitnum_int_r(state[1],30,1) | des_bitnum_int_r(state[0],30,0);
    out[2] = des_bitnum_int_r(state[1],5,7) | des_bitnum_int_r(state[0],5,6) |
             des_bitnum_int_r(state[1],13,5) | des_bitnum_int_r(state[0],13,4) |
             des_bitnum_int_r(state[1],21,3) | des_bitnum_int_r(state[0],21,2) |
             des_bitnum_int_r(state[1],29,1) | des_bitnum_int_r(state[0],29,0);
    out[3] = des_bitnum_int_r(state[1],4,7) | des_bitnum_int_r(state[0],4,6) |
             des_bitnum_int_r(state[1],12,5) | des_bitnum_int_r(state[0],12,4) |
             des_bitnum_int_r(state[1],20,3) | des_bitnum_int_r(state[0],20,2) |
             des_bitnum_int_r(state[1],28,1) | des_bitnum_int_r(state[0],28,0);
    out[4] = des_bitnum_int_r(state[1],3,7) | des_bitnum_int_r(state[0],3,6) |
             des_bitnum_int_r(state[1],11,5) | des_bitnum_int_r(state[0],11,4) |
             des_bitnum_int_r(state[1],19,3) | des_bitnum_int_r(state[0],19,2) |
             des_bitnum_int_r(state[1],27,1) | des_bitnum_int_r(state[0],27,0);
    out[5] = des_bitnum_int_r(state[1],2,7) | des_bitnum_int_r(state[0],2,6) |
             des_bitnum_int_r(state[1],10,5) | des_bitnum_int_r(state[0],10,4) |
             des_bitnum_int_r(state[1],18,3) | des_bitnum_int_r(state[0],18,2) |
             des_bitnum_int_r(state[1],26,1) | des_bitnum_int_r(state[0],26,0);
    out[6] = des_bitnum_int_r(state[1],1,7) | des_bitnum_int_r(state[0],1,6) |
             des_bitnum_int_r(state[1],9,5) | des_bitnum_int_r(state[0],9,4) |
             des_bitnum_int_r(state[1],17,3) | des_bitnum_int_r(state[0],17,2) |
             des_bitnum_int_r(state[1],25,1) | des_bitnum_int_r(state[0],25,0);
    out[7] = des_bitnum_int_r(state[1],0,7) | des_bitnum_int_r(state[0],0,6) |
             des_bitnum_int_r(state[1],8,5) | des_bitnum_int_r(state[0],8,4) |
             des_bitnum_int_r(state[1],16,3) | des_bitnum_int_r(state[0],16,2) |
             des_bitnum_int_r(state[1],24,1) | des_bitnum_int_r(state[0],24,0);
}

static uint32_t des_f(uint32_t state, const uint8_t key[6]) {
    uint8_t lrgstate[6];
    uint32_t t1, t2;

    // Expansion Permutation
    t1 = des_bitnum_int_l(state,31,0) | ((state & 0xf0000000) >> 1) | des_bitnum_int_l(state,4,5) |
         des_bitnum_int_l(state,3,6) | ((state & 0x0f000000) >> 3) | des_bitnum_int_l(state,8,11) |
         des_bitnum_int_l(state,7,12) | ((state & 0x00f00000) >> 5) | des_bitnum_int_l(state,12,17) |
         des_bitnum_int_l(state,11,18) | ((state & 0x000f0000) >> 7) | des_bitnum_int_l(state,16,23);
    t2 = des_bitnum_int_l(state,15,0) | ((state & 0x0000f000) << 15) | des_bitnum_int_l(state,20,5) |
         des_bitnum_int_l(state,19,6) | ((state & 0x00000f00) << 13) | des_bitnum_int_l(state,24,11) |
         des_bitnum_int_l(state,23,12) | ((state & 0x000000f0) << 11) | des_bitnum_int_l(state,28,17) |
         des_bitnum_int_l(state,27,18) | ((state & 0x0000000f) << 9) | des_bitnum_int_l(state,0,23);

    lrgstate[0] = (t1 >> 24) & 0xFF;
    lrgstate[1] = (t1 >> 16) & 0xFF;
    lrgstate[2] = (t1 >> 8) & 0xFF;
    lrgstate[3] = (t2 >> 24) & 0xFF;
    lrgstate[4] = (t2 >> 16) & 0xFF;
    lrgstate[5] = (t2 >> 8) & 0xFF;

    // Key XOR
    lrgstate[0] ^= key[0]; lrgstate[1] ^= key[1]; lrgstate[2] ^= key[2];
    lrgstate[3] ^= key[3]; lrgstate[4] ^= key[4]; lrgstate[5] ^= key[5];

    // S-Box Permutation
    uint32_t out = 0;
    out |= (uint32_t)des_sbox1[des_sbox_bit(lrgstate[0] >> 2)] << 28;
    out |= (uint32_t)des_sbox2[des_sbox_bit(((lrgstate[0] & 0x03) << 4) | (lrgstate[1] >> 4))] << 24;
    out |= (uint32_t)des_sbox3[des_sbox_bit(((lrgstate[1] & 0x0f) << 2) | (lrgstate[2] >> 6))] << 20;
    out |= (uint32_t)des_sbox4[des_sbox_bit(lrgstate[2] & 0x3f)] << 16;
    out |= (uint32_t)des_sbox5[des_sbox_bit(lrgstate[3] >> 2)] << 12;
    out |= (uint32_t)des_sbox6[des_sbox_bit(((lrgstate[3] & 0x03) << 4) | (lrgstate[4] >> 4))] << 8;
    out |= (uint32_t)des_sbox7[des_sbox_bit(((lrgstate[4] & 0x0f) << 2) | (lrgstate[5] >> 6))] << 4;
    out |= (uint32_t)des_sbox8[des_sbox_bit(lrgstate[5] & 0x3f)];

    // P-Box Permutation
    out = des_bitnum_int_l(out,15,0) | des_bitnum_int_l(out,6,1) | des_bitnum_int_l(out,19,2) |
          des_bitnum_int_l(out,20,3) | des_bitnum_int_l(out,28,4) | des_bitnum_int_l(out,11,5) |
          des_bitnum_int_l(out,27,6) | des_bitnum_int_l(out,16,7) | des_bitnum_int_l(out,0,8) |
          des_bitnum_int_l(out,14,9) | des_bitnum_int_l(out,22,10) | des_bitnum_int_l(out,25,11) |
          des_bitnum_int_l(out,4,12) | des_bitnum_int_l(out,17,13) | des_bitnum_int_l(out,30,14) |
          des_bitnum_int_l(out,9,15) | des_bitnum_int_l(out,1,16) | des_bitnum_int_l(out,7,17) |
          des_bitnum_int_l(out,23,18) | des_bitnum_int_l(out,13,19) | des_bitnum_int_l(out,31,20) |
          des_bitnum_int_l(out,26,21) | des_bitnum_int_l(out,2,22) | des_bitnum_int_l(out,8,23) |
          des_bitnum_int_l(out,18,24) | des_bitnum_int_l(out,12,25) | des_bitnum_int_l(out,29,26) |
          des_bitnum_int_l(out,5,27) | des_bitnum_int_l(out,21,28) | des_bitnum_int_l(out,10,29) |
          des_bitnum_int_l(out,3,30) | des_bitnum_int_l(out,24,31);

    return out;
}

static void des_key_setup(const uint8_t key[8], uint8_t schedule[16][6], bool decrypt) {
    static const int key_rnd_shift[16] = {1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1};
    static const int key_perm_c[28] = {56,48,40,32,24,16,8,0,57,49,41,33,25,17,
                                       9,1,58,50,42,34,26,18,10,2,59,51,43,35};
    static const int key_perm_d[28] = {62,54,46,38,30,22,14,6,61,53,45,37,29,21,
                                       13,5,60,52,44,36,28,20,12,4,27,19,11,3};
    static const int key_compression[48] = {13,16,10,23,0,4,2,27,14,5,20,9,
                                            22,18,11,3,25,7,15,6,26,19,12,1,
                                            40,51,30,36,46,54,29,39,50,44,32,47,
                                            43,48,38,55,33,52,45,41,49,35,28,31};

    uint32_t C = 0, D = 0;
    for (int i = 0; i < 28; ++i) {
        C |= des_bitnum(key, key_perm_c[i], 31 - i);
        D |= des_bitnum(key, key_perm_d[i], 31 - i);
    }

    for (int i = 0; i < 16; ++i) {
        C = ((C << key_rnd_shift[i]) | (C >> (28 - key_rnd_shift[i]))) & 0xfffffff0;
        D = ((D << key_rnd_shift[i]) | (D >> (28 - key_rnd_shift[i]))) & 0xfffffff0;

        int to_gen = decrypt ? (15 - i) : i;
        for (int j = 0; j < 6; ++j) schedule[to_gen][j] = 0;
        for (int j = 0; j < 24; ++j)
            schedule[to_gen][j / 8] |= des_bitnum_int_r(C, key_compression[j], 7 - (j % 8));
        for (int j = 24; j < 48; ++j)
            schedule[to_gen][j / 8] |= des_bitnum_int_r(D, key_compression[j] - 28, 7 - (j % 8));
    }
}

static void des_crypt_block(const uint8_t in[8], uint8_t out[8], const uint8_t key[16][6]) {
    uint32_t state[2];
    des_ip(state, in);

    for (int idx = 0; idx < 15; ++idx) {
        uint32_t t = state[1];
        state[1] = des_f(state[1], key[idx]) ^ state[0];
        state[0] = t;
    }
    // Final loop (no switch)
    state[0] = des_f(state[1], key[15]) ^ state[0];

    des_inv_ip(state, out);
}

std::vector<uint8_t> des_encrypt(const uint8_t *key, size_t key_len,
                                 const uint8_t *data, size_t data_len) {
    if (key_len < 8) ThrowFakeluaException("des_encrypt: key must be at least 8 bytes");

    uint8_t schedule[16][6];
    des_key_setup(key, schedule, false);

    // Zero-pad to multiple of 8
    size_t padded = (data_len + 7) & ~size_t(7);
    std::vector<uint8_t> padded_data(data, data + data_len);
    padded_data.resize(padded, 0);

    std::vector<uint8_t> out(padded);
    for (size_t i = 0; i < padded; i += 8) {
        des_crypt_block(padded_data.data() + i, out.data() + i, schedule);
    }
    return out;
}

std::vector<uint8_t> des_decrypt(const uint8_t *key, size_t key_len,
                                 const uint8_t *data, size_t data_len) {
    if (key_len < 8) ThrowFakeluaException("des_decrypt: key must be at least 8 bytes");
    if (data_len % 8 != 0) ThrowFakeluaException("des_decrypt: length must be a multiple of 8");

    uint8_t schedule[16][6];
    des_key_setup(key, schedule, true);

    std::vector<uint8_t> out(data_len);
    for (size_t i = 0; i < data_len; i += 8) {
        des_crypt_block(data + i, out.data() + i, schedule);
    }
    return out;
}

// ── Triple DES (3DES) ──

std::vector<uint8_t> triple_des_encrypt(const uint8_t *key, size_t key_len,
                                        const uint8_t *data, size_t data_len) {
    if (key_len < 24) ThrowFakeluaException("triple_des_encrypt: key must be at least 24 bytes");

    // 3DES = EDE: encrypt with K1, decrypt with K2, encrypt with K3
    std::vector<uint8_t> tmp = des_encrypt(key, 8, data, data_len);
    tmp = des_decrypt(key + 8, 8, tmp.data(), tmp.size());
    return des_encrypt(key + 16, 8, tmp.data(), tmp.size());
}

std::vector<uint8_t> triple_des_decrypt(const uint8_t *key, size_t key_len,
                                        const uint8_t *data, size_t data_len) {
    if (key_len < 24) ThrowFakeluaException("triple_des_decrypt: key must be at least 24 bytes");
    if (data_len % 8 != 0) ThrowFakeluaException("triple_des_decrypt: length must be a multiple of 8");

    // 3DES DED: decrypt with K3, encrypt with K2, decrypt with K1
    std::vector<uint8_t> tmp = des_decrypt(key + 16, 8, data, data_len);
    tmp = des_encrypt(key + 8, 8, tmp.data(), tmp.size());
    return des_decrypt(key, 8, tmp.data(), tmp.size());
}

}  // namespace fakelua::crypto
