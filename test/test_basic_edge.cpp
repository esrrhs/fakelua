#include "fakelua.h"
#include "gtest/gtest.h"

using namespace fakelua;

TEST(test_basic_edge, test_select_number) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "BasicTest.test_select_number", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic_edge, test_select_hash) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "BasicTest.test_select_hash", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic_edge, test_select_out_of_range) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "BasicTest.test_select_out_of_range", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic_edge, test_error_basic) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "BasicTest.test_error_basic", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic_edge, test_error_with_level) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "BasicTest.test_error_with_level", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic_edge, test_assert_success) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "BasicTest.test_assert_success", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic_edge, test_assert_fail) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "BasicTest.test_assert_fail", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic_edge, test_assert_no_msg) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "BasicTest.test_assert_no_msg", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic_edge, test_pcall_success) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "BasicTest.test_pcall_success", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic_edge, test_pcall_fail) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "BasicTest.test_pcall_fail", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic_edge, test_xpcall_success) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "BasicTest.test_xpcall_success", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic_edge, test_xpcall_fail) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "BasicTest.test_xpcall_fail", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic_edge, test_type_various) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "BasicTest.test_type_various", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic_edge, test_tostring_various) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "BasicTest.test_tostring_various", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic_edge, test_tonumber_various) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "BasicTest.test_tonumber_various", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic_edge, test_print_basic) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "BasicTest.test_print_basic", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic_edge, test_pairs_iter) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "BasicTest.test_pairs_iter", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic_edge, test_ipairs_iter) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "BasicTest.test_ipairs_iter", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic_edge, test_next_func) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "BasicTest.test_next_func", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_basic_edge, test_collectgarbage) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./basic/test_basic_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "BasicTest.test_collectgarbage", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}
