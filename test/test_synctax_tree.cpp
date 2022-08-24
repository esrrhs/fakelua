#include "gtest/gtest.h"
#include "fakelua/fakelua.h"

using namespace fakelua;

TEST(syntax_tree, label) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    L->compile_file("./test_label.lua");
}
