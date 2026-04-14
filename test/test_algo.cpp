#include "compile/compiler.h"
#include "fakelua.h"
#include "gtest/gtest.h"

using namespace fakelua;

TEST(algo, fibonacci) {
    const auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);

    int i = 0;
    CompileFile(s, "./algo/fibonacci.lua", {});
    Call(s, JIT_TCC, "test", i, 30);
    ASSERT_EQ(i, 832040);

    i = 0;
    CompileFile(s, "./algo/fibonacci.lua", {.debug_mode = false});
    Call(s, JIT_TCC, "test", i, 30);
    ASSERT_EQ(i, 832040);
}
