#include "native/crypto/native_crypto.h"
#include "native/crypto/crypto_aes.h"
#include "native/crypto/crypto_cipher.h"
#include "native/crypto/crypto_digest.h"
#include "native/native_common.h"
#include "util/logging.h"

#include <boost/algorithm/hex.hpp>
#include <boost/crc.hpp>
#include <boost/hash2/xxhash.hpp>
#include <boost/uuid.hpp>
#include <cstdint>
#include <format>
#include <iterator>
#include <mutex>
#include <openssl/evp.h>
#include <openssl/opensslv.h>
#include <string>
#if OPENSSL_VERSION_MAJOR >= 3
#include <openssl/provider.h>
#endif

namespace fakelua::crypto {

// crypto.md5(data) → hex string
static CVar CryptoMd5(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "crypto.md5", "data expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    std::string data = inter::FakeluaToNativeString(s, a0);
    auto digest = Md5(data);
    return inter::NativeToFakeluaString(s, ToHex(digest.data(), digest.size()));
}

// crypto.sha1(data) → hex string
static CVar CryptoSha1(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "crypto.sha1", "data expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    std::string data = inter::FakeluaToNativeString(s, a0);
    auto digest = Sha1(data);
    return inter::NativeToFakeluaString(s, ToHex(digest.data(), digest.size()));
}

// crypto.sha256(data) → hex string
static CVar CryptoSha256(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "crypto.sha256", "data expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    std::string data = inter::FakeluaToNativeString(s, a0);
    auto digest = Sha256(data);
    return inter::NativeToFakeluaString(s, ToHex(digest.data(), digest.size()));
}

// AES helpers
// Read a 16-byte key/iv from a Lua string argument
static void ReadKeyArg(State *s, CVar arg, uint8_t out[16], const char *name) {
    std::string data = inter::FakeluaToNativeString(s, arg);
    if (data.size() != 16) {
        ThrowFakeluaException(std::format("{} must be exactly 16 bytes", name));
    }
    memcpy(out, data.data(), 16);
}

// Read arbitrary-length data from a Lua string argument
static std::string ReadDataArg(State *s, CVar arg) {
    return inter::FakeluaToNativeString(s, arg);
}

// crypto.aes_encrypt_ecb(data, key) → encrypted data (16-byte blocks)
static CVar CryptoAesEncryptEcb(State *s, CVar *args, int n) {
    if (n < 2) ThrowBadArgument(1, "crypto.aes_encrypt_ecb", "data and key expected");
    std::string data = ReadDataArg(s, inter::GetNativeArg(s, args, n, 0));
    CVar key_arg = inter::GetNativeArg(s, args, n, 1);
    uint8_t key[16];
    ReadKeyArg(s, key_arg, key, "key");

    if (data.size() % 16 != 0) {
        LOG_ERROR(s, "crypto", "aes_encrypt_ecb: data length {} not multiple of 16", data.size());
        ThrowFakeluaException("crypto.aes_encrypt_ecb: data length must be a multiple of 16");
    }

    std::string out(data.size(), '\0');
    for (size_t i = 0; i < data.size(); i += 16) {
        AesEncryptEcb(reinterpret_cast<const uint8_t *>(data.data() + i), reinterpret_cast<uint8_t *>(out.data() + i), key, AesKeySize::AES_128);
    }
    LOG_DEBUG(s, "crypto", "aes_encrypt_ecb: len={}", data.size());
    return inter::NativeToFakeluaString(s, out);
}

// crypto.aes_decrypt_ecb(data, key) → decrypted data
static CVar CryptoAesDecryptEcb(State *s, CVar *args, int n) {
    if (n < 2) ThrowBadArgument(1, "crypto.aes_decrypt_ecb", "data and key expected");
    std::string data = ReadDataArg(s, inter::GetNativeArg(s, args, n, 0));
    CVar key_arg = inter::GetNativeArg(s, args, n, 1);
    uint8_t key[16];
    ReadKeyArg(s, key_arg, key, "key");

    if (data.size() % 16 != 0) {
        LOG_ERROR(s, "crypto", "aes_decrypt_ecb: data length {} not multiple of 16", data.size());
        ThrowFakeluaException("crypto.aes_decrypt_ecb: data length must be a multiple of 16");
    }

    std::string out(data.size(), '\0');
    for (size_t i = 0; i < data.size(); i += 16) {
        AesDecryptEcb(reinterpret_cast<const uint8_t *>(data.data() + i), reinterpret_cast<uint8_t *>(out.data() + i), key, AesKeySize::AES_128);
    }
    LOG_DEBUG(s, "crypto", "aes_decrypt_ecb: len={}", data.size());
    return inter::NativeToFakeluaString(s, out);
}

// crypto.aes_encrypt_cbc(data, key, iv) → encrypted data (with PKCS#7 padding)
static CVar CryptoAesEncryptCbc(State *s, CVar *args, int n) {
    if (n < 3) ThrowBadArgument(1, "crypto.aes_encrypt_cbc", "data, key, and iv expected");
    std::string data = ReadDataArg(s, inter::GetNativeArg(s, args, n, 0));
    CVar key_arg = inter::GetNativeArg(s, args, n, 1);
    CVar iv_arg = inter::GetNativeArg(s, args, n, 2);
    uint8_t key[16], iv[16];
    ReadKeyArg(s, key_arg, key, "key");
    ReadKeyArg(s, iv_arg, iv, "iv");

    auto out = AesEncryptCbc(reinterpret_cast<const uint8_t *>(data.data()), data.size(), key, AesKeySize::AES_128, iv);
    return inter::NativeToFakeluaString(s, std::string(out.begin(), out.end()));
}

// crypto.aes_decrypt_cbc(data, key, iv) → decrypted data
static CVar CryptoAesDecryptCbc(State *s, CVar *args, int n) {
    if (n < 3) ThrowBadArgument(1, "crypto.aes_decrypt_cbc", "data, key, and iv expected");
    std::string data = ReadDataArg(s, inter::GetNativeArg(s, args, n, 0));
    CVar key_arg = inter::GetNativeArg(s, args, n, 1);
    CVar iv_arg = inter::GetNativeArg(s, args, n, 2);
    uint8_t key[16], iv[16];
    ReadKeyArg(s, key_arg, key, "key");
    ReadKeyArg(s, iv_arg, iv, "iv");

    auto out = AesDecryptCbc(reinterpret_cast<const uint8_t *>(data.data()), data.size(), key, AesKeySize::AES_128, iv);
    return inter::NativeToFakeluaString(s, std::string(out.begin(), out.end()));
}

// crypto.aes_encrypt_ctr(data, key, iv) → encrypted data (no padding)
static CVar CryptoAesEncryptCtr(State *s, CVar *args, int n) {
    if (n < 3) ThrowBadArgument(1, "crypto.aes_encrypt_ctr", "data, key, and iv expected");
    std::string data = ReadDataArg(s, inter::GetNativeArg(s, args, n, 0));
    CVar key_arg = inter::GetNativeArg(s, args, n, 1);
    CVar iv_arg = inter::GetNativeArg(s, args, n, 2);
    uint8_t key[16], iv[16];
    ReadKeyArg(s, key_arg, key, "key");
    ReadKeyArg(s, iv_arg, iv, "iv");

    auto out = AesEncryptCtr(reinterpret_cast<const uint8_t *>(data.data()), data.size(), key, AesKeySize::AES_128, iv);
    return inter::NativeToFakeluaString(s, std::string(out.begin(), out.end()));
}

// crypto.aes_decrypt_ctr(data, key, iv) → decrypted data
static CVar CryptoAesDecryptCtr(State *s, CVar *args, int n) {
    if (n < 3) ThrowBadArgument(1, "crypto.aes_decrypt_ctr", "data, key, and iv expected");
    std::string data = ReadDataArg(s, inter::GetNativeArg(s, args, n, 0));
    CVar key_arg = inter::GetNativeArg(s, args, n, 1);
    CVar iv_arg = inter::GetNativeArg(s, args, n, 2);
    uint8_t key[16], iv[16];
    ReadKeyArg(s, key_arg, key, "key");
    ReadKeyArg(s, iv_arg, iv, "iv");

    auto out = AesDecryptCtr(reinterpret_cast<const uint8_t *>(data.data()), data.size(), key, AesKeySize::AES_128, iv);
    return inter::NativeToFakeluaString(s, std::string(out.begin(), out.end()));
}

// crypto.base64_encode(data) → base64 string
static CVar CryptoBase64Encode(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "crypto.base64_encode", "data expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    std::string data = inter::FakeluaToNativeString(s, a0);
    std::string out = Base64Encode(reinterpret_cast<const uint8_t *>(data.data()), data.size());
    return inter::NativeToFakeluaString(s, out);
}

// crypto.base64_decode(data) → binary data
static CVar CryptoBase64Decode(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "crypto.base64_decode", "data expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    std::string data = inter::FakeluaToNativeString(s, a0);
    std::string out = Base64Decode(reinterpret_cast<const uint8_t *>(data.data()), data.size());
    return inter::NativeToFakeluaString(s, out);
}

// crypto.uuid() → RFC 4122 v4 UUID string (xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx)
static CVar CryptoUuid(State *s, CVar * /*args*/, int /*n*/) {
    boost::uuids::random_generator gen;
    return inter::NativeToFakeluaString(s, boost::uuids::to_string(gen()));
}

// crypto.crc32(data) → CRC-32/ISO-HDLC (PKZIP) as unsigned 32-bit integer
static CVar CryptoCrc32(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "crypto.crc32", "data expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    std::string data = inter::FakeluaToNativeString(s, a0);
    boost::crc_32_type crc;
    crc.process_bytes(data.data(), data.size());
    return inter::NativeToFakeluaLonglong(s, static_cast<long long>(crc.checksum()));
}

// crypto.xxhash(data) → xxHash-64 as 16-char lowercase hex (Boost.Hash2, not a password hash)
static CVar CryptoXxhash(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "crypto.xxhash", "data expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    std::string data = inter::FakeluaToNativeString(s, a0);
    boost::hash2::xxhash_64 h;
    if (!data.empty()) {
        h.update(data.data(), data.size());
    }
    std::uint64_t v = h.result();
    uint8_t bytes[8];
    for (int i = 7; i >= 0; --i) {
        bytes[i] = static_cast<uint8_t>(v & 0xffu);
        v >>= 8;
    }
    return inter::NativeToFakeluaString(s, ToHex(bytes, 8));
}

// crypto.hex_encode(data) → hex string
static CVar CryptoHexEncode(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "crypto.hex_encode", "data expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    std::string data = inter::FakeluaToNativeString(s, a0);
    return inter::NativeToFakeluaString(s, ToHex(reinterpret_cast<const uint8_t *>(data.data()), data.size()));
}

// crypto.hex_decode(hex) → binary data
static CVar CryptoHexDecode(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "crypto.hex_decode", "hex string expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    std::string hex = inter::FakeluaToNativeString(s, a0);
    if (hex.size() % 2 != 0) {
        ThrowFakeluaException("crypto.hex_decode: hex string must have even length");
    }
    try {
        std::string out;
        out.reserve(hex.size() / 2);
        boost::algorithm::unhex(hex.begin(), hex.end(), std::back_inserter(out));
        return inter::NativeToFakeluaString(s, out);
    } catch (const boost::algorithm::hex_decode_error &) {
        ThrowFakeluaException("crypto.hex_decode: invalid hex character");
    }
}

// crypto.rc4(key, data) → encrypted/decrypted data (RC4 is symmetric)
static CVar CryptoRc4(State *s, CVar *args, int n) {
    if (n < 2) ThrowBadArgument(1, "crypto.rc4", "key and data expected");
    CVar key_arg = inter::GetNativeArg(s, args, n, 0);
    CVar data_arg = inter::GetNativeArg(s, args, n, 1);
    std::string key = inter::FakeluaToNativeString(s, key_arg);
    std::string data = inter::FakeluaToNativeString(s, data_arg);
    auto out = Rc4(key, data);
    return inter::NativeToFakeluaString(s, std::string(out.begin(), out.end()));
}

// crypto.blowfish_encrypt(key, data) → encrypted data (ECB, zero-padded)
static CVar CryptoBlowfishEncrypt(State *s, CVar *args, int n) {
    if (n < 2) ThrowBadArgument(1, "crypto.blowfish_encrypt", "key and data expected");
    CVar key_arg = inter::GetNativeArg(s, args, n, 0);
    CVar data_arg = inter::GetNativeArg(s, args, n, 1);
    std::string key = inter::FakeluaToNativeString(s, key_arg);
    std::string data = inter::FakeluaToNativeString(s, data_arg);
    auto out = BlowfishEncrypt(reinterpret_cast<const uint8_t *>(key.data()), key.size(), reinterpret_cast<const uint8_t *>(data.data()), data.size());
    return inter::NativeToFakeluaString(s, std::string(out.begin(), out.end()));
}

// crypto.blowfish_decrypt(key, data) → decrypted data
static CVar CryptoBlowfishDecrypt(State *s, CVar *args, int n) {
    if (n < 2) ThrowBadArgument(1, "crypto.blowfish_decrypt", "key and data expected");
    CVar key_arg = inter::GetNativeArg(s, args, n, 0);
    CVar data_arg = inter::GetNativeArg(s, args, n, 1);
    std::string key = inter::FakeluaToNativeString(s, key_arg);
    std::string data = inter::FakeluaToNativeString(s, data_arg);
    auto out = BlowfishDecrypt(reinterpret_cast<const uint8_t *>(key.data()), key.size(), reinterpret_cast<const uint8_t *>(data.data()), data.size());
    return inter::NativeToFakeluaString(s, std::string(out.begin(), out.end()));
}

// crypto.des_encrypt(key, data) → encrypted data (ECB, zero-padded, key >= 8 bytes)
static CVar CryptoDesEncrypt(State *s, CVar *args, int n) {
    if (n < 2) ThrowBadArgument(1, "crypto.des_encrypt", "key and data expected");
    CVar key_arg = inter::GetNativeArg(s, args, n, 0);
    CVar data_arg = inter::GetNativeArg(s, args, n, 1);
    std::string key = inter::FakeluaToNativeString(s, key_arg);
    std::string data = inter::FakeluaToNativeString(s, data_arg);
    auto out = DesEncrypt(reinterpret_cast<const uint8_t *>(key.data()), key.size(), reinterpret_cast<const uint8_t *>(data.data()), data.size());
    return inter::NativeToFakeluaString(s, std::string(out.begin(), out.end()));
}

// crypto.des_decrypt(key, data) → decrypted data
static CVar CryptoDesDecrypt(State *s, CVar *args, int n) {
    if (n < 2) ThrowBadArgument(1, "crypto.des_decrypt", "key and data expected");
    CVar key_arg = inter::GetNativeArg(s, args, n, 0);
    CVar data_arg = inter::GetNativeArg(s, args, n, 1);
    std::string key = inter::FakeluaToNativeString(s, key_arg);
    std::string data = inter::FakeluaToNativeString(s, data_arg);
    auto out = DesDecrypt(reinterpret_cast<const uint8_t *>(key.data()), key.size(), reinterpret_cast<const uint8_t *>(data.data()), data.size());
    return inter::NativeToFakeluaString(s, std::string(out.begin(), out.end()));
}

// crypto.triple_des_encrypt(key, data) → encrypted data (key >= 24 bytes)
static CVar CryptoTripleDesEncrypt(State *s, CVar *args, int n) {
    if (n < 2) ThrowBadArgument(1, "crypto.triple_des_encrypt", "key and data expected");
    CVar key_arg = inter::GetNativeArg(s, args, n, 0);
    CVar data_arg = inter::GetNativeArg(s, args, n, 1);
    std::string key = inter::FakeluaToNativeString(s, key_arg);
    std::string data = inter::FakeluaToNativeString(s, data_arg);
    auto out = TripleDesEncrypt(reinterpret_cast<const uint8_t *>(key.data()), key.size(), reinterpret_cast<const uint8_t *>(data.data()), data.size());
    return inter::NativeToFakeluaString(s, std::string(out.begin(), out.end()));
}

// crypto.triple_des_decrypt(key, data) → decrypted data
static CVar CryptoTripleDesDecrypt(State *s, CVar *args, int n) {
    if (n < 2) ThrowBadArgument(1, "crypto.triple_des_decrypt", "key and data expected");
    CVar key_arg = inter::GetNativeArg(s, args, n, 0);
    CVar data_arg = inter::GetNativeArg(s, args, n, 1);
    std::string key = inter::FakeluaToNativeString(s, key_arg);
    std::string data = inter::FakeluaToNativeString(s, data_arg);
    auto out = TripleDesDecrypt(reinterpret_cast<const uint8_t *>(key.data()), key.size(), reinterpret_cast<const uint8_t *>(data.data()), data.size());
    return inter::NativeToFakeluaString(s, std::string(out.begin(), out.end()));
}

// =============================================================================
// Generic EVP interface — exposes ALL OpenSSL algorithms to Lua
// =============================================================================
// crypto.digest(algo, data)           → hex string
//   algo: any OpenSSL digest name, e.g. "md5", "sha1", "sha256", "sha512",
//         "sha3-256", "blake2b512", "sm3", etc.
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
// Note on AEAD modes (GCM, CCM, OCB):
//   These require additional tag/aad handling not yet exposed here. For now,
//   only non-AEAD ciphers are reliably supported.
// =============================================================================

// Internal: run an EVP cipher operation (encrypt or decrypt)
static std::vector<uint8_t> EvpCipherOp(const std::string &algo, const std::string &key, const std::string &iv, const uint8_t *data, size_t data_len, bool encrypt, bool no_padding) {

    const EVP_CIPHER *cipher = EVP_get_cipherbyname(algo.c_str());
    if (!cipher) {
        ThrowFakeluaException(std::format("crypto.{}: unknown cipher '{}' — use an OpenSSL cipher name such as "
                                          "'aes-128-cbc', 'aes-256-ecb', 'chacha20', 'bf-ecb', 'rc4', 'des-ede3-ecb'",
                                          encrypt ? "encrypt" : "decrypt", algo));
    }

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        ThrowFakeluaException(std::format("crypto.{}: failed to create EVP_CIPHER_CTX", encrypt ? "encrypt" : "decrypt"));
    }

