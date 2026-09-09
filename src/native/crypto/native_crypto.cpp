#include "native/crypto/native_crypto.h"
#include "native/crypto/crypto_digest.h"
#include "native/crypto/crypto_cipher.h"
#include "native/crypto/crypto_aes.h"
#include "native/native_common.h"
#include "util/logging.h"

#include <format>
#include <mutex>
#include <string>
#include <openssl/evp.h>
#include <openssl/opensslv.h>
#if OPENSSL_VERSION_MAJOR >= 3
#  include <openssl/provider.h>
#endif

namespace fakelua::crypto {

// crypto.md5(data) → hex string
static CVar crypto_md5(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "crypto.md5", "data expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    std::string data = inter::FakeluaToNativeString(s, a0);
    auto digest = md5(data);
    return inter::NativeToFakeluaString(s, to_hex(digest.data(), digest.size()));
}

// crypto.sha1(data) → hex string
static CVar crypto_sha1(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "crypto.sha1", "data expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    std::string data = inter::FakeluaToNativeString(s, a0);
    auto digest = sha1(data);
    return inter::NativeToFakeluaString(s, to_hex(digest.data(), digest.size()));
}

// crypto.sha256(data) → hex string
static CVar crypto_sha256(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "crypto.sha256", "data expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    std::string data = inter::FakeluaToNativeString(s, a0);
    auto digest = sha256(data);
    return inter::NativeToFakeluaString(s, to_hex(digest.data(), digest.size()));
}

// ── AES helpers ──

// Read a 16-byte key/iv from a Lua string argument
static void read_key_arg(State *s, CVar arg, uint8_t out[16], const char *name) {
    std::string data = inter::FakeluaToNativeString(s, arg);
    if (data.size() != 16) {
        ThrowFakeluaException(std::format("{} must be exactly 16 bytes", name));
    }
    memcpy(out, data.data(), 16);
}

// Read arbitrary-length data from a Lua string argument
static std::string read_data_arg(State *s, CVar arg) {
    return inter::FakeluaToNativeString(s, arg);
}

// crypto.aes_encrypt_ecb(data, key) → encrypted data (16-byte blocks)
static CVar crypto_aes_encrypt_ecb(State *s, CVar *args, int n) {
    if (n < 2) ThrowBadArgument(1, "crypto.aes_encrypt_ecb", "data and key expected");
    std::string data = read_data_arg(s, inter::GetNativeArg(s, args, n, 0));
    CVar key_arg = inter::GetNativeArg(s, args, n, 1);
    uint8_t key[16];
    read_key_arg(s, key_arg, key, "key");

    if (data.size() % 16 != 0) {
        LOG_ERROR("crypto", "aes_encrypt_ecb: data length {} not multiple of 16", data.size());
        ThrowFakeluaException("crypto.aes_encrypt_ecb: data length must be a multiple of 16");
    }

    std::string out(data.size(), '\0');
    for (size_t i = 0; i < data.size(); i += 16) {
        aes_encrypt_ecb(reinterpret_cast<const uint8_t *>(data.data() + i),
                        reinterpret_cast<uint8_t *>(out.data() + i),
                        key, AesKeySize::AES_128);
    }
    LOG_DEBUG("crypto", "aes_encrypt_ecb: len={}", data.size());
    return inter::NativeToFakeluaString(s, out);
}

// crypto.aes_decrypt_ecb(data, key) → decrypted data
static CVar crypto_aes_decrypt_ecb(State *s, CVar *args, int n) {
    if (n < 2) ThrowBadArgument(1, "crypto.aes_decrypt_ecb", "data and key expected");
    std::string data = read_data_arg(s, inter::GetNativeArg(s, args, n, 0));
    CVar key_arg = inter::GetNativeArg(s, args, n, 1);
    uint8_t key[16];
    read_key_arg(s, key_arg, key, "key");

    if (data.size() % 16 != 0) {
        LOG_ERROR("crypto", "aes_decrypt_ecb: data length {} not multiple of 16", data.size());
        ThrowFakeluaException("crypto.aes_decrypt_ecb: data length must be a multiple of 16");
    }

    std::string out(data.size(), '\0');
    for (size_t i = 0; i < data.size(); i += 16) {
        aes_decrypt_ecb(reinterpret_cast<const uint8_t *>(data.data() + i),
                        reinterpret_cast<uint8_t *>(out.data() + i),
                        key, AesKeySize::AES_128);
    }
    LOG_DEBUG("crypto", "aes_decrypt_ecb: len={}", data.size());
    return inter::NativeToFakeluaString(s, out);
}

// crypto.aes_encrypt_cbc(data, key, iv) → encrypted data (with PKCS#7 padding)
static CVar crypto_aes_encrypt_cbc(State *s, CVar *args, int n) {
    if (n < 3) ThrowBadArgument(1, "crypto.aes_encrypt_cbc", "data, key, and iv expected");
    std::string data = read_data_arg(s, inter::GetNativeArg(s, args, n, 0));
    CVar key_arg = inter::GetNativeArg(s, args, n, 1);
    CVar iv_arg = inter::GetNativeArg(s, args, n, 2);
    uint8_t key[16], iv[16];
    read_key_arg(s, key_arg, key, "key");
    read_key_arg(s, iv_arg, iv, "iv");

    auto out = aes_encrypt_cbc(reinterpret_cast<const uint8_t *>(data.data()), data.size(),
                               key, AesKeySize::AES_128, iv);
    return inter::NativeToFakeluaString(s, std::string(out.begin(), out.end()));
}

// crypto.aes_decrypt_cbc(data, key, iv) → decrypted data
static CVar crypto_aes_decrypt_cbc(State *s, CVar *args, int n) {
    if (n < 3) ThrowBadArgument(1, "crypto.aes_decrypt_cbc", "data, key, and iv expected");
    std::string data = read_data_arg(s, inter::GetNativeArg(s, args, n, 0));
    CVar key_arg = inter::GetNativeArg(s, args, n, 1);
    CVar iv_arg = inter::GetNativeArg(s, args, n, 2);
    uint8_t key[16], iv[16];
    read_key_arg(s, key_arg, key, "key");
    read_key_arg(s, iv_arg, iv, "iv");

    auto out = aes_decrypt_cbc(reinterpret_cast<const uint8_t *>(data.data()), data.size(),
                               key, AesKeySize::AES_128, iv);
    return inter::NativeToFakeluaString(s, std::string(out.begin(), out.end()));
}

// crypto.aes_encrypt_ctr(data, key, iv) → encrypted data (no padding)
static CVar crypto_aes_encrypt_ctr(State *s, CVar *args, int n) {
    if (n < 3) ThrowBadArgument(1, "crypto.aes_encrypt_ctr", "data, key, and iv expected");
    std::string data = read_data_arg(s, inter::GetNativeArg(s, args, n, 0));
    CVar key_arg = inter::GetNativeArg(s, args, n, 1);
    CVar iv_arg = inter::GetNativeArg(s, args, n, 2);
    uint8_t key[16], iv[16];
    read_key_arg(s, key_arg, key, "key");
    read_key_arg(s, iv_arg, iv, "iv");

    auto out = aes_encrypt_ctr(reinterpret_cast<const uint8_t *>(data.data()), data.size(),
                               key, AesKeySize::AES_128, iv);
    return inter::NativeToFakeluaString(s, std::string(out.begin(), out.end()));
}

// crypto.aes_decrypt_ctr(data, key, iv) → decrypted data
static CVar crypto_aes_decrypt_ctr(State *s, CVar *args, int n) {
    if (n < 3) ThrowBadArgument(1, "crypto.aes_decrypt_ctr", "data, key, and iv expected");
    std::string data = read_data_arg(s, inter::GetNativeArg(s, args, n, 0));
    CVar key_arg = inter::GetNativeArg(s, args, n, 1);
    CVar iv_arg = inter::GetNativeArg(s, args, n, 2);
    uint8_t key[16], iv[16];
    read_key_arg(s, key_arg, key, "key");
    read_key_arg(s, iv_arg, iv, "iv");

    auto out = aes_decrypt_ctr(reinterpret_cast<const uint8_t *>(data.data()), data.size(),
                               key, AesKeySize::AES_128, iv);
    return inter::NativeToFakeluaString(s, std::string(out.begin(), out.end()));
}

// crypto.base64_encode(data) → base64 string
static CVar crypto_base64_encode(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "crypto.base64_encode", "data expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    std::string data = inter::FakeluaToNativeString(s, a0);
    std::string out = base64_encode(
        reinterpret_cast<const uint8_t *>(data.data()), data.size());
    return inter::NativeToFakeluaString(s, out);
}

// crypto.base64_decode(data) → binary data
static CVar crypto_base64_decode(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "crypto.base64_decode", "data expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    std::string data = inter::FakeluaToNativeString(s, a0);
    std::string out = base64_decode(
        reinterpret_cast<const uint8_t *>(data.data()), data.size());
    return inter::NativeToFakeluaString(s, out);
}

// crypto.hex_encode(data) → hex string
static CVar crypto_hex_encode(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "crypto.hex_encode", "data expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    std::string data = inter::FakeluaToNativeString(s, a0);
    return inter::NativeToFakeluaString(s, to_hex(reinterpret_cast<const uint8_t *>(data.data()), data.size()));
}

// crypto.hex_decode(hex) → binary data
static CVar crypto_hex_decode(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "crypto.hex_decode", "hex string expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    std::string hex = inter::FakeluaToNativeString(s, a0);
    if (hex.size() % 2 != 0) {
        ThrowFakeluaException("crypto.hex_decode: hex string must have even length");
    }
    std::string out;
    out.reserve(hex.size() / 2);
    auto nibble = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    };
    for (size_t i = 0; i < hex.size(); i += 2) {
        int hi = nibble(hex[i]);
        int lo = nibble(hex[i + 1]);
        if (hi < 0 || lo < 0) {
            ThrowFakeluaException("crypto.hex_decode: invalid hex character");
        }
        out.push_back(static_cast<char>((hi << 4) | lo));
    }
    return inter::NativeToFakeluaString(s, out);
}

