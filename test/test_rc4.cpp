#include "fakelua.h"
#include "gtest/gtest.h"

using namespace fakelua;

// Helper: run a Lua test function from test/lua/crypto/test_rc4.lua
static int run_rc4_test(State *s, const char *func) {
    CompileConfig config;
    CompileFile(s, "./crypto/test_rc4.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, func, ret);
    return static_cast<int>(ret);
}

TEST(test_rc4, keystream) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    EXPECT_EQ(run_rc4_test(s, "CryptoTest.test_rc4_keystream"), 1);
    FakeluaDeleteState(s);
}

TEST(test_rc4, encrypt_decrypt) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    EXPECT_EQ(run_rc4_test(s, "CryptoTest.test_rc4_encrypt_decrypt"), 1);
    FakeluaDeleteState(s);
}
