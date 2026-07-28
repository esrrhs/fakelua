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
