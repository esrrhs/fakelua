#include <gtest/gtest.h>

#include "fakelua.h"

using namespace fakelua;

TEST(test_math, test_math_basic) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./math/test_math_basic.lua", config);
        double res1 = 0;
        Call(s, jit_type, "test_math_basic", res1);
        EXPECT_NEAR(res1, 157.14159265358979, 1e-4);
    }

    FakeluaDeleteState(s);
}

TEST(test_math, test_math_trig) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./math/test_math_trig.lua", config);
        double res2 = 0.0;
        Call(s, jit_type, "test_math_trig", res2);
        EXPECT_NEAR(res2, 184.14159265358979, 1e-4);
    }

    FakeluaDeleteState(s);
}

TEST(test_math, test_math_exp_log) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./math/test_math_exp_log.lua", config);
        double res = 0.0;
        Call(s, jit_type, "test_math_exp_log", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_math, test_math_utils) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./math/test_math_utils.lua", config);
        double res = 0.0;
        Call(s, jit_type, "test_math_utils", res);
        EXPECT_NEAR(res, 33.5, 1e-4);
    }

    FakeluaDeleteState(s);
}

TEST(test_math, test_math_constants) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./math/test_math_constants.lua", config);
        int64_t res = 0;
        Call(s, jit_type, "test_math_constants", res);
        EXPECT_EQ(res, 100);
    }

    FakeluaDeleteState(s);
}

TEST(test_math, test_math_deg_rad) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./math/test_math_deg_rad.lua", config);
        int64_t res = 0;
        Call(s, jit_type, "test_math_deg_rad", res);
        EXPECT_EQ(res, 200);
    }

    FakeluaDeleteState(s);
}

TEST(test_math, test_math_random) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./math/test_math_random.lua", config);
        int64_t res = 0;
        Call(s, jit_type, "test_math_random", res);
        EXPECT_EQ(res, 300);
    }

    FakeluaDeleteState(s);
}

TEST(test_math, test_math_random_reverse_interval) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    CompileFile(s, "./math/test_math_random.lua", config);
    // TCC 是 C 编译器，不支持 C++ 异常传播，只测试 GCC 后端
    // math.random(100, 50) 应抛 "interval is empty"
    double res = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "test_math_random_reverse_interval", res), std::exception);

    FakeluaDeleteState(s);
}

TEST(test_math, test_math_modf_frexp) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./math/test_math_modf_frexp.lua", config);
        int64_t res = 0;
        Call(s, jit_type, "test_math_modf_frexp", res);
        EXPECT_EQ(res, 400);
    }

    FakeluaDeleteState(s);
}


TEST(test_math, test_math_atan2) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./math/test_math_atan2.lua", config);
        double res = 0;
        Call(s, jit_type, "test_math_atan2", res);
        EXPECT_NEAR(res, 6000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_math, test_math_copysign) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./math/test_math_copysign.lua", config);
        double res = 0;
        Call(s, jit_type, "test_math_copysign", res);
        EXPECT_NEAR(res, 7000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_math, test_math_tointeger_bad_arg) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    CompileFile(s, "./math/test_math_tointeger_bad_arg.lua", config);

    // TCC 是 C 编译器，不支持 C++ 异常传播，只测试 GCC 后端
    double res = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "test_math_tointeger_bad_arg", res), std::exception);

    FakeluaDeleteState(s);
}

TEST(test_math, test_math_tointeger_bad_arg_table) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    CompileFile(s, "./math/test_math_tointeger_bad_arg_table.lua", config);

    // TCC 是 C 编译器，不支持 C++ 异常传播，只测试 GCC 后端
    double res = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "test_math_tointeger_bad_tbl", res), std::exception);

    FakeluaDeleteState(s);
}

TEST(test_math, test_math_abs_bad_arg) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    CompileFile(s, "./math/test_math_abs_bad_arg.lua", config);

    // TCC 是 C 编译器，不支持 C++ 异常传播，只测试 GCC 后端
    double res = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "test_math_abs_bad_arg", res), std::exception);

    FakeluaDeleteState(s);
}

TEST(test_math, test_math_abs_bad_arg_table) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    CompileFile(s, "./math/test_math_abs_bad_arg_table.lua", config);

    // TCC 是 C 编译器，不支持 C++ 异常传播，只测试 GCC 后端
    double res = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "test_math_abs_bad_tbl", res), std::exception);

    FakeluaDeleteState(s);
}

