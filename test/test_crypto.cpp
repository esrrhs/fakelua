#include "fakelua.h"
#include "jit/vm.h"
#include "native/crypto/crypto_aes.h"
#include "native/crypto/crypto_cipher.h"
#include "native/crypto/crypto_digest.h"
#include "test_jit.h"
#include "gtest/gtest.h"
#include <array>
#include <memory>
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/evp.h>

using namespace fakelua;

// crypto 模块测试 — 哈希

TEST(test_crypto, md5_empty) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_crypto_md5.lua", config);
    int64_t ret = 0;
    CallAll(s, "CryptoTest.test_md5", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_crypto, sha1_empty) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_crypto_sha1.lua", config);
    int64_t ret = 0;
    CallAll(s, "CryptoTest.test_sha1", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_crypto, sha256_empty) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_crypto_sha256.lua", config);
    int64_t ret = 0;
    CallAll(s, "CryptoTest.test_sha256", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_crypto, md5_hello) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_crypto_md5_hello.lua", config);
    int64_t ret = 0;
    CallAll(s, "CryptoTest.test_md5_hello", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_crypto, sha1_hello) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_crypto_sha1_hello.lua", config);
    int64_t ret = 0;
    CallAll(s, "CryptoTest.test_sha1_hello", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// crypto 模块测试 — Base64

TEST(test_crypto, base64_encode_decode) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_base64.lua", config);
    int64_t ret = 0;
    CallAll(s, "CryptoTest.test_base64", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_crypto, base64_whitespace) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_base64.lua", config);
    int64_t ret = 0;
    CallAll(s, "CryptoTest.test_base64_whitespace", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_crypto, base64_invalid) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_base64.lua", config);
    int64_t ret = 0;
    CallThrow(s, "CryptoTest.test_base64_invalid", ret);
    FakeluaDeleteState(s);
}

// crypto 模块测试 — AES-128

