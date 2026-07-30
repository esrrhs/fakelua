#include <gtest/gtest.h>

#include "fakelua.h"

using namespace fakelua;

TEST(test_basic, test_basic_global) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        CompileFile(s, "./basic/test_basic_global.lua", config);
        double res = 0;
        Call(s, jit_type, "test_basic_global", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}