TEST(test_math, test_math_jit_guard) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    CompileFile(s, "./math/test_math_jit_guard.lua", config);

    double res = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "test_math_sqrt_bad_arg", res), std::exception);
    EXPECT_THROW(Call(s, JIT_GCC, "test_math_sin_bad_arg", res), std::exception);
    EXPECT_THROW(Call(s, JIT_GCC, "test_math_fmod_bad_arg", res), std::exception);
    EXPECT_THROW(Call(s, JIT_GCC, "test_math_randomseed_bad_table", res), std::exception);
    EXPECT_THROW(Call(s, JIT_GCC, "test_math_modf_bad_arg", res), std::exception);

    FakeluaDeleteState(s);
}

TEST(test_math, test_math_randomseed_nan) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./math/test_math_jit_guard.lua", config);
        double res = 0;
        Call(s, jit_type, "test_math_randomseed_nan", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_math, test_math_abs) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./math/test_math_abs.lua", config);
        double res = 0;
        Call(s, jit_type, "test_math_abs", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_math, test_math_floor_ceil) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./math/test_math_floor_ceil.lua", config);
        double res = 0;
        Call(s, jit_type, "test_math_floor_ceil", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_math, test_math_sqrt) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./math/test_math_sqrt.lua", config);
        double res = 0;
        Call(s, jit_type, "test_math_sqrt", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_math, test_math_trig_full) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./math/test_math_trig_full.lua", config);
        double res = 0;
        Call(s, jit_type, "test_math_trig_full", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_math, test_math_sinh_cosh_tanh) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./math/test_math_sinh_cosh_tanh.lua", config);
        double res = 0;
        Call(s, jit_type, "test_math_sinh_cosh_tanh", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_math, test_math_fmod_ldexp) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./math/test_math_fmod_ldexp.lua", config);
        double res = 0;
        Call(s, jit_type, "test_math_fmod_ldexp", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_math, test_math_ult_ldexp_int) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./math/test_math_ult_ldexp_int.lua", config);
        double res = 0;
        Call(s, jit_type, "test_math_ult_ldexp_int", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_math, test_math_ult_2pow63) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    CompileFile(s, "./math/test_math_ult_ldexp_int.lua", config);
    double res = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "test_math_ult_2pow63", res), std::exception);
    EXPECT_THROW(Call(s, JIT_GCC, "test_math_ult_frac", res), std::exception);
    EXPECT_THROW(Call(s, JIT_GCC, "test_math_ldexp_2pow63", res), std::exception);

    FakeluaDeleteState(s);
}

TEST(test_math, test_math_type_tointeger) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./math/test_math_type_tointeger.lua", config);
        double res = 0;
        Call(s, jit_type, "test_math_type_tointeger", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_math, test_math_max_min) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./math/test_math_max_min.lua", config);
        double res = 0;
        Call(s, jit_type, "test_math_max_min", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_math, test_math_constants_full) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./math/test_math_constants_full.lua", config);
        double res = 0;
        Call(s, jit_type, "test_math_constants_full", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_math, test_math_log_with_base) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./math/test_math_log_atan.lua", config);
        double res = 0;
        Call(s, jit_type, "test_math_log_with_base", res);
        EXPECT_NEAR(res, 5000, 0.5);

        res = 0;
        Call(s, jit_type, "test_math_atan_two_args", res);
        EXPECT_NEAR(res, 5000, 0.5);

        res = 0;
        Call(s, jit_type, "test_math_modf_int", res);
        EXPECT_NEAR(res, 5000, 0.5);

        res = 0;
        Call(s, jit_type, "test_math_random_range", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_math, test_math_type_nil) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./math/test_math_type_nil.lua", config);
        int64_t res = 0;
        Call(s, jit_type, "test_math_type_nil", res);
        EXPECT_EQ(res, 5000);
    }

    FakeluaDeleteState(s);
}

TEST(test_math, test_math_fmod_zero) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./math/test_math_fmod_zero.lua", config);
        int64_t res = 0;
        Call(s, jit_type, "test_math_fmod_zero", res);
        EXPECT_EQ(res, 5000);
    }

    FakeluaDeleteState(s);
}

TEST(test_math, test_math_boundary_nan_inf) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./math/test_math_boundary_nan_inf.lua", config);
        int64_t res = 0;
        Call(s, jit_type, "test_math_boundary_nan_inf", res);
        EXPECT_EQ(res, 5000);
    }

    FakeluaDeleteState(s);
}

TEST(test_math, test_math_boundary_integer) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./math/test_math_boundary_integer.lua", config);
        int64_t res = 0;
        Call(s, jit_type, "test_math_boundary_integer", res);
        EXPECT_EQ(res, 5000);
    }

    FakeluaDeleteState(s);
}

