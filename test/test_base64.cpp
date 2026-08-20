#include "fakelua.h"
#include "gtest/gtest.h"

using namespace fakelua;

TEST(test_base64, encode_decode) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_base64.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CryptoTest.test_base64", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}
