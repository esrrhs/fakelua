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

TEST(test_url, format_bad_port) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./url/test_url.lua", config);
    int64_t ret = 0;
    CallAll(s, "UrlTest.test_format_bad_port", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_url, authority_optional_query_values_and_malformed_escapes) {
    FakeluaStateGuard state;
    CompileConfig config;
    CompileFile(state.GetState(), "./url/test_url_additional_contracts.lua", config);
    int64_t result = 0;
    CallAll(state.GetState(), "test_url_contracts", result);
    EXPECT_EQ(result, 1);
}