// crypto.rc4(key, data) → encrypted/decrypted data (RC4 is symmetric)
static CVar crypto_rc4(State *s, CVar *args, int n) {
    if (n < 2) ThrowBadArgument(1, "crypto.rc4", "key and data expected");
    CVar key_arg = inter::GetNativeArg(s, args, n, 0);
    CVar data_arg = inter::GetNativeArg(s, args, n, 1);
    std::string key = inter::FakeluaToNativeString(s, key_arg);
    std::string data = inter::FakeluaToNativeString(s, data_arg);
    auto out = rc4(key, data);
    return inter::NativeToFakeluaString(s, std::string(out.begin(), out.end()));
}


// crypto.blowfish_encrypt(key, data) → encrypted data (ECB, zero-padded)
static CVar crypto_blowfish_encrypt(State *s, CVar *args, int n) {
    if (n < 2) ThrowBadArgument(1, "crypto.blowfish_encrypt", "key and data expected");
    CVar key_arg = inter::GetNativeArg(s, args, n, 0);
    CVar data_arg = inter::GetNativeArg(s, args, n, 1);
    std::string key = inter::FakeluaToNativeString(s, key_arg);
    std::string data = inter::FakeluaToNativeString(s, data_arg);
    auto out = blowfish_encrypt(
        reinterpret_cast<const uint8_t *>(key.data()), key.size(),
        reinterpret_cast<const uint8_t *>(data.data()), data.size());
    return inter::NativeToFakeluaString(s, std::string(out.begin(), out.end()));
}

