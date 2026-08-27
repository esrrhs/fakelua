#include "native/crypto/native_crypto.h"
#include "native/crypto/hash.h"
#include "native/crypto/aes.h"
#include "native/native_common.h"
#include "util/logging.h"

#include <string>

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

// Read an 8-byte key argument
static void read_8byte_key_arg(State *s, CVar arg, uint8_t out[8], const char *name) {
    std::string data = inter::FakeluaToNativeString(s, arg);
    if (data.size() < 8) {
        ThrowFakeluaException(std::format("{} must be at least 8 bytes", name));
    }
    memcpy(out, data.data(), 8);
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

void RegisterCryptoLibraryApi(State *s) {
    if (!s) return;
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
}

}  // namespace fakelua::crypto