    // Phase 1: init with cipher only (no key/iv yet) so we can adjust key length
    auto init_fn = encrypt ? EVP_EncryptInit_ex : EVP_DecryptInit_ex;
    if (init_fn(ctx, cipher, nullptr, nullptr, nullptr) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException(std::format("crypto.{}: EVP init failed for cipher '{}'", encrypt ? "encrypt" : "decrypt", algo));
    }

    // For variable-key-length ciphers (RC4, Blowfish, ChaCha20, ...),
    // set the actual key length before providing the key bytes.
    int expected_key_len = EVP_CIPHER_CTX_key_length(ctx);
    if (static_cast<int>(key.size()) != expected_key_len) {
        if (EVP_CIPHER_CTX_set_key_length(ctx, static_cast<int>(key.size())) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            ThrowFakeluaException(std::format("crypto.{}: key length {} is not valid for cipher '{}' (expected {} bytes)", encrypt ? "encrypt" : "decrypt", key.size(), algo, expected_key_len));
        }
    }

    // Validate IV length
    int cipher_iv_len = EVP_CIPHER_CTX_iv_length(ctx);
    const uint8_t *iv_ptr = nullptr;
    if (cipher_iv_len > 0) {
        if (static_cast<int>(iv.size()) != cipher_iv_len) {
            EVP_CIPHER_CTX_free(ctx);
            ThrowFakeluaException(std::format("crypto.{}: IV length {} is not valid for cipher '{}' (expected {} bytes)", encrypt ? "encrypt" : "decrypt", iv.size(), algo, cipher_iv_len));
        }
        iv_ptr = reinterpret_cast<const uint8_t *>(iv.data());
    } else if (!iv.empty()) {
        // Cipher doesn't use an IV but one was provided — warn-and-ignore is safer
        // than error, since callers often pass a dummy IV regardless.
    }