// crypto.blowfish_decrypt(key, data) → decrypted data
static CVar crypto_blowfish_decrypt(State *s, CVar *args, int n) {
    if (n < 2) ThrowBadArgument(1, "crypto.blowfish_decrypt", "key and data expected");
    CVar key_arg = inter::GetNativeArg(s, args, n, 0);
    CVar data_arg = inter::GetNativeArg(s, args, n, 1);
    std::string key = inter::FakeluaToNativeString(s, key_arg);
    std::string data = inter::FakeluaToNativeString(s, data_arg);
    auto out = blowfish_decrypt(
        reinterpret_cast<const uint8_t *>(key.data()), key.size(),
        reinterpret_cast<const uint8_t *>(data.data()), data.size());
    return inter::NativeToFakeluaString(s, std::string(out.begin(), out.end()));
}

// crypto.des_encrypt(key, data) → encrypted data (ECB, zero-padded, key >= 8 bytes)
static CVar crypto_des_encrypt(State *s, CVar *args, int n) {
    if (n < 2) ThrowBadArgument(1, "crypto.des_encrypt", "key and data expected");
    CVar key_arg = inter::GetNativeArg(s, args, n, 0);
    CVar data_arg = inter::GetNativeArg(s, args, n, 1);
    std::string key = inter::FakeluaToNativeString(s, key_arg);
    std::string data = inter::FakeluaToNativeString(s, data_arg);
    auto out = des_encrypt(
        reinterpret_cast<const uint8_t *>(key.data()), key.size(),
        reinterpret_cast<const uint8_t *>(data.data()), data.size());
    return inter::NativeToFakeluaString(s, std::string(out.begin(), out.end()));
}

