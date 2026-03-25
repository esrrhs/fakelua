#include "compile/compiler.h"
#include "fakelua.h"
#include "gtest/gtest.h"

using namespace fakelua;

TEST(algo, fibonacci) {
    auto L = FakeluaNewState();
    ASSERT_NE(L.get(), nullptr);

    int i = 0;
    L->CompileFile("./algo/fibonacci.lua", {});
    L->call("test", std::tie(i), 30);
    ASSERT_EQ(i, 832040);

    i = 0;
    L->CompileFile("./algo/fibonacci.lua", {.debug_mode = false});
    L->call("test", std::tie(i), 30);
    ASSERT_EQ(i, 832040);
}
