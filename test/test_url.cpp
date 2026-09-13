#include "fakelua.h"
#include "test_jit.h"
#include "gtest/gtest.h"

using namespace fakelua;

TEST(test_url, parse) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./url/test_url.lua", config);
    int64_t ret = 0;
    CallAll(s, "UrlTest.test_parse", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_url, format) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./url/test_url.lua", config);
    int64_t ret = 0;
    CallAll(s, "UrlTest.test_format", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_url, encode_decode) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./url/test_url.lua", config);
    int64_t ret = 0;
    CallAll(s, "UrlTest.test_encode_decode", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}