// crypto.des_decrypt(key, data) → decrypted data
static CVar crypto_des_decrypt(State *s, CVar *args, int n) {
    if (n < 2) ThrowBadArgument(1, "crypto.des_decrypt", "key and data expected");
    CVar key_arg = inter::GetNativeArg(s, args, n, 0);
    CVar data_arg = inter::GetNativeArg(s, args, n, 1);
    std::string key = inter::FakeluaToNativeString(s, key_arg);
    std::string data = inter::FakeluaToNativeString(s, data_arg);
    auto out = des_decrypt(
        reinterpret_cast<const uint8_t *>(key.data()), key.size(),
        reinterpret_cast<const uint8_t *>(data.data()), data.size());
    return inter::NativeToFakeluaString(s, std::string(out.begin(), out.end()));
}

// crypto.triple_des_encrypt(key, data) → encrypted data (key >= 24 bytes)
static CVar crypto_triple_des_encrypt(State *s, CVar *args, int n) {
    if (n < 2) ThrowBadArgument(1, "crypto.triple_des_encrypt", "key and data expected");
    CVar key_arg = inter::GetNativeArg(s, args, n, 0);
    CVar data_arg = inter::GetNativeArg(s, args, n, 1);
    std::string key = inter::FakeluaToNativeString(s, key_arg);
    std::string data = inter::FakeluaToNativeString(s, data_arg);
    auto out = triple_des_encrypt(
        reinterpret_cast<const uint8_t *>(key.data()), key.size(),
        reinterpret_cast<const uint8_t *>(data.data()), data.size());
    return inter::NativeToFakeluaString(s, std::string(out.begin(), out.end()));
}

// crypto.triple_des_decrypt(key, data) → decrypted data
static CVar crypto_triple_des_decrypt(State *s, CVar *args, int n) {
    if (n < 2) ThrowBadArgument(1, "crypto.triple_des_decrypt", "key and data expected");
    CVar key_arg = inter::GetNativeArg(s, args, n, 0);
    CVar data_arg = inter::GetNativeArg(s, args, n, 1);
    std::string key = inter::FakeluaToNativeString(s, key_arg);
    std::string data = inter::FakeluaToNativeString(s, data_arg);
    auto out = triple_des_decrypt(
        reinterpret_cast<const uint8_t *>(key.data()), key.size(),
        reinterpret_cast<const uint8_t *>(data.data()), data.size());
    return inter::NativeToFakeluaString(s, std::string(out.begin(), out.end()));
}