TEST(test_math, test_math_boundary_error) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    CompileFile(s, "./math/test_math_boundary_error.lua", config);

    // TCC 是 C 编译器，不支持 C++ 异常传播，只测试 GCC 后端
    double res = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "test_math_random_bad_arg", res), std::exception);
    EXPECT_THROW(Call(s, JIT_GCC, "test_math_randomseed_bad_arg", res), std::exception);

    FakeluaDeleteState(s);
}

TEST(test_math, test_math_critical_boundary) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    // TCC 是 C 编译器，不支持 C++ 异常传播和某些表操作，只测试 GCC 后端
    CompileFile(s, "./math/test_math_critical_boundary.lua", config);
    int64_t res = 0;
    Call(s, JIT_GCC, "test_math_critical_boundary", res);
    EXPECT_EQ(res, 9999);

    FakeluaDeleteState(s);
}

TEST(test_math, test_math_int64_min_arith) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./math/test_math_int64_min_arith.lua", config);
        int64_t res = 0;
        Call(s, jit_type, "test_math_int64_min_arith", res);
        EXPECT_EQ(res, 100);
    }

    FakeluaDeleteState(s);
}

TEST(test_math, test_float_2pow63) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./math/test_float_2pow63.lua", config);
        int64_t res = 0;
        Call(s, jit_type, "test_float_2pow63", res);
        EXPECT_EQ(res, 100);
    }

    FakeluaDeleteState(s);
}

TEST(test_math, test_for_int64_overflow) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./math/test_for_int64_overflow.lua", config);
        int64_t res = 0;
        Call(s, jit_type, "test_for_int64_overflow", res);
        EXPECT_EQ(res, 100);
    }

    FakeluaDeleteState(s);
}


TEST(test_math, test_string_sub_overflow) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    CompileFile(s, "./math/test_math_critical_boundary.lua", config);
    // 超大 float index 应抛 "number has no integer representation"
    double res = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "test_string_sub_overflow", res), std::exception);

    FakeluaDeleteState(s);
}

TEST(test_math, test_string_byte_overflow) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    CompileFile(s, "./math/test_math_critical_boundary.lua", config);
    double res = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "test_string_byte_overflow", res), std::exception);

    FakeluaDeleteState(s);
}

TEST(test_math, test_math_random_neg) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    CompileFile(s, "./math/test_math_critical_boundary.lua", config);
    double res = 0;
    // math.random(-5) 应抛 "interval is empty"
    EXPECT_THROW(Call(s, JIT_GCC, "test_math_random_neg", res), std::exception);

    FakeluaDeleteState(s);
}

TEST(test_math, test_math_random_reverse) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    CompileFile(s, "./math/test_math_critical_boundary.lua", config);
    double res = 0;
    // math.random(5,3) 应抛 "interval is empty"
    EXPECT_THROW(Call(s, JIT_GCC, "test_math_random_reverse", res), std::exception);

    FakeluaDeleteState(s);
}

TEST(test_math, test_math_random_2pow63) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    CompileFile(s, "./math/test_math_critical_boundary.lua", config);
    double res = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "test_math_random_2pow63", res), std::exception);

    FakeluaDeleteState(s);
}

TEST(test_math, test_string_method_colon) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    // 验证 string 库方法 colon 调用不 crash（Bug #1: FlGetTableStrId on string）
    CompileFile(s, "./math/test_math_critical_boundary.lua", config);
    int64_t res = 0;
    Call(s, JIT_GCC, "test_string_method_colon", res);
    EXPECT_EQ(res, 0);

    FakeluaDeleteState(s);
}

TEST(test_math, test_err_match) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    // 验证 err:match 不 crash（Bug #1: FlGetTableStrId on string in method call）
    CompileFile(s, "./math/test_math_critical_boundary.lua", config);
    int64_t res = 0;
    Call(s, JIT_GCC, "test_err_match", res);
    EXPECT_EQ(res, 0);

    FakeluaDeleteState(s);
}

