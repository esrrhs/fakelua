#include "fakelua.h"
#include "gtest/gtest.h"

using namespace fakelua;

// ─────────────────────────────────────────────────────────────────────────────
// crypto 模块测试 — 哈希
// ─────────────────────────────────────────────────────────────────────────────

TEST(test_crypto, md5_empty) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_crypto_md5.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CryptoTest.test_md5", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_crypto, sha1_empty) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_crypto_sha1.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CryptoTest.test_sha1", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_crypto, sha256_empty) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_crypto_sha256.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CryptoTest.test_sha256", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_crypto, md5_hello) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_crypto_md5_hello.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CryptoTest.test_md5_hello", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_crypto, sha1_hello) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_crypto_sha1_hello.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CryptoTest.test_sha1_hello", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// ─────────────────────────────────────────────────────────────────────────────
// crypto 模块测试 — Base64
// ─────────────────────────────────────────────────────────────────────────────

TEST(test_crypto, base64_encode_decode) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_base64.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CryptoTest.test_base64", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_crypto, base64_whitespace) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_base64.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CryptoTest.test_base64_whitespace", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_crypto, base64_invalid) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_base64.lua", config);
    int64_t ret = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "CryptoTest.test_base64_invalid", ret), std::exception);
    FakeluaDeleteState(s);
}

// ─────────────────────────────────────────────────────────────────────────────
// crypto 模块测试 — AES-128
// ─────────────────────────────────────────────────────────────────────────────

TEST(test_crypto, aes_ecb_encrypt) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_aes_ecb.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CryptoTest.test_ecb", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_crypto, aes_cbc_encrypt) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_aes_cbc.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CryptoTest.test_cbc", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_crypto, aes_ctr_encrypt) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_aes_ctr.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CryptoTest.test_ctr", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// ─────────────────────────────────────────────────────────────────────────────
// crypto 模块测试 — RC4
// ─────────────────────────────────────────────────────────────────────────────

TEST(test_crypto, rc4_keystream) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_rc4.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CryptoTest.test_rc4_keystream", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_crypto, rc4_encrypt_decrypt) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_rc4.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CryptoTest.test_rc4_encrypt_decrypt", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// ─────────────────────────────────────────────────────────────────────────────
// crypto 模块测试 — Blowfish
// ─────────────────────────────────────────────────────────────────────────────

TEST(test_crypto, blowfish_encrypt_decrypt) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_blowfish.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CryptoTest.test_blowfish", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// ─────────────────────────────────────────────────────────────────────────────
// crypto 模块测试 — DES / 3DES
// ─────────────────────────────────────────────────────────────────────────────

TEST(test_crypto, des_encrypt_decrypt) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_des.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CryptoTest.test_des", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_crypto, triple_des_encrypt_decrypt) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_des.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CryptoTest.test_triple_des", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_crypto, hex_roundtrip) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_crypto_hex.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CryptoTest.test_hex_roundtrip", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_crypto, hex_decode_invalid) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_crypto_hex.lua", config);
    int64_t ret = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "CryptoTest.test_hex_decode_invalid", ret), std::exception);
    FakeluaDeleteState(s);
}