// =============================================================================
// Generic EVP interface — exposes ALL OpenSSL algorithms to Lua
// =============================================================================
//
// crypto.digest(algo, data)           → hex string
//   algo: any OpenSSL digest name, e.g. "md5", "sha1", "sha256", "sha512",
//         "sha3-256", "blake2b512", "sm3", etc.
//
// crypto.encrypt(algo, key, iv, data [, no_padding])  → binary string
// crypto.decrypt(algo, key, iv, data [, no_padding])  → binary string
//   algo: any OpenSSL cipher name, e.g.
//         "aes-128-cbc", "aes-256-gcm", "chacha20", "sm4-cbc",
//         "camellia-128-ecb", "rc4", "bf-ecb", "des-ecb", "des-ede3-ecb", ...
//   key : raw key bytes; length must match cipher requirements (or be accepted
//         by EVP_CIPHER_CTX_set_key_length for variable-length ciphers).
//   iv  : raw IV bytes; pass empty string "" for ciphers that don't use an IV.
//   no_padding (optional, default false): pass true to disable EVP PKCS#7
//         padding (required when padding is handled externally or not needed).
//
// Note on AEAD modes (GCM, CCM, OCB):
//   These require additional tag/aad handling not yet exposed here. For now,
//   only non-AEAD ciphers are reliably supported.
// =============================================================================

// ─────────────────────────────────────────────────────────────────────────────
// Internal: run an EVP cipher operation (encrypt or decrypt)
// ─────────────────────────────────────────────────────────────────────────────
static std::vector<uint8_t> evp_cipher_op(
        const std::string &algo,
        const std::string &key,
        const std::string &iv,
        const uint8_t *data, size_t data_len,
        bool encrypt, bool no_padding) {

    const EVP_CIPHER *cipher = EVP_get_cipherbyname(algo.c_str());
    if (!cipher) {
        ThrowFakeluaException(std::format(
            "crypto.{}: unknown cipher '{}' — use an OpenSSL cipher name such as "
            "'aes-128-cbc', 'aes-256-ecb', 'chacha20', 'bf-ecb', 'rc4', 'des-ede3-ecb'",
            encrypt ? "encrypt" : "decrypt", algo));
    }

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        ThrowFakeluaException(std::format(
            "crypto.{}: failed to create EVP_CIPHER_CTX", encrypt ? "encrypt" : "decrypt"));
    }

    // Phase 1: init with cipher only (no key/iv yet) so we can adjust key length
    auto init_fn = encrypt ? EVP_EncryptInit_ex : EVP_DecryptInit_ex;
    if (init_fn(ctx, cipher, nullptr, nullptr, nullptr) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException(std::format("crypto.{}: EVP init failed for cipher '{}'",
            encrypt ? "encrypt" : "decrypt", algo));
    }

    // For variable-key-length ciphers (RC4, Blowfish, ChaCha20, ...),
    // set the actual key length before providing the key bytes.
    int expected_key_len = EVP_CIPHER_CTX_key_length(ctx);
    if (static_cast<int>(key.size()) != expected_key_len) {
        if (EVP_CIPHER_CTX_set_key_length(ctx, static_cast<int>(key.size())) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            ThrowFakeluaException(std::format(
                "crypto.{}: key length {} is not valid for cipher '{}' (expected {} bytes)",
                encrypt ? "encrypt" : "decrypt",
                key.size(), algo, expected_key_len));
        }
    }

    // Validate IV length
    int cipher_iv_len = EVP_CIPHER_CTX_iv_length(ctx);
    const uint8_t *iv_ptr = nullptr;
    if (cipher_iv_len > 0) {
        if (static_cast<int>(iv.size()) != cipher_iv_len) {
            EVP_CIPHER_CTX_free(ctx);
            ThrowFakeluaException(std::format(
                "crypto.{}: IV length {} is not valid for cipher '{}' (expected {} bytes)",
                encrypt ? "encrypt" : "decrypt",
                iv.size(), algo, cipher_iv_len));
        }
        iv_ptr = reinterpret_cast<const uint8_t *>(iv.data());
    } else if (!iv.empty()) {
        // Cipher doesn't use an IV but one was provided — warn-and-ignore is safer
        // than error, since callers often pass a dummy IV regardless.
    }

    // Phase 2: provide key and IV
    if (init_fn(ctx, nullptr, nullptr,
                reinterpret_cast<const uint8_t *>(key.data()), iv_ptr) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException(std::format("crypto.{}: failed to set key/IV for cipher '{}'",
            encrypt ? "encrypt" : "decrypt", algo));
    }

    // Configure padding
    if (EVP_CIPHER_CTX_set_padding(ctx, no_padding ? 0 : 1) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException(std::format("crypto.{}: EVP_CIPHER_CTX_set_padding failed",
            encrypt ? "encrypt" : "decrypt"));
    }

    // Allocate output: max size = data_len + one full block for padding
    int block_size = EVP_CIPHER_CTX_block_size(ctx);
    if (block_size < 1) block_size = 1;
    std::vector<uint8_t> out(data_len + static_cast<size_t>(block_size));

    int outlen = 0;
    if (data_len > 0) {
        auto update_fn = encrypt ? EVP_EncryptUpdate : EVP_DecryptUpdate;
        if (update_fn(ctx, out.data(), &outlen, data, static_cast<int>(data_len)) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            ThrowFakeluaException(std::format("crypto.{}: EVP_Update failed for cipher '{}'",
                encrypt ? "encrypt" : "decrypt", algo));
        }
    }

    int final_len = 0;
    auto final_fn = encrypt ? EVP_EncryptFinal_ex : EVP_DecryptFinal_ex;
    if (final_fn(ctx, out.data() + outlen, &final_len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException(std::format(
            "crypto.{}: EVP_Final failed for cipher '{}' — "
            "check key/IV length, padding setting, or data integrity",
            encrypt ? "encrypt" : "decrypt", algo));
    }

    out.resize(static_cast<size_t>(outlen) + static_cast<size_t>(final_len));
    EVP_CIPHER_CTX_free(ctx);
    return out;
}