// Math unified tests (GCC backend for coverage)
TEST(test_math, unified_abs_various) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_unified.lua", config);
    int64_t ret = 0;
    Call(s, JIT_GCC, "MathUnified.test_math_abs_various", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math, unified_floor_ceil_various) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_unified.lua", config);
    int64_t ret = 0;
    Call(s, JIT_GCC, "MathUnified.test_math_floor_ceil_various", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math, unified_sqrt_various) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_unified.lua", config);
    int64_t ret = 0;
    Call(s, JIT_GCC, "MathUnified.test_math_sqrt_various", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math, unified_pow_various) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_unified.lua", config);
    int64_t ret = 0;
    Call(s, JIT_GCC, "MathUnified.test_math_pow_various", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math, unified_trig_various) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_unified.lua", config);
    int64_t ret = 0;
    Call(s, JIT_GCC, "MathUnified.test_math_trig_various", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math, unified_asin_acos) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_unified.lua", config);
    int64_t ret = 0;
    Call(s, JIT_GCC, "MathUnified.test_math_asin_acos", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math, unified_atan_various) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_unified.lua", config);
    int64_t ret = 0;
    Call(s, JIT_GCC, "MathUnified.test_math_atan_various", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math, unified_exp_log_various) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_unified.lua", config);
    int64_t ret = 0;
    Call(s, JIT_GCC, "MathUnified.test_math_exp_log_various", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math, unified_hyperbolic) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_unified.lua", config);
    int64_t ret = 0;
    Call(s, JIT_GCC, "MathUnified.test_math_hyperbolic", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math, unified_fmod_various) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_unified.lua", config);
    int64_t ret = 0;
    Call(s, JIT_GCC, "MathUnified.test_math_fmod_various", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math, unified_ldexp_various) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_unified.lua", config);
    int64_t ret = 0;
    Call(s, JIT_GCC, "MathUnified.test_math_ldexp_various", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math, unified_deg_rad_various) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_unified.lua", config);
    int64_t ret = 0;
    Call(s, JIT_GCC, "MathUnified.test_math_deg_rad_various", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math, unified_copysign_various) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_unified.lua", config);
    int64_t ret = 0;
    Call(s, JIT_GCC, "MathUnified.test_math_copysign_various", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math, unified_modf_various) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_unified.lua", config);
    int64_t ret = 0;
    Call(s, JIT_GCC, "MathUnified.test_math_modf_various", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math, unified_frexp_various) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_unified.lua", config);
    int64_t ret = 0;
    Call(s, JIT_GCC, "MathUnified.test_math_frexp_various", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math, unified_type_various) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_unified.lua", config);
    int64_t ret = 0;
    Call(s, JIT_GCC, "MathUnified.test_math_type_various", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math, unified_tointeger_various) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_unified.lua", config);
    int64_t ret = 0;
    Call(s, JIT_GCC, "MathUnified.test_math_tointeger_various", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math, unified_ult_various) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_unified.lua", config);
    int64_t ret = 0;
    Call(s, JIT_GCC, "MathUnified.test_math_ult_various", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math, unified_max_min_various) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_unified.lua", config);
    int64_t ret = 0;
    Call(s, JIT_GCC, "MathUnified.test_math_max_min_various", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math, unified_random_various) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_unified.lua", config);
    int64_t ret = 0;
    Call(s, JIT_GCC, "MathUnified.test_math_random_various", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math, unified_random_m) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_unified.lua", config);
    int64_t ret = 0;
    Call(s, JIT_GCC, "MathUnified.test_math_random_m", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math, unified_random_m_n) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_unified.lua", config);
    int64_t ret = 0;
    Call(s, JIT_GCC, "MathUnified.test_math_random_m_n", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math, unified_randomseed_various) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_unified.lua", config);
    int64_t ret = 0;
    Call(s, JIT_GCC, "MathUnified.test_math_randomseed_various", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

// Math edge case tests
TEST(test_math, math_random_zero) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "MathTest.test_math_random_zero", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math, math_randomseed_with_arg) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "MathTest.test_math_randomseed_with_arg", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math, math_randomseed_no_arg) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "MathTest.test_math_randomseed_no_arg", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math, math_random_int_arg) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "MathTest.test_math_random_int_arg", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math, math_random_int_args) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "MathTest.test_math_random_int_args", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math, math_random_negative) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "MathTest.test_math_random_negative", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math, math_random_reverse) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "MathTest.test_math_random_reverse", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math, math_abs_int64_min) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "MathTest.test_math_abs_int64_min", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math, math_modf_negative) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "MathTest.test_math_modf_negative", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math, math_frexp_negative) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "MathTest.test_math_frexp_negative", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math, math_frexp_zero) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "MathTest.test_math_frexp_zero", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math, math_type_various) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "MathTest.test_math_type_various", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math, math_tointeger_various) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "MathTest.test_math_tointeger_various", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math, math_ult_unsigned) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "MathTest.test_math_ult_unsigned", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math, math_deg_various) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "MathTest.test_math_deg_various", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math, math_rad_various) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "MathTest.test_math_rad_various", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math, math_copysign_various) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "MathTest.test_math_copysign_various", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math, math_log_with_base) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "MathTest.test_math_log_with_base", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math, math_atan_two_args) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "MathTest.test_math_atan_two_args", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math, math_max_string) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "MathTest.test_math_max_string", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_math, math_floor_ceil_int) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./math/test_math_edge.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "MathTest.test_math_floor_ceil_int", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}
