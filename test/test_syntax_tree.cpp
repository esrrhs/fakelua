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

    auto wantstr = ""
                   "(block)[2:1]\n"
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

    auto wantstr = ""
                   "(block)[2:1]\n"
                   "  (assign)[2:3]\n"
                   "    (varlist)[2:1]\n"
                   "      (var)[2:1]\n"
                   "        type: simple\n"
                   "        name: a\n"
                   "    (explist)[2:5]\n"
                   "      (exp)[2:5]\n"
                   "        type: number\n"
                   "        value: 1\n";

    ASSERT_EQ(dumpstr, wantstr);
}

TEST(syntax_tree, assign) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    compiler c;
    auto result = c.compile_file("./test_assign.lua");
    ASSERT_NE(result.chunk, nullptr);

    auto dumpstr = result.chunk->dump();
    LOG(INFO) << "\n" << dumpstr;

    auto wantstr = ""
                   "(block)[2:1]\n"
                   "  (assign)[2:14]\n"
                   "    (varlist)[2:1]\n"
                   "      (var)[2:1]\n"
                   "        type: simple\n"
                   "        name: a\n"
                   "      (var)[2:5]\n"
                   "        type: square\n"
                   "        (prefixexp)[2:4]\n"
                   "          type: var\n"
                   "          (var)[2:4]\n"
                   "            type: simple\n"
                   "            name: b\n"
                   "        (exp)[2:6]\n"
                   "          type: number\n"
                   "          value: 2\n"
                   "      (var)[2:11]\n"
                   "        type: dot\n"
                   "        (prefixexp)[2:10]\n"
                   "          type: var\n"
                   "          (var)[2:10]\n"
                   "            type: simple\n"
                   "            name: c\n"
                   "        name: d\n"
                   "    (explist)[2:16]\n"
                   "      (exp)[2:16]\n"
                   "        type: nil\n"
                   "        value: \n"
                   "      (exp)[2:21]\n"
                   "        type: tableconstructor\n"
                   "        value: \n"
                   "        (tableconstructor)[2:21]\n"
                   "          (empty)[2:21]\n"
                   "      (exp)[2:25]\n"
                   "        type: binop\n"
                   "        value: \n"
                   "        (exp)[2:25]\n"
                   "          type: prefixexp\n"
                   "          value: \n"
                   "          (prefixexp)[2:25]\n"
                   "            type: var\n"
                   "            (var)[2:25]\n"
                   "              type: simple\n"
                   "              name: e\n"
                   "        (binop)[2:27]\n"
                   "          op: PLUS\n"
                   "        (exp)[2:29]\n"
                   "          type: prefixexp\n"
                   "          value: \n"
                   "          (prefixexp)[2:29]\n"
                   "            type: var\n"
                   "            (var)[2:29]\n"
                   "              type: simple\n"
                   "              name: f\n";

    ASSERT_EQ(dumpstr, wantstr);
}