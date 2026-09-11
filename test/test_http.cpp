#include "fakelua.h"
#include "test_jit.h"
#include "gtest/gtest.h"

using namespace fakelua;

TEST(test_http, echo) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./http/test_http.lua", config);
    int64_t ret = 0;
    CallAll(s, "HttpTest.test_echo", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_http, connect_fail) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./http/test_http.lua", config);
    int64_t ret = 0;
    CallAll(s, "HttpTest.test_connect_fail", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_http, tls_echo) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./http/test_http.lua", config);
    int64_t ret = 0;
    CallAll(s, "HttpTest.test_tls_echo", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}
