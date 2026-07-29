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
        EXPECT_NEAR(res, 5.0, 1e-4);
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