TEST(test_crypto, aes_ecb_encrypt) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_aes_ecb.lua", config);
    int64_t ret = 0;
    CallAll(s, "CryptoTest.test_ecb", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_crypto, aes_cbc_encrypt) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_aes_cbc.lua", config);
    int64_t ret = 0;
    CallAll(s, "CryptoTest.test_cbc", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_crypto, aes_ctr_encrypt) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_aes_ctr.lua", config);
    int64_t ret = 0;
    CallAll(s, "CryptoTest.test_ctr", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// crypto 模块测试 — RC4

TEST(test_crypto, rc4_keystream) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_rc4.lua", config);
    int64_t ret = 0;
    CallAll(s, "CryptoTest.test_rc4_keystream", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_crypto, rc4_encrypt_decrypt) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_rc4.lua", config);
    int64_t ret = 0;
    CallAll(s, "CryptoTest.test_rc4_encrypt_decrypt", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// crypto 模块测试 — Blowfish

TEST(test_crypto, blowfish_encrypt_decrypt) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_blowfish.lua", config);
    int64_t ret = 0;
    CallAll(s, "CryptoTest.test_blowfish", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// crypto 模块测试 — DES / 3DES

TEST(test_crypto, des_encrypt_decrypt) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_des.lua", config);
    int64_t ret = 0;
    CallAll(s, "CryptoTest.test_des", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_crypto, triple_des_encrypt_decrypt) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_des.lua", config);
    int64_t ret = 0;
    CallAll(s, "CryptoTest.test_triple_des", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_crypto, hex_roundtrip) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_crypto_hex.lua", config);
    int64_t ret = 0;
    CallAll(s, "CryptoTest.test_hex_roundtrip", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_crypto, hex_decode_invalid) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_crypto_hex.lua", config);
    int64_t ret = 0;
    CallThrow(s, "CryptoTest.test_hex_decode_invalid", ret);
    FakeluaDeleteState(s);
}

// Generic EVP interface tests
static void RunEvpTest(const char *func_name) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_evp.lua", config);
    int64_t ret = 0;
    CallAll(s, func_name, ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_crypto, evp_digest_md5) {
    RunEvpTest("CryptoTest.test_evp_digest_md5");
}

TEST(test_crypto, evp_digest_sha1) {
    RunEvpTest("CryptoTest.test_evp_digest_sha1");
}

TEST(test_crypto, evp_digest_sha256) {
    RunEvpTest("CryptoTest.test_evp_digest_sha256");
}

TEST(test_crypto, evp_digest_sha512) {
    RunEvpTest("CryptoTest.test_evp_digest_sha512");
}

TEST(test_crypto, evp_digest_raw) {
    RunEvpTest("CryptoTest.test_evp_digest_raw");
}

TEST(test_crypto, evp_digest_unknown) {
    RunEvpTest("CryptoTest.test_evp_digest_unknown");
}

TEST(test_crypto, evp_aes128_cbc) {
    RunEvpTest("CryptoTest.test_evp_aes128_cbc");
}

TEST(test_crypto, evp_aes256_cbc) {
    RunEvpTest("CryptoTest.test_evp_aes256_cbc");
}

TEST(test_crypto, evp_aes128_ecb) {
    RunEvpTest("CryptoTest.test_evp_aes128_ecb");
}

TEST(test_crypto, evp_rc4) {
    RunEvpTest("CryptoTest.test_evp_rc4");
}

TEST(test_crypto, evp_blowfish) {
    RunEvpTest("CryptoTest.test_evp_blowfish");
}

TEST(test_crypto, evp_des) {
    RunEvpTest("CryptoTest.test_evp_des");
}

TEST(test_crypto, evp_3des) {
    RunEvpTest("CryptoTest.test_evp_3des");
}

TEST(test_crypto, evp_unknown_cipher) {
    RunEvpTest("CryptoTest.test_evp_unknown_cipher");
}

TEST(test_crypto, evp_wrong_key) {
    RunEvpTest("CryptoTest.test_evp_wrong_key_length");
}

TEST(test_crypto, evp_wrong_iv) {
    RunEvpTest("CryptoTest.test_evp_wrong_iv_length");
}

// Extended EVP tests — algorithms newly accessible via generic interface
static void RunEvpExtTest(const char *func_name) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_evp_extended.lua", config);
    int64_t ret = 0;
    CallAll(s, func_name, ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// Digests
TEST(test_crypto, evp_sha224) {
    RunEvpExtTest("CryptoTest.test_evp_sha224");
}

TEST(test_crypto, evp_sha384) {
    RunEvpExtTest("CryptoTest.test_evp_sha384");
}

TEST(test_crypto, evp_sha3_256) {
    RunEvpExtTest("CryptoTest.test_evp_sha3_256");
}

TEST(test_crypto, evp_sha3_512) {
    RunEvpExtTest("CryptoTest.test_evp_sha3_512");
}

TEST(test_crypto, evp_blake2b512) {
    RunEvpExtTest("CryptoTest.test_evp_blake2b512");
}

TEST(test_crypto, evp_blake2s256) {
    RunEvpExtTest("CryptoTest.test_evp_blake2s256");
}

TEST(test_crypto, evp_ripemd160) {
    RunEvpExtTest("CryptoTest.test_evp_ripemd160");
}

// AES-192 (was dead code before)
TEST(test_crypto, evp_aes192_cbc) {
    RunEvpExtTest("CryptoTest.test_evp_aes192_cbc");
}

TEST(test_crypto, evp_aes192_ecb) {
    RunEvpExtTest("CryptoTest.test_evp_aes192_ecb");
}

// AES modes
TEST(test_crypto, evp_aes128_cfb) {
    RunEvpExtTest("CryptoTest.test_evp_aes128_cfb");
}

TEST(test_crypto, evp_aes128_ofb) {
    RunEvpExtTest("CryptoTest.test_evp_aes128_ofb");
}

TEST(test_crypto, evp_aes128_ctr) {
    RunEvpExtTest("CryptoTest.test_evp_aes128_ctr");
}

TEST(test_crypto, evp_aes256_ctr) {
    RunEvpExtTest("CryptoTest.test_evp_aes256_ctr");
}

TEST(test_crypto, evp_aes256_ecb) {
    RunEvpExtTest("CryptoTest.test_evp_aes256_ecb");
}

// ChaCha20
TEST(test_crypto, evp_chacha20) {
    RunEvpExtTest("CryptoTest.test_evp_chacha20");
}

// Camellia
TEST(test_crypto, evp_camellia128) {
    RunEvpExtTest("CryptoTest.test_evp_camellia128_cbc");
}

TEST(test_crypto, evp_camellia256) {
    RunEvpExtTest("CryptoTest.test_evp_camellia256_cbc");
}

// DES-CBC / 3DES-CBC
TEST(test_crypto, evp_des_cbc) {
    RunEvpExtTest("CryptoTest.test_evp_des_cbc");
}

TEST(test_crypto, evp_3des_cbc) {
    RunEvpExtTest("CryptoTest.test_evp_3des_cbc");
}

// ARIA
TEST(test_crypto, evp_aria128_cbc) {
    RunEvpExtTest("CryptoTest.test_evp_aria128_cbc");
}

// digest_raw
TEST(test_crypto, evp_digest_raw_sha256) {
    RunEvpExtTest("CryptoTest.test_evp_digest_raw_sha256");
}

TEST(test_crypto, evp_digest_raw_sha3_256) {
    RunEvpExtTest("CryptoTest.test_evp_digest_raw_sha3_256");
}

TEST(test_crypto, uuid_v4) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_crypto_uuid.lua", config);
    int64_t ret = 0;
    CallAll(s, "CryptoTest.test_uuid", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_crypto, crc32) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_crypto_crc32.lua", config);
    int64_t ret = 0;
    CallAll(s, "CryptoTest.test_crc32", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_crypto, xxhash) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_crypto_xxhash.lua", config);
    int64_t ret = 0;
    CallAll(s, "CryptoTest.test_xxhash", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_crypto, aes_cbc_rejects_truncation_and_corrupted_padding) {
    std::array<uint8_t, 16> input{};
    std::array<uint8_t, 16> key{};
    std::array<uint8_t, 16> iv{};
    auto encrypted = crypto::AesEncryptCbc(input.data(), input.size(), key.data(), crypto::AesKeySize::AES_128, iv.data());
    ASSERT_EQ(encrypted.size(), 32);
    EXPECT_EQ(crypto::AesDecryptCbc(encrypted.data(), encrypted.size(), key.data(), crypto::AesKeySize::AES_128, iv.data()), std::vector<uint8_t>(input.begin(), input.end()));
    EXPECT_THROW(crypto::AesDecryptCbc(encrypted.data(), 0, key.data(), crypto::AesKeySize::AES_128, iv.data()), std::exception);
    EXPECT_THROW(crypto::AesDecryptCbc(encrypted.data(), encrypted.size() - 1, key.data(), crypto::AesKeySize::AES_128, iv.data()), std::exception);
    // CBC XORs the preceding ciphertext block into the plaintext. Flipping its
    // last byte changes only the last padding byte from 16 to 17, deterministically.
    encrypted[15] ^= 1;
    try {
        (void) crypto::AesDecryptCbc(encrypted.data(), encrypted.size(), key.data(), crypto::AesKeySize::AES_128, iv.data());
        FAIL() << "corrupted PKCS#7 padding was accepted";
    } catch (const std::exception &e) {
        EXPECT_NE(std::string(e.what()).find("bad padding"), std::string::npos);
    }
    ERR_clear_error();
}

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
TEST(test_crypto, unavailable_provider_is_reported_and_context_is_restored) {
    FakeluaStateGuard state;
    std::array<uint8_t, 32> key{};
    std::array<uint8_t, 16> input{};
    std::array<uint8_t, 16> output{};
    std::array<uint8_t, 16> iv{};
    {
        std::unique_ptr<OSSL_LIB_CTX, decltype(&OSSL_LIB_CTX_free)> context(OSSL_LIB_CTX_new(), OSSL_LIB_CTX_free);
        ASSERT_NE(context, nullptr);
        ASSERT_EQ(EVP_set_default_properties(context.get(), "provider=fakelua_unavailable"), 1);

        struct RestoreThreadContext {
            OSSL_LIB_CTX *previous;

            ~RestoreThreadContext() {
                OSSL_LIB_CTX_set0_default(previous);
                ERR_clear_error();
            }
        } restore{OSSL_LIB_CTX_set0_default(context.get())};

        // Real provider selection failure, isolated to this thread/context; no
        // mocks, global provider unloads, or changes to other tests' contexts.
        EXPECT_THROW(crypto::AesEncryptEcb(input.data(), output.data(), key.data(), crypto::AesKeySize::AES_128), std::exception);
        EXPECT_THROW(crypto::AesDecryptEcb(input.data(), output.data(), key.data(), crypto::AesKeySize::AES_128), std::exception);
        EXPECT_THROW(crypto::AesEncryptCbc(input.data(), input.size(), key.data(), crypto::AesKeySize::AES_128, iv.data()), std::exception);
        EXPECT_THROW(crypto::AesDecryptCbc(input.data(), input.size(), key.data(), crypto::AesKeySize::AES_128, iv.data()), std::exception);
        EXPECT_THROW(crypto::AesEncryptCtr(input.data(), input.size(), key.data(), crypto::AesKeySize::AES_128, iv.data()), std::exception);
        CVar digest_args[] = {inter::NativeToFakeluaString(state.GetState(), "sha256"), inter::NativeToFakeluaString(state.GetState(), "data")};
        EXPECT_THROW(CallByNameArgs(state.GetState(), JIT_INTERP, "crypto.digest", 2, digest_args), std::exception);
        EXPECT_THROW(CallByNameArgs(state.GetState(), JIT_INTERP, "crypto.digest_raw", 2, digest_args), std::exception);
        CVar cipher_args[] = {inter::NativeToFakeluaString(state.GetState(), "aes-128-cbc"), inter::NativeToFakeluaString(state.GetState(), std::string(16, '\0')),
                              inter::NativeToFakeluaString(state.GetState(), std::string(16, '\0')), inter::NativeToFakeluaString(state.GetState(), std::string(16, '\0'))};
        EXPECT_THROW(CallByNameArgs(state.GetState(), JIT_INTERP, "crypto.encrypt", 4, cipher_args), std::exception);
        EXPECT_THROW(crypto::Md5(input.data(), input.size()), std::exception);
        EXPECT_THROW(crypto::Sha1(input.data(), input.size()), std::exception);
        EXPECT_THROW(crypto::Sha256(input.data(), input.size()), std::exception);
        EXPECT_THROW(crypto::Rc4(key.data(), 16, input.data(), input.size()), std::exception);
        EXPECT_THROW(crypto::BlowfishEncrypt(key.data(), 16, input.data(), input.size()), std::exception);
        EXPECT_THROW(crypto::BlowfishDecrypt(key.data(), 16, input.data(), input.size()), std::exception);
        EXPECT_THROW(crypto::DesEncrypt(key.data(), 8, input.data(), input.size()), std::exception);
        EXPECT_THROW(crypto::DesDecrypt(key.data(), 8, input.data(), input.size()), std::exception);
        EXPECT_THROW(crypto::TripleDesEncrypt(key.data(), 24, input.data(), input.size()), std::exception);
        EXPECT_THROW(crypto::TripleDesDecrypt(key.data(), 24, input.data(), input.size()), std::exception);
    }
    // Published AES-128 zero-key/zero-block vector proves the caller's original
    // provider context is usable after all failure cleanups.
    crypto::AesEncryptEcb(input.data(), output.data(), key.data(), crypto::AesKeySize::AES_128);
    const std::array<uint8_t, 16> expected{0x66, 0xe9, 0x4b, 0xd4, 0xef, 0x8a, 0x2c, 0x3b, 0x88, 0x4c, 0xfa, 0x59, 0xca, 0x34, 0x2b, 0x2e};
    EXPECT_EQ(output, expected);
}
#endif

TEST(test_crypto, block_lengths_iv_encoding_and_generic_padding_errors) {
    FakeluaStateGuard state;
    CompileFile(state.GetState(), "./crypto/test_crypto_input_contracts.lua", {});
    for (auto backend: AllJitTypes()) {
        SCOPED_TRACE(JitTypeName(backend));
        int64_t result = 0;
        Call(state.GetState(), backend, "test_crypto_input_contracts", result);
        EXPECT_EQ(result, 1);
    }
}

TEST(test_crypto, aes_192_and_256_known_answers_and_mode_roundtrips) {
    const std::array<uint8_t, 16> plain{};
    const std::array<uint8_t, 32> key{};
    const std::array<uint8_t, 16> iv{};
    const std::array<std::array<uint8_t, 16>, 2> expected{{{0xaa, 0xe0, 0x69, 0x92, 0xac, 0xbf, 0x52, 0xa3, 0xe8, 0xf4, 0xa9, 0x6e, 0xc9, 0x30, 0x0b, 0xd7},
                                                           {0xdc, 0x95, 0xc0, 0x78, 0xa2, 0x40, 0x89, 0x89, 0xad, 0x48, 0xa2, 0x14, 0x92, 0x84, 0x20, 0x87}}};
    size_t index = 0;
    for (auto bits: {crypto::AesKeySize::AES_192, crypto::AesKeySize::AES_256}) {
        std::array<uint8_t, 16> encrypted{}, restored{};
        crypto::AesEncryptEcb(plain.data(), encrypted.data(), key.data(), bits);
        EXPECT_EQ(encrypted, expected[index++]);
        crypto::AesDecryptEcb(encrypted.data(), restored.data(), key.data(), bits);
        EXPECT_EQ(restored, plain);
        std::vector<uint8_t> message(31);
        for (size_t i = 0; i < message.size(); ++i) message[i] = static_cast<uint8_t>(i);
        auto cbc = crypto::AesEncryptCbc(message.data(), message.size(), key.data(), bits, iv.data());
        EXPECT_EQ(crypto::AesDecryptCbc(cbc.data(), cbc.size(), key.data(), bits, iv.data()), message);
        auto ctr = crypto::AesEncryptCtr(message.data(), message.size(), key.data(), bits, iv.data());
        EXPECT_EQ(ctr.size(), message.size());
        EXPECT_EQ(crypto::AesDecryptCtr(ctr.data(), ctr.size(), key.data(), bits, iv.data()), message);
    }
    std::array<uint8_t, 16> out{};
    const auto invalid = static_cast<crypto::AesKeySize>(0);
    EXPECT_THROW(crypto::AesEncryptEcb(plain.data(), out.data(), key.data(), invalid), std::exception);
    EXPECT_THROW(crypto::AesEncryptCbc(plain.data(), plain.size(), key.data(), invalid, iv.data()), std::exception);
    EXPECT_THROW(crypto::AesEncryptCtr(plain.data(), plain.size(), key.data(), invalid, iv.data()), std::exception);
}
