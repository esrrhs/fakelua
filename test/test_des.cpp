#include "fakelua.h"
#include "gtest/gtest.h"

using namespace fakelua;

TEST(test_des, encrypt_decrypt) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_des.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CryptoTest.test_des", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_des, triple_des) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./crypto/test_des.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "CryptoTest.test_triple_des", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}
