#include "compile/compiler.h"
#include "fakelua.h"
#include "gtest/gtest.h"

using namespace fakelua;

TEST(jitter, empty_file) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    compiler c;
    auto result = c.compile_file(L, "./jit/test_empty_file.lua", {});
}

TEST(jitter, empty_func) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    compiler c;
    auto result = c.compile_file(L, "./jit/test_empty_func.lua", {});
}

TEST(jitter, const_define) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    compiler c;
    auto result = c.compile_file(L, "./jit/test_const_define.lua", {});
}
