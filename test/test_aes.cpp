#include "fakelua.h"
#include "gtest/gtest.h"

using namespace fakelua;

// ── AES-128 ECB test ──

// Key: "1234567890123456"
// Plaintext: "Hello, World!!!!" (16 bytes)
// Expected ciphertext: 61b80625e3f5b36cfd4cea22045061c6
TEST(test_aes, ecb_encrypt) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_aes_ecb.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CryptoTest.test_ecb", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// ── AES-128 CBC test ──

TEST(test_aes, cbc_encrypt) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_aes_cbc.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CryptoTest.test_cbc", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// ── AES-128 CTR test ──

TEST(test_aes, ctr_encrypt) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_aes_ctr.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CryptoTest.test_ctr", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}
