#include "fakelua.h"
#include "gtest/gtest.h"
#include <cmath>

using namespace fakelua;

TEST(test_math, test_native_math_builtins) {
    auto *s = FakeluaNewState();

    CompileConfig config;
    CompileFile(s, "./test_math.lua", config);

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        double res1 = 0;
        Call(s, jit_type, "test_math_basic", res1);
        EXPECT_DOUBLE_EQ(res1, 86.0);

        double res2 = 0.0;
        Call(s, jit_type, "test_math_trig", res2);
        EXPECT_NEAR(res2, 181.0, 1e-4);
    }

    FakeluaDeleteState(s);
}