    // Phase 2: provide key and IV
    if (init_fn(ctx, nullptr, nullptr, reinterpret_cast<const uint8_t *>(key.data()), iv_ptr) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException(std::format("crypto.{}: failed to set key/IV for cipher '{}'", encrypt ? "encrypt" : "decrypt", algo));
    }

    // Configure padding
    if (EVP_CIPHER_CTX_set_padding(ctx, no_padding ? 0 : 1) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException(std::format("crypto.{}: EVP_CIPHER_CTX_set_padding failed", encrypt ? "encrypt" : "decrypt"));
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
            ThrowFakeluaException(std::format("crypto.{}: EVP_Update failed for cipher '{}'", encrypt ? "encrypt" : "decrypt", algo));
        }
    }

    int final_len = 0;
    auto final_fn = encrypt ? EVP_EncryptFinal_ex : EVP_DecryptFinal_ex;
    if (final_fn(ctx, out.data() + outlen, &final_len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ThrowFakeluaException(std::format("crypto.{}: EVP_Final failed for cipher '{}' — "
                                          "check key/IV length, padding setting, or data integrity",
                                          encrypt ? "encrypt" : "decrypt", algo));
    }

    out.resize(static_cast<size_t>(outlen) + static_cast<size_t>(final_len));
    EVP_CIPHER_CTX_free(ctx);
    return out;
}