// ─────────────────────────────────────────────────────────────────────────────
// crypto.digest(algo, data) → hex string
// ─────────────────────────────────────────────────────────────────────────────
static CVar crypto_digest(State *s, CVar *args, int n) {
    if (n < 2) ThrowBadArgument(1, "crypto.digest", "algo and data expected");
    std::string algo = inter::FakeluaToNativeString(s, inter::GetNativeArg(s, args, n, 0));
    std::string data = inter::FakeluaToNativeString(s, inter::GetNativeArg(s, args, n, 1));

    const EVP_MD *md = EVP_get_digestbyname(algo.c_str());
    if (!md) {
        ThrowFakeluaException(std::format(
            "crypto.digest: unknown digest algorithm '{}' — use an OpenSSL digest name "
            "such as 'md5', 'sha1', 'sha256', 'sha512', 'sha3-256', 'blake2b512', 'sm3'",
            algo));
    }

    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (!ctx) ThrowFakeluaException("crypto.digest: failed to create EVP_MD_CTX");

    if (EVP_DigestInit_ex(ctx, md, nullptr) != 1) {
        EVP_MD_CTX_free(ctx);
        ThrowFakeluaException(std::format("crypto.digest: EVP_DigestInit_ex failed for '{}'", algo));
    }
    if (!data.empty() && EVP_DigestUpdate(ctx, data.data(), data.size()) != 1) {
        EVP_MD_CTX_free(ctx);
        ThrowFakeluaException(std::format("crypto.digest: EVP_DigestUpdate failed for '{}'", algo));
    }

    uint8_t digest_buf[EVP_MAX_MD_SIZE];
    unsigned int digest_len = 0;
    if (EVP_DigestFinal_ex(ctx, digest_buf, &digest_len) != 1) {
        EVP_MD_CTX_free(ctx);
        ThrowFakeluaException(std::format("crypto.digest: EVP_DigestFinal_ex failed for '{}'", algo));
    }
    EVP_MD_CTX_free(ctx);

    return inter::NativeToFakeluaString(s, to_hex(digest_buf, digest_len));
}

