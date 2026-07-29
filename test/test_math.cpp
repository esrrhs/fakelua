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

TEST(test_math, test_math_extended) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./math/test_math_extended.lua", config);
        int64_t r1 = 0, r2 = 0, r3 = 0, r4 = 0;
        Call(s, jit_type, "test_math_constants", r1);
        EXPECT_EQ(r1, 100);

        Call(s, jit_type, "test_math_deg_rad", r2);
        EXPECT_EQ(r2, 200);

        Call(s, jit_type, "test_math_random", r3);
        EXPECT_EQ(r3, 300);

        Call(s, jit_type, "test_math_modf_frexp", r4);
        EXPECT_EQ(r4, 400);
    }

    FakeluaDeleteState(s);
}
