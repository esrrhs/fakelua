#include "fakelua.h"
#include "gtest/gtest.h"

using namespace fakelua;

TEST(test_blowfish, encrypt_decrypt) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_blowfish.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CryptoTest.test_blowfish", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}
