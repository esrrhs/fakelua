#include "fakelua.h"
#include "gtest/gtest.h"

using namespace fakelua;

TEST(test_math_edge, test_math_random_zero) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "MathTest.test_math_random_zero", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math_edge, test_math_randomseed_with_arg) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "MathTest.test_math_randomseed_with_arg", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math_edge, test_math_randomseed_no_arg) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "MathTest.test_math_randomseed_no_arg", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math_edge, test_math_random_int_arg) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "MathTest.test_math_random_int_arg", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math_edge, test_math_random_int_args) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "MathTest.test_math_random_int_args", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math_edge, test_math_random_negative) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "MathTest.test_math_random_negative", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math_edge, test_math_random_reverse) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "MathTest.test_math_random_reverse", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math_edge, test_math_abs_int64_min) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "MathTest.test_math_abs_int64_min", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math_edge, test_math_modf_negative) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "MathTest.test_math_modf_negative", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math_edge, test_math_frexp_negative) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "MathTest.test_math_frexp_negative", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math_edge, test_math_frexp_zero) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "MathTest.test_math_frexp_zero", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math_edge, test_math_type_various) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "MathTest.test_math_type_various", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math_edge, test_math_tointeger_various) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "MathTest.test_math_tointeger_various", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math_edge, test_math_ult_unsigned) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "MathTest.test_math_ult_unsigned", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math_edge, test_math_deg_various) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "MathTest.test_math_deg_various", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math_edge, test_math_rad_various) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "MathTest.test_math_rad_various", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math_edge, test_math_copysign_various) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "MathTest.test_math_copysign_various", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math_edge, test_math_log_with_base) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "MathTest.test_math_log_with_base", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math_edge, test_math_atan_two_args) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "MathTest.test_math_atan_two_args", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math_edge, test_math_max_string) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "MathTest.test_math_max_string", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math_edge, test_math_floor_ceil_int) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "MathTest.test_math_floor_ceil_int", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}
