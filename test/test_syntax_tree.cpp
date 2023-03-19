#include "gtest/gtest.h"
#include "fakelua/fakelua.h"
#include "compile/compiler.h"

using namespace fakelua;

TEST(syntax_tree, label) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    compiler c;
    auto result = c.compile_file("./test_label.lua");
    ASSERT_NE(result.chunk, nullptr);

    auto dumpstr = result.chunk->dump();
    LOG(INFO) << "\n" << dumpstr;

    auto wantstr = "(block)[2:1]\n"
                   "  aa(label)[2:3]\n"
                   "  bb(label)[4:3]\n";

    ASSERT_EQ(dumpstr, wantstr);
}

TEST(syntax_tree, assign_simple) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    compiler c;
    auto result = c.compile_file("./test_assign_simple.lua");
    ASSERT_NE(result.chunk, nullptr);

    auto dumpstr = result.chunk->dump();
    LOG(INFO) << "\n" << dumpstr;

    auto wantstr = "(block)[2:1]\n"
                   "  aa(label)[2:3]\n"
                   "  bb(label)[4:3]\n";

    ASSERT_EQ(dumpstr, wantstr);
}
