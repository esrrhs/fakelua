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
