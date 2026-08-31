#include "fakelua.h"
#include "gtest/gtest.h"

using namespace fakelua;

TEST(test_string_edge, test_pack_basic) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./string/test_string_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "test_pack_basic", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_string_edge, test_packsize_basic) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./string/test_string_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "test_packsize_basic", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_string_edge, test_unpack_basic) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./string/test_string_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "test_unpack_basic", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_string_edge, test_find_plain) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./string/test_string_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "test_find_plain", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_string_edge, test_gsub_replace) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./string/test_string_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "test_gsub_replace", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_string_edge, test_gsub_count) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./string/test_string_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "test_gsub_count", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}