// crypto.digest(algo, data) → hex string
static CVar CryptoDigest(State *s, CVar *args, int n) {
    if (n < 2) ThrowBadArgument(1, "crypto.digest", "algo and data expected");
    std::string algo = inter::FakeluaToNativeString(s, inter::GetNativeArg(s, args, n, 0));
    std::string data = inter::FakeluaToNativeString(s, inter::GetNativeArg(s, args, n, 1));

    const EVP_MD *md = EVP_get_digestbyname(algo.c_str());
    if (!md) {
        ThrowFakeluaException(std::format("crypto.digest: unknown digest algorithm '{}' — use an OpenSSL digest name "
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

    return inter::NativeToFakeluaString(s, ToHex(digest_buf, digest_len));
}

// crypto.digest_raw(algo, data) → raw binary bytes (not hex-encoded)
// Useful when the digest output needs to be used as input to another operation.
static CVar CryptoDigestRaw(State *s, CVar *args, int n) {
    if (n < 2) ThrowBadArgument(1, "crypto.digest_raw", "algo and data expected");
    std::string algo = inter::FakeluaToNativeString(s, inter::GetNativeArg(s, args, n, 0));
    std::string data = inter::FakeluaToNativeString(s, inter::GetNativeArg(s, args, n, 1));

    const EVP_MD *md = EVP_get_digestbyname(algo.c_str());
    if (!md) {
        ThrowFakeluaException(std::format("crypto.digest_raw: unknown digest algorithm '{}'", algo));
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

// crypto.encrypt(algo, key, iv, data [, no_padding]) → binary string
static CVar CryptoEncryptEvp(State *s, CVar *args, int n) {
    if (n < 4) ThrowBadArgument(1, "crypto.encrypt", "algo, key, iv, data expected");
    std::string algo = inter::FakeluaToNativeString(s, inter::GetNativeArg(s, args, n, 0));
    std::string key = inter::FakeluaToNativeString(s, inter::GetNativeArg(s, args, n, 1));
    std::string iv = inter::FakeluaToNativeString(s, inter::GetNativeArg(s, args, n, 2));
    std::string data = inter::FakeluaToNativeString(s, inter::GetNativeArg(s, args, n, 3));
    bool no_padding = false;
    if (n >= 5) {
        no_padding = inter::FakeluaToNativeBool(s, inter::GetNativeArg(s, args, n, 4));
    }

    auto out = EvpCipherOp(algo, key, iv, reinterpret_cast<const uint8_t *>(data.data()), data.size(), true, no_padding);
    return inter::NativeToFakeluaString(s, std::string(out.begin(), out.end()));
}

// crypto.decrypt(algo, key, iv, data [, no_padding]) → binary string
static CVar CryptoDecryptEvp(State *s, CVar *args, int n) {
    if (n < 4) ThrowBadArgument(1, "crypto.decrypt", "algo, key, iv, data expected");
    std::string algo = inter::FakeluaToNativeString(s, inter::GetNativeArg(s, args, n, 0));
    std::string key = inter::FakeluaToNativeString(s, inter::GetNativeArg(s, args, n, 1));
    std::string iv = inter::FakeluaToNativeString(s, inter::GetNativeArg(s, args, n, 2));
    std::string data = inter::FakeluaToNativeString(s, inter::GetNativeArg(s, args, n, 3));
    bool no_padding = false;
    if (n >= 5) {
        no_padding = inter::FakeluaToNativeBool(s, inter::GetNativeArg(s, args, n, 4));
    }

    auto out = EvpCipherOp(algo, key, iv, reinterpret_cast<const uint8_t *>(data.data()), data.size(), false, no_padding);
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

    RegisterNativeFunction(s, "crypto.md5", 1, false, CryptoMd5);
    RegisterNativeFunction(s, "crypto.sha1", 1, false, CryptoSha1);
    RegisterNativeFunction(s, "crypto.sha256", 1, false, CryptoSha256);
    RegisterNativeFunction(s, "crypto.hex_encode", 1, false, CryptoHexEncode);
    RegisterNativeFunction(s, "crypto.hex_decode", 1, false, CryptoHexDecode);
    RegisterNativeFunction(s, "crypto.base64_encode", 1, false, CryptoBase64Encode);
    RegisterNativeFunction(s, "crypto.base64_decode", 1, false, CryptoBase64Decode);
    RegisterNativeFunction(s, "crypto.uuid", 0, false, CryptoUuid);
    RegisterNativeFunction(s, "crypto.crc32", 1, false, CryptoCrc32);
    RegisterNativeFunction(s, "crypto.xxhash", 1, false, CryptoXxhash);
    RegisterNativeFunction(s, "crypto.aes_encrypt_ecb", 2, false, CryptoAesEncryptEcb);
    RegisterNativeFunction(s, "crypto.aes_decrypt_ecb", 2, false, CryptoAesDecryptEcb);
    RegisterNativeFunction(s, "crypto.aes_encrypt_cbc", 3, false, CryptoAesEncryptCbc);
    RegisterNativeFunction(s, "crypto.aes_decrypt_cbc", 3, false, CryptoAesDecryptCbc);
    RegisterNativeFunction(s, "crypto.aes_encrypt_ctr", 3, false, CryptoAesEncryptCtr);
    RegisterNativeFunction(s, "crypto.aes_decrypt_ctr", 3, false, CryptoAesDecryptCtr);
    RegisterNativeFunction(s, "crypto.rc4", 2, false, CryptoRc4);
    RegisterNativeFunction(s, "crypto.blowfish_encrypt", 2, false, CryptoBlowfishEncrypt);
    RegisterNativeFunction(s, "crypto.blowfish_decrypt", 2, false, CryptoBlowfishDecrypt);
    RegisterNativeFunction(s, "crypto.des_encrypt", 2, false, CryptoDesEncrypt);
    RegisterNativeFunction(s, "crypto.des_decrypt", 2, false, CryptoDesDecrypt);
    RegisterNativeFunction(s, "crypto.triple_des_encrypt", 2, false, CryptoTripleDesEncrypt);
    RegisterNativeFunction(s, "crypto.triple_des_decrypt", 2, false, CryptoTripleDesDecrypt);
    // Generic EVP interface
    RegisterNativeFunction(s, "crypto.digest", 2, false, CryptoDigest);
    RegisterNativeFunction(s, "crypto.digest_raw", 2, false, CryptoDigestRaw);
    RegisterNativeFunction(s, "crypto.encrypt", 4, true, CryptoEncryptEvp);
    RegisterNativeFunction(s, "crypto.decrypt", 4, true, CryptoDecryptEvp);
}


}// namespace fakelua::crypto