// ─────────────────────────────────────────────────────────────────────────────
// crypto.digest_raw(algo, data) → raw binary bytes (not hex-encoded)
// Useful when the digest output needs to be used as input to another operation.
// ─────────────────────────────────────────────────────────────────────────────
static CVar crypto_digest_raw(State *s, CVar *args, int n) {
    if (n < 2) ThrowBadArgument(1, "crypto.digest_raw", "algo and data expected");
    std::string algo = inter::FakeluaToNativeString(s, inter::GetNativeArg(s, args, n, 0));
    std::string data = inter::FakeluaToNativeString(s, inter::GetNativeArg(s, args, n, 1));

    const EVP_MD *md = EVP_get_digestbyname(algo.c_str());
    if (!md) {
        ThrowFakeluaException(std::format(
            "crypto.digest_raw: unknown digest algorithm '{}'", algo));
    }

    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (!ctx) ThrowFakeluaException("crypto.digest_raw: failed to create EVP_MD_CTX");

    if (EVP_DigestInit_ex(ctx, md, nullptr) != 1) {
        EVP_MD_CTX_free(ctx);
        ThrowFakeluaException(std::format("crypto.digest_raw: EVP_DigestInit_ex failed for '{}'", algo));
    }
    if (!data.empty() && EVP_DigestUpdate(ctx, data.data(), data.size()) != 1) {
        EVP_MD_CTX_free(ctx);
        ThrowFakeluaException(std::format("crypto.digest_raw: EVP_DigestUpdate failed for '{}'", algo));
    }

    uint8_t digest_buf[EVP_MAX_MD_SIZE];
    unsigned int digest_len = 0;
    if (EVP_DigestFinal_ex(ctx, digest_buf, &digest_len) != 1) {
        EVP_MD_CTX_free(ctx);
        ThrowFakeluaException(std::format("crypto.digest_raw: EVP_DigestFinal_ex failed for '{}'", algo));
    }
    EVP_MD_CTX_free(ctx);

    return inter::NativeToFakeluaString(s, std::string(reinterpret_cast<const char *>(digest_buf), digest_len));
}

// ─────────────────────────────────────────────────────────────────────────────
// crypto.encrypt(algo, key, iv, data [, no_padding]) → binary string
// ─────────────────────────────────────────────────────────────────────────────
static CVar crypto_encrypt_evp(State *s, CVar *args, int n) {
    if (n < 4) ThrowBadArgument(1, "crypto.encrypt", "algo, key, iv, data expected");
    std::string algo = inter::FakeluaToNativeString(s, inter::GetNativeArg(s, args, n, 0));
    std::string key  = inter::FakeluaToNativeString(s, inter::GetNativeArg(s, args, n, 1));
    std::string iv   = inter::FakeluaToNativeString(s, inter::GetNativeArg(s, args, n, 2));
    std::string data = inter::FakeluaToNativeString(s, inter::GetNativeArg(s, args, n, 3));
    bool no_padding = false;
    if (n >= 5) {
        no_padding = inter::FakeluaToNativeBool(s, inter::GetNativeArg(s, args, n, 4));
    }

    auto out = evp_cipher_op(algo, key, iv,
        reinterpret_cast<const uint8_t *>(data.data()), data.size(), true, no_padding);
    return inter::NativeToFakeluaString(s, std::string(out.begin(), out.end()));
}

// ─────────────────────────────────────────────────────────────────────────────
// crypto.decrypt(algo, key, iv, data [, no_padding]) → binary string
// ─────────────────────────────────────────────────────────────────────────────
static CVar crypto_decrypt_evp(State *s, CVar *args, int n) {
    if (n < 4) ThrowBadArgument(1, "crypto.decrypt", "algo, key, iv, data expected");
    std::string algo = inter::FakeluaToNativeString(s, inter::GetNativeArg(s, args, n, 0));
    std::string key  = inter::FakeluaToNativeString(s, inter::GetNativeArg(s, args, n, 1));
    std::string iv   = inter::FakeluaToNativeString(s, inter::GetNativeArg(s, args, n, 2));
    std::string data = inter::FakeluaToNativeString(s, inter::GetNativeArg(s, args, n, 3));
    bool no_padding = false;
    if (n >= 5) {
        no_padding = inter::FakeluaToNativeBool(s, inter::GetNativeArg(s, args, n, 4));
    }

    auto out = evp_cipher_op(algo, key, iv,
        reinterpret_cast<const uint8_t *>(data.data()), data.size(), false, no_padding);
    return inter::NativeToFakeluaString(s, std::string(out.begin(), out.end()));
}

