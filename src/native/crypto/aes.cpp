#include "native/crypto/aes.h"

#include <cstring>

namespace fakelua::crypto {

// ─────────────────────────────────────────────────────────────────────────────
// AES S-Box and inverse S-Box
// ─────────────────────────────────────────────────────────────────────────────

static const uint8_t sbox[256] = {
    0x63,0x7c,0x77,0x7b,0xf2,0x6b,0x6f,0xc5,0x30,0x01,0x67,0x2b,0xfe,0xd7,0xab,0x76,
    0xca,0x82,0xc9,0x7d,0xfa,0x59,0x47,0xf0,0xad,0xd4,0xa2,0xaf,0x9c,0xa4,0x72,0xc0,
    0xb7,0xfd,0x93,0x26,0x36,0x3f,0xf7,0xcc,0x34,0xa5,0xe5,0xf1,0x71,0xd8,0x31,0x15,
    0x04,0xc7,0x23,0xc3,0x18,0x96,0x05,0x9a,0x07,0x12,0x80,0xe2,0xeb,0x27,0xb2,0x75,
    0x09,0x83,0x2c,0x1a,0x1b,0x6e,0x5a,0xa0,0x52,0x3b,0xd6,0xb3,0x29,0xe3,0x2f,0x84,
    0x53,0xd1,0x00,0xed,0x20,0xfc,0xb1,0x5b,0x6a,0xcb,0xbe,0x39,0x4a,0x4c,0x58,0xcf,
    0xd0,0xef,0xaa,0xfb,0x43,0x4d,0x33,0x85,0x45,0xf9,0x02,0x7f,0x50,0x3c,0x9f,0xa8,
    0x51,0xa3,0x40,0x8f,0x92,0x9d,0x38,0xf5,0xbc,0xb6,0xda,0x21,0x10,0xff,0xf3,0xd2,
    0xcd,0x0c,0x13,0xec,0x5f,0x97,0x44,0x17,0xc4,0xa7,0x7e,0x3d,0x64,0x5d,0x19,0x73,
    0x60,0x81,0x4f,0xdc,0x22,0x2a,0x90,0x88,0x46,0xee,0xb8,0x14,0xde,0x5e,0x0b,0xdb,
    0xe0,0x32,0x3a,0x0a,0x49,0x06,0x24,0x5c,0xc2,0xd3,0xac,0x62,0x91,0x95,0xe4,0x79,
    0xe7,0xc8,0x37,0x6d,0x8d,0xd5,0x4e,0xa9,0x6c,0x56,0xf4,0xea,0x65,0x7a,0xae,0x08,
    0xba,0x78,0x25,0x2e,0x1c,0xa6,0xb4,0xc6,0xe8,0xdd,0x74,0x1f,0x4b,0xbd,0x8b,0x8a,
    0x70,0x3e,0xb5,0x66,0x48,0x03,0xf6,0x0e,0x61,0x35,0x57,0xb9,0x86,0xc1,0x1d,0x9e,
    0xe1,0xf8,0x98,0x11,0x69,0xd9,0x8e,0x94,0x9b,0x1e,0x87,0xe9,0xce,0x55,0x28,0xdf,
    0x8c,0xa1,0x89,0x0d,0xbf,0xe6,0x42,0x68,0x41,0x99,0x2d,0x0f,0xb0,0x54,0xbb,0x16,
};

static const uint8_t inv_sbox[256] = {
    0x52,0x09,0x6a,0xd5,0x30,0x36,0xa5,0x38,0xbf,0x40,0xa3,0x9e,0x81,0xf3,0xd7,0xfb,
    0x7c,0xe3,0x39,0x82,0x9b,0x2f,0xff,0x87,0x34,0x8e,0x43,0x44,0xc4,0xde,0xe9,0xcb,
    0x54,0x7b,0x94,0x32,0xa6,0xc2,0x23,0x3d,0xee,0x4c,0x95,0x0b,0x42,0xfa,0xc3,0x4e,
    0x08,0x2e,0xa1,0x66,0x28,0xd9,0x24,0xb2,0x76,0x5b,0xa2,0x49,0x6d,0x8b,0xd1,0x25,
    0x72,0xf8,0xf6,0x64,0x86,0x68,0x98,0x16,0xd4,0xa4,0x5c,0xcc,0x5d,0x65,0xb6,0x92,
    0x6c,0x70,0x48,0x50,0xfd,0xed,0xb9,0xda,0x5e,0x15,0x46,0x57,0xa7,0x8d,0x9d,0x84,
    0x90,0xd8,0xab,0x00,0x8c,0xbc,0xd3,0x0a,0xf7,0xe4,0x58,0x05,0xb8,0xb3,0x45,0x06,
    0xd0,0x2c,0x1e,0x8f,0xca,0x3f,0x0f,0x02,0xc1,0xaf,0xbd,0x03,0x01,0x13,0x8a,0x6b,
    0x3a,0x91,0x11,0x41,0x4f,0x67,0xdc,0xea,0x97,0xf2,0xcf,0xce,0xf0,0xb4,0xe6,0x73,
    0x96,0xac,0x74,0x22,0xe7,0xad,0x35,0x85,0xe2,0xf9,0x37,0xe8,0x1c,0x75,0xdf,0x6e,
    0x47,0xf1,0x1a,0x71,0x1d,0x29,0xc5,0x89,0x6f,0xb7,0x62,0x0e,0xaa,0x18,0xbe,0x1b,
    0xfc,0x56,0x3e,0x4b,0xc6,0xd2,0x79,0x20,0x9a,0xdb,0xc0,0xfe,0x78,0xcd,0x5a,0xf4,
    0x1f,0xdd,0xa8,0x33,0x88,0x07,0xc7,0x31,0xb1,0x12,0x10,0x59,0x27,0x80,0xec,0x5f,
    0x60,0x51,0x7f,0xa9,0x19,0xb5,0x4a,0x0d,0x2d,0xe5,0x7a,0x9f,0x93,0xc9,0x9c,0xef,
    0xa0,0xe0,0x3b,0x4d,0xae,0x2a,0xf5,0xb0,0xc8,0xeb,0xbb,0x3c,0x83,0x53,0x99,0x61,
    0x17,0x2b,0x04,0x7e,0xba,0x77,0xd6,0x26,0xe1,0x69,0x14,0x63,0x55,0x21,0x0c,0x7d,
};

// ── Round constants ──
static const uint8_t rcon[11] = {
    0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36,
};

// ── GF(2^8) multiplication helpers ──
static uint8_t xtime(uint8_t x) {
    return (x << 1) ^ ((x & 0x80) ? 0x1b : 0x00);
}

// ─────────────────────────────────────────────────────────────────────────────
// Key expansion
// ─────────────────────────────────────────────────────────────────────────────

static void key_expansion(const uint8_t key[16], uint8_t *round_keys, int rounds) {
    // For AES-128, key is 16 bytes, round_keys needs 176 bytes (11 * 16)
    // For AES-256, key is 32 bytes, round_keys needs 240 bytes (15 * 16)
    // This function assumes key is 16 bytes (AES-128) for simplicity.
    // For other key sizes, a more complex expansion is needed.

    // Copy the original key
    for (int i = 0; i < 16; ++i) {
        round_keys[i] = key[i];
    }

    uint8_t temp[4];
    int key_len = 16;  // AES-128
    int round_key_len = (rounds + 1) * 16;

    for (int i = key_len; i < round_key_len; i += 4) {
        // Read previous 4 bytes
        for (int j = 0; j < 4; ++j) {
            temp[j] = round_keys[i - 4 + j];
        }

        // Every key_len bytes, apply transformation
        if (i % key_len == 0) {
            // RotWord
            uint8_t t = temp[0];
            temp[0] = temp[1];
            temp[1] = temp[2];
            temp[2] = temp[3];
            temp[3] = t;

            // SubWord
            for (int j = 0; j < 4; ++j) {
                temp[j] = sbox[temp[j]];
            }

            // XOR with Rcon
            temp[0] ^= rcon[i / key_len];
        }

        // XOR with the word key_len bytes back
        for (int j = 0; j < 4; ++j) {
            round_keys[i + j] = round_keys[i + j - key_len] ^ temp[j];
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// AES block cipher primitives
// ─────────────────────────────────────────────────────────────────────────────

static void add_round_key(uint8_t state[16], const uint8_t *round_key) {
    for (int i = 0; i < 16; ++i) {
        state[i] ^= round_key[i];
    }
}

static void sub_bytes(uint8_t state[16]) {
    for (int i = 0; i < 16; ++i) {
        state[i] = sbox[state[i]];
    }
}

static void inv_sub_bytes(uint8_t state[16]) {
    for (int i = 0; i < 16; ++i) {
        state[i] = inv_sbox[state[i]];
    }
}

static void shift_rows(uint8_t state[16]) {
    uint8_t temp;
    // Row 1: shift left by 1
    temp = state[1]; state[1] = state[5]; state[5] = state[9]; state[9] = state[13]; state[13] = temp;
    // Row 2: shift left by 2
    temp = state[2]; state[2] = state[10]; state[10] = temp;
    temp = state[6]; state[6] = state[14]; state[14] = temp;
    // Row 3: shift left by 3
    temp = state[3]; state[3] = state[15]; state[15] = state[11]; state[11] = state[7]; state[7] = temp;
}

static void inv_shift_rows(uint8_t state[16]) {
    uint8_t temp;
    // Row 1: shift right by 1
    temp = state[13]; state[13] = state[9]; state[9] = state[5]; state[5] = state[1]; state[1] = temp;
    // Row 2: shift right by 2
    temp = state[2]; state[2] = state[10]; state[10] = temp;
    temp = state[6]; state[6] = state[14]; state[14] = temp;
    // Row 3: shift right by 3
    temp = state[3]; state[3] = state[7]; state[7] = state[11]; state[11] = state[15]; state[15] = temp;
}

static void mix_columns(uint8_t state[16]) {
    for (int i = 0; i < 4; ++i) {
        int c = i * 4;
        uint8_t a0 = state[c], a1 = state[c+1], a2 = state[c+2], a3 = state[c+3];
        state[c]   = xtime(a0) ^ (xtime(a1) ^ a1) ^ a2 ^ a3;
        state[c+1] = a0 ^ xtime(a1) ^ (xtime(a2) ^ a2) ^ a3;
        state[c+2] = a0 ^ a1 ^ xtime(a2) ^ (xtime(a3) ^ a3);
        state[c+3] = (xtime(a0) ^ a0) ^ a1 ^ a2 ^ xtime(a3);
    }
}

static uint8_t gf_mul(uint8_t a, uint8_t b) {
    uint8_t result = 0;
    for (int i = 0; i < 8; ++i) {
        if (b & 1) result ^= a;
        uint8_t hi = a & 0x80;
        a <<= 1;
        if (hi) a ^= 0x1b;
        b >>= 1;
    }
    return result;
}

static void inv_mix_columns(uint8_t state[16]) {
    for (int i = 0; i < 4; ++i) {
        int c = i * 4;
        uint8_t a0 = state[c], a1 = state[c+1], a2 = state[c+2], a3 = state[c+3];
        state[c]   = gf_mul(a0,14) ^ gf_mul(a1,11) ^ gf_mul(a2,13) ^ gf_mul(a3,9);
        state[c+1] = gf_mul(a0,9) ^ gf_mul(a1,14) ^ gf_mul(a2,11) ^ gf_mul(a3,13);
        state[c+2] = gf_mul(a0,13) ^ gf_mul(a1,9) ^ gf_mul(a2,14) ^ gf_mul(a3,11);
        state[c+3] = gf_mul(a0,11) ^ gf_mul(a1,13) ^ gf_mul(a2,9) ^ gf_mul(a3,14);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// AES-128 encrypt/decrypt single block
// ─────────────────────────────────────────────────────────────────────────────

void aes_encrypt_ecb(const uint8_t in[16], uint8_t out[16],
                     const uint8_t key[16], AesKeySize key_size) {
    int rounds = 10;  // AES-128
    uint8_t round_keys[176];  // 11 * 16 bytes

    key_expansion(key, round_keys, rounds);

    uint8_t state[16];
    memcpy(state, in, 16);

    add_round_key(state, round_keys);

    for (int r = 1; r <= rounds; ++r) {
        sub_bytes(state);
        shift_rows(state);
        if (r < rounds) mix_columns(state);
        add_round_key(state, round_keys + r * 16);
    }

    memcpy(out, state, 16);
}

void aes_decrypt_ecb(const uint8_t in[16], uint8_t out[16],
                     const uint8_t key[16], AesKeySize key_size) {
    int rounds = 10;  // AES-128
    uint8_t round_keys[176];

    key_expansion(key, round_keys, rounds);

    uint8_t state[16];
    memcpy(state, in, 16);

    add_round_key(state, round_keys + rounds * 16);

    for (int r = rounds - 1; r >= 0; --r) {
        inv_shift_rows(state);
        inv_sub_bytes(state);
        add_round_key(state, round_keys + r * 16);
        if (r > 0) inv_mix_columns(state);
    }

    memcpy(out, state, 16);
}

// ─────────────────────────────────────────────────────────────────────────────
// PKCS#7 padding helpers
// ─────────────────────────────────────────────────────────────────────────────

static std::vector<uint8_t> pkcs7_pad(const uint8_t *data, size_t len) {
    uint8_t pad = AES_BLOCK_SIZE - (len % AES_BLOCK_SIZE);
    std::vector<uint8_t> result(len + pad);
    memcpy(result.data(), data, len);
    for (size_t i = len; i < result.size(); ++i) {
        result[i] = pad;
    }
    return result;
}

static std::vector<uint8_t> pkcs7_unpad(const uint8_t *data, size_t len) {
    if (len == 0 || len % AES_BLOCK_SIZE != 0) return {};
    uint8_t pad = data[len - 1];
    if (pad < 1 || pad > AES_BLOCK_SIZE) return {};
    for (size_t i = len - pad; i < len; ++i) {
        if (data[i] != pad) return {};
    }
    return std::vector<uint8_t>(data, data + len - pad);
}

// ─────────────────────────────────────────────────────────────────────────────
// CBC mode
// ─────────────────────────────────────────────────────────────────────────────

std::vector<uint8_t> aes_encrypt_cbc(const uint8_t *data, size_t len,
                                     const uint8_t key[16], AesKeySize key_size,
                                     const uint8_t iv[16]) {
    auto padded = pkcs7_pad(data, len);
    std::vector<uint8_t> result(padded.size());

    uint8_t prev[16];
    memcpy(prev, iv, 16);

    for (size_t i = 0; i < padded.size(); i += AES_BLOCK_SIZE) {
        uint8_t block[16];
        for (int j = 0; j < 16; ++j) {
            block[j] = padded[i + j] ^ prev[j];
        }
        aes_encrypt_ecb(block, result.data() + i, key, key_size);
        memcpy(prev, result.data() + i, 16);
    }

    return result;
}

std::vector<uint8_t> aes_decrypt_cbc(const uint8_t *data, size_t len,
                                     const uint8_t key[16], AesKeySize key_size,
                                     const uint8_t iv[16]) {
    if (len == 0 || len % AES_BLOCK_SIZE != 0) return {};

    std::vector<uint8_t> result(len);
    uint8_t prev[16];
    memcpy(prev, iv, 16);

    for (size_t i = 0; i < len; i += AES_BLOCK_SIZE) {
        uint8_t block[16];
        aes_decrypt_ecb(data + i, block, key, key_size);
        for (int j = 0; j < 16; ++j) {
            result[i + j] = block[j] ^ prev[j];
        }
        memcpy(prev, data + i, 16);
    }

    return pkcs7_unpad(result.data(), result.size());
}

// ─────────────────────────────────────────────────────────────────────────────
// CTR mode
// ─────────────────────────────────────────────────────────────────────────────

// CTR mode: nonce (first 8 bytes) + counter (last 8 bytes, big-endian, starting from 0)
// This matches Python's AES.MODE_CTR convention.
static void increment_counter(uint8_t counter[8]) {
    for (int i = 7; i >= 0; --i) {
        if (++counter[i] != 0) break;
    }
}

std::vector<uint8_t> aes_encrypt_ctr(const uint8_t *data, size_t len,
                                     const uint8_t key[16], AesKeySize key_size,
                                     const uint8_t iv[16]) {
    std::vector<uint8_t> result(len);
    uint8_t counter_block[16];
    // First 8 bytes = nonce (from IV), last 8 bytes = counter (starts at 0)
    memcpy(counter_block, iv, 8);
    memset(counter_block + 8, 0, 8);

    for (size_t i = 0; i < len; i += AES_BLOCK_SIZE) {
        uint8_t keystream[16];
        aes_encrypt_ecb(counter_block, keystream, key, key_size);
        size_t block_len = std::min((size_t)AES_BLOCK_SIZE, len - i);
        for (size_t j = 0; j < block_len; ++j) {
            result[i + j] = data[i + j] ^ keystream[j];
        }
        increment_counter(counter_block + 8);
    }

    return result;
}

std::vector<uint8_t> aes_decrypt_ctr(const uint8_t *data, size_t len,
                                     const uint8_t key[16], AesKeySize key_size,
                                     const uint8_t iv[16]) {
    // CTR mode encryption and decryption are the same operation
    return aes_encrypt_ctr(data, len, key, key_size, iv);
}

}  // namespace fakelua::crypto
