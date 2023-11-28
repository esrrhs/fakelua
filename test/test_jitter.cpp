#include "compile/compiler.h"
#include "fakelua.h"
#include "gtest/gtest.h"

using namespace fakelua;

TEST(jitter, const_define) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    compiler c;
    auto result = c.compile_file(L, "./jit/test_const_define.lua", {});
}