void RegisterCryptoLibraryApi(State *s) {
    if (!s) return;

    // On OpenSSL 3.x, RC4 / Blowfish / DES are in the "legacy" provider which
    // is not loaded by default. Load it once here so callers don't have to
    // configure openssl.cnf manually. Failure is silently ignored: if the
    // legacy provider is already loaded (via config), or if this OpenSSL build
    // doesn't have it, the rest of the API still works.
    static std::once_flag legacy_provider_init;
    std::call_once(legacy_provider_init, []() {
#if OPENSSL_VERSION_MAJOR >= 3
        OSSL_PROVIDER_load(nullptr, "legacy");
        OSSL_PROVIDER_load(nullptr, "default");
#endif
    });

    RegisterNativeFunction(s, "crypto.md5", 1, false, crypto_md5);
    RegisterNativeFunction(s, "crypto.sha1", 1, false, crypto_sha1);
    RegisterNativeFunction(s, "crypto.sha256", 1, false, crypto_sha256);
    RegisterNativeFunction(s, "crypto.hex_encode", 1, false, crypto_hex_encode);
    RegisterNativeFunction(s, "crypto.hex_decode", 1, false, crypto_hex_decode);
    RegisterNativeFunction(s, "crypto.base64_encode", 1, false, crypto_base64_encode);
    RegisterNativeFunction(s, "crypto.base64_decode", 1, false, crypto_base64_decode);
    RegisterNativeFunction(s, "crypto.aes_encrypt_ecb", 2, false, crypto_aes_encrypt_ecb);
    RegisterNativeFunction(s, "crypto.aes_decrypt_ecb", 2, false, crypto_aes_decrypt_ecb);
    RegisterNativeFunction(s, "crypto.aes_encrypt_cbc", 3, false, crypto_aes_encrypt_cbc);
    RegisterNativeFunction(s, "crypto.aes_decrypt_cbc", 3, false, crypto_aes_decrypt_cbc);
    RegisterNativeFunction(s, "crypto.aes_encrypt_ctr", 3, false, crypto_aes_encrypt_ctr);
    RegisterNativeFunction(s, "crypto.aes_decrypt_ctr", 3, false, crypto_aes_decrypt_ctr);
    RegisterNativeFunction(s, "crypto.rc4", 2, false, crypto_rc4);
    RegisterNativeFunction(s, "crypto.blowfish_encrypt", 2, false, crypto_blowfish_encrypt);
    RegisterNativeFunction(s, "crypto.blowfish_decrypt", 2, false, crypto_blowfish_decrypt);
    RegisterNativeFunction(s, "crypto.des_encrypt", 2, false, crypto_des_encrypt);
    RegisterNativeFunction(s, "crypto.des_decrypt", 2, false, crypto_des_decrypt);
    RegisterNativeFunction(s, "crypto.triple_des_encrypt", 2, false, crypto_triple_des_encrypt);
    RegisterNativeFunction(s, "crypto.triple_des_decrypt", 2, false, crypto_triple_des_decrypt);
    // ── Generic EVP interface ──
    RegisterNativeFunction(s, "crypto.digest", 2, false, crypto_digest);
    RegisterNativeFunction(s, "crypto.digest_raw", 2, false, crypto_digest_raw);
    RegisterNativeFunction(s, "crypto.encrypt", 4, true, crypto_encrypt_evp);
    RegisterNativeFunction(s, "crypto.decrypt", 4, true, crypto_decrypt_evp);
}


}  // namespace fakelua::crypto
