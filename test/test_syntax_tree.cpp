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

TEST(syntax_tree, function_call) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    compiler c;
    auto result = c.compile_file("./test_function_call.lua");
    ASSERT_NE(result.chunk, nullptr);

    auto dumpstr = result.chunk->dump();
    LOG(INFO) << "\n" << dumpstr;

    auto wantstr = ""
                   "(block)[2:1]\n"
                   "  (functioncall)[2:1]\n"
                   "    (prefixexp)[2:1]\n"
                   "      type: functioncall\n"
                   "      (functioncall)[2:1]\n"
                   "        (prefixexp)[2:1]\n"
                   "          type: var\n"
                   "          (var)[2:7]\n"
                   "            type: dot\n"
                   "            (prefixexp)[2:1]\n"
                   "              type: var\n"
                   "              (var)[2:4]\n"
                   "                type: square\n"
                   "                (prefixexp)[2:1]\n"
                   "                  type: var\n"
                   "                  (var)[2:2]\n"
                   "                    type: dot\n"
                   "                    (prefixexp)[2:1]\n"
                   "                      type: var\n"
                   "                      (var)[2:1]\n"
                   "                        type: simple\n"
                   "                        name: a\n"
                   "                    name: b\n"
                   "                (exp)[2:5]\n"
                   "                  type: number\n"
                   "                  value: 1\n"
                   "            name: func_a\n"
                   "        (args)[2:14]\n"
                   "          (explist)[2:15]\n"
                   "            (exp)[2:15]\n"
                   "              type: prefixexp\n"
                   "              value: \n"
                   "              (prefixexp)[2:15]\n"
                   "                type: var\n"
                   "                (var)[2:15]\n"
                   "                  type: simple\n"
                   "                  name: a\n"
                   "            (exp)[2:17]\n"
                   "              type: prefixexp\n"
                   "              value: \n"
                   "              (prefixexp)[2:17]\n"
                   "                type: var\n"
                   "                (var)[2:17]\n"
                   "                  type: simple\n"
                   "                  name: b\n"
                   "            (exp)[2:19]\n"
                   "              type: prefixexp\n"
                   "              value: \n"
                   "              (prefixexp)[2:19]\n"
                   "                type: var\n"
                   "                (var)[2:19]\n"
                   "                  type: simple\n"
                   "                  name: c\n"
                   "    name: func_c\n"
                   "    (args)[2:28]\n"
                   "      empty\n";

    ASSERT_EQ(dumpstr, wantstr);
}

TEST(syntax_tree, break) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    compiler c;
    auto result = c.compile_file("./test_break.lua");
    ASSERT_NE(result.chunk, nullptr);

    auto dumpstr = result.chunk->dump();
    LOG(INFO) << "\n" << dumpstr;

    auto wantstr = ""
                   "(block)[2:1]\n"
                   "  (while)[2:1]\n"
                   "    (exp)[2:7]\n"
                   "      type: true\n"
                   "      value: \n"
                   "    (block)[3:5]\n"
                   "      (break)[3:5]\n";

    ASSERT_EQ(dumpstr, wantstr);
}

TEST(syntax_tree, continue) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    compiler c;
    auto result = c.compile_file("./test_continue.lua");
    ASSERT_NE(result.chunk, nullptr);

    auto dumpstr = result.chunk->dump();
    LOG(INFO) << "\n" << dumpstr;

    auto wantstr = ""
                   "(block)[1:1]\n"
                   "  (local_var)[1:1]\n"
                   "    (namelist)[1:7]\n"
                   "      name: a\n"
                   "    (explist)[1:11]\n"
                   "      (exp)[1:11]\n"
                   "        type: tableconstructor\n"
                   "        value: \n"
                   "        (tableconstructor)[1:11]\n"
                   "          (fieldlist)[1:13]\n"
                   "            (field)[1:13]\n"
                   "              type: object\n"
                   "              name: b\n"
                   "              (exp)[1:17]\n"
                   "                type: number\n"
                   "                value: 1\n"
                   "            (field)[1:20]\n"
                   "              type: object\n"
                   "              name: c\n"
                   "              (exp)[1:24]\n"
                   "                type: string\n"
                   "                value: 2\n"
                   "  (goto)[2:6]\n"
                   "    label: continue\n"
                   "  continue(label)[3:4]\n";

    ASSERT_EQ(dumpstr, wantstr);
}

TEST(syntax_tree, do_end) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    compiler c;
    auto result = c.compile_file("./test_do_end.lua");
    ASSERT_NE(result.chunk, nullptr);

    auto dumpstr = result.chunk->dump();
    LOG(INFO) << "\n" << dumpstr;

    auto wantstr = ""
                   "(block)[1:1]\n"
                   "  (block)[1:4]\n"
                   "    (local_var)[1:4]\n"
                   "      (namelist)[1:10]\n"
                   "        name: a\n"
                   "      (explist)[1:14]\n"
                   "        (exp)[1:14]\n"
                   "          type: binop\n"
                   "          value: \n"
                   "          (exp)[1:14]\n"
                   "            type: prefixexp\n"
                   "            value: \n"
                   "            (prefixexp)[1:14]\n"
                   "              type: var\n"
                   "              (var)[1:15]\n"
                   "                type: dot\n"
                   "                (prefixexp)[1:14]\n"
                   "                  type: var\n"
                   "                  (var)[1:14]\n"
                   "                    type: simple\n"
                   "                    name: b\n"
                   "                name: c\n"
                   "          (binop)[1:18]\n"
                   "            op: OR\n"
                   "          (exp)[1:21]\n"
                   "            type: tableconstructor\n"
                   "            value: \n"
                   "            (tableconstructor)[1:21]\n"
                   "              (empty)[1:21]\n";

    ASSERT_EQ(dumpstr, wantstr);
}

TEST(syntax_tree, while) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    compiler c;
    auto result = c.compile_file("./test_while.lua");
    ASSERT_NE(result.chunk, nullptr);

    auto dumpstr = result.chunk->dump();
    LOG(INFO) << "\n" << dumpstr;

    auto wantstr = ""
                   "(block)[2:1]\n"
                   "  (while)[2:1]\n"
                   "    (exp)[2:7]\n"
                   "      type: unop\n"
                   "      value: \n"
                   "      (unop)[2:7]\n"
                   "        op: NOT\n"
                   "      (exp)[2:11]\n"
                   "        type: prefixexp\n"
                   "        value: \n"
                   "        (prefixexp)[2:11]\n"
                   "          type: functioncall\n"
                   "          (functioncall)[2:11]\n"
                   "            (prefixexp)[2:11]\n"
                   "              type: var\n"
                   "              (var)[2:11]\n"
                   "                type: simple\n"
                   "                name: loop\n"
                   "            (args)[2:15]\n"
                   "              empty\n"
                   "    (block)[2:18]\n";

    ASSERT_EQ(dumpstr, wantstr);
}

TEST(syntax_tree, repeat) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    compiler c;
    auto result = c.compile_file("./test_repeat.lua");
    ASSERT_NE(result.chunk, nullptr);

    auto dumpstr = result.chunk->dump();
    LOG(INFO) << "\n" << dumpstr;

    auto wantstr = ""
                   "(block)[1:1]\n"
                   "  (repeat)[1:1]\n"
                   "    (block)[2:5]\n"
                   "      (assign)[2:7]\n"
                   "        (varlist)[2:5]\n"
                   "          (var)[2:5]\n"
                   "            type: simple\n"
                   "            name: n\n"
                   "        (explist)[2:9]\n"
                   "          (exp)[2:9]\n"
                   "            type: binop\n"
                   "            value: \n"
                   "            (exp)[2:9]\n"
                   "              type: prefixexp\n"
                   "              value: \n"
                   "              (prefixexp)[2:9]\n"
                   "                type: var\n"
                   "                (var)[2:9]\n"
                   "                  type: simple\n"
                   "                  name: n\n"
                   "            (binop)[2:11]\n"
                   "              op: PLUS\n"
                   "            (exp)[2:13]\n"
                   "              type: number\n"
                   "              value: 1\n"
                   "    (exp)[3:7]\n"
                   "      type: binop\n"
                   "      value: \n"
                   "      (exp)[3:7]\n"
                   "        type: prefixexp\n"
                   "        value: \n"
                   "        (prefixexp)[3:7]\n"
                   "          type: var\n"
                   "          (var)[3:7]\n"
                   "            type: simple\n"
                   "            name: n\n"
                   "      (binop)[3:9]\n"
                   "        op: EQUAL\n"
                   "      (exp)[3:12]\n"
                   "        type: number\n"
                   "        value: 0\n";

    ASSERT_EQ(dumpstr, wantstr);
}

TEST(syntax_tree, if) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    compiler c;
    auto result = c.compile_file("./test_if.lua");
    ASSERT_NE(result.chunk, nullptr);

    auto dumpstr = result.chunk->dump();
    LOG(INFO) << "\n" << dumpstr;

    auto wantstr = ""
                   "(block)[1:1]\n"
                   "  (if)[1:1]\n"
                   "    (exp)[1:4]\n"
                   "      type: prefixexp\n"
                   "      value: \n"
                   "      (prefixexp)[1:4]\n"
                   "        type: exp\n"
                   "        (exp)[1:5]\n"
                   "          type: prefixexp\n"
                   "          value: \n"
                   "          (prefixexp)[1:5]\n"
                   "            type: var\n"
                   "            (var)[1:17]\n"
                   "              type: dot\n"
                   "              (prefixexp)[1:5]\n"
                   "                type: var\n"
                   "                (var)[1:9]\n"
                   "                  type: dot\n"
                   "                  (prefixexp)[1:5]\n"
                   "                    type: var\n"
                   "                    (var)[1:5]\n"
                   "                      type: simple\n"
                   "                      name: self\n"
                   "                  name: methods\n"
                   "              name: init\n"
                   "    (block)[2:5]\n"
                   "      (if)[2:5]\n"
                   "        (exp)[2:8]\n"
                   "          type: prefixexp\n"
                   "          value: \n"
                   "          (prefixexp)[2:8]\n"
                   "            type: exp\n"
                   "            (exp)[2:9]\n"
                   "              type: binop\n"
                   "              value: \n"
                   "              (exp)[2:9]\n"
                   "                type: prefixexp\n"
                   "                value: \n"
                   "                (prefixexp)[2:9]\n"
                   "                  type: functioncall\n"
                   "                  (functioncall)[2:9]\n"
                   "                    (prefixexp)[2:9]\n"
                   "                      type: var\n"
                   "                      (var)[2:9]\n"
                   "                        type: simple\n"
                   "                        name: tostring\n"
                   "                    (args)[2:17]\n"
                   "                      (explist)[2:18]\n"
                   "                        (exp)[2:18]\n"
                   "                          type: prefixexp\n"
                   "                          value: \n"
                   "                          (prefixexp)[2:18]\n"
                   "                            type: var\n"
                   "                            (var)[2:21]\n"
                   "                              type: square\n"
                   "                              (prefixexp)[2:18]\n"
                   "                                type: var\n"
                   "                                (var)[2:18]\n"
                   "                                  type: simple\n"
                   "                                  name: arg\n"
                   "                              (exp)[2:22]\n"
                   "                                type: number\n"
                   "                                value: 1\n"
                   "              (binop)[2:26]\n"
                   "                op: NOT_EQUAL\n"
                   "              (exp)[2:29]\n"
                   "                type: string\n"
                   "                value: ___CREATE_ONLY___\n"
                   "        (block)[3:9]\n"
                   "          (functioncall)[3:9]\n"
                   "            (prefixexp)[3:9]\n"
                   "              type: var\n"
                   "              (var)[3:9]\n"
                   "                type: simple\n"
                   "                name: obj\n"
                   "            name: init\n"
                   "            (args)[3:17]\n"
                   "              (explist)[3:18]\n"
                   "                (exp)[3:18]\n"
                   "                  type: prefixexp\n"
                   "                  value: \n"
                   "                  (prefixexp)[3:18]\n"
                   "                    type: functioncall\n"
                   "                    (functioncall)[3:18]\n"
                   "                      (prefixexp)[3:18]\n"
                   "                        type: var\n"
                   "                        (var)[3:18]\n"
                   "                          type: simple\n"
                   "                          name: unpack\n"
                   "                      (args)[3:24]\n"
                   "                        (explist)[3:25]\n"
                   "                          (exp)[3:25]\n"
                   "                            type: prefixexp\n"
                   "                            value: \n"
                   "                            (prefixexp)[3:25]\n"
                   "                              type: var\n"
                   "                              (var)[3:25]\n"
                   "                                type: simple\n"
                   "                                name: arg\n"
                   "        (elseif)[3:9]\n"
                   "        (block)[5:9]\n"
                   "          (assign)[5:31]\n"
                   "            (varlist)[5:9]\n"
                   "              (var)[5:12]\n"
                   "                type: dot\n"
                   "                (prefixexp)[5:9]\n"
                   "                  type: var\n"
                   "                  (var)[5:9]\n"
                   "                    type: simple\n"
                   "                    name: obj\n"
                   "                name: ___CREATE_ONLY___\n"
                   "            (explist)[5:33]\n"
                   "              (exp)[5:33]\n"
                   "                type: true\n"
                   "                value: \n"
                   "    (elseif)[7:1]\n"
                   "      (exp)[7:8]\n"
                   "        type: prefixexp\n"
                   "        value: \n"
                   "        (prefixexp)[7:8]\n"
                   "          type: exp\n"
                   "          (exp)[7:9]\n"
                   "            type: binop\n"
                   "            value: \n"
                   "            (exp)[7:9]\n"
                   "              type: prefixexp\n"
                   "              value: \n"
                   "              (prefixexp)[7:9]\n"
                   "                type: functioncall\n"
                   "                (functioncall)[7:9]\n"
                   "                  (prefixexp)[7:9]\n"
                   "                    type: var\n"
                   "                    (var)[7:9]\n"
                   "                      type: simple\n"
                   "                      name: tostring\n"
                   "                  (args)[7:17]\n"
                   "                    (explist)[7:18]\n"
                   "                      (exp)[7:18]\n"
                   "                        type: prefixexp\n"
                   "                        value: \n"
                   "                        (prefixexp)[7:18]\n"
                   "                          type: var\n"
                   "                          (var)[7:21]\n"
                   "                            type: square\n"
                   "                            (prefixexp)[7:18]\n"
                   "                              type: var\n"
                   "                              (var)[7:18]\n"
                   "                                type: simple\n"
                   "                                name: arg\n"
                   "                            (exp)[7:22]\n"
                   "                              type: number\n"
                   "                              value: 1\n"
                   "            (binop)[7:26]\n"
                   "              op: NOT_EQUAL\n"
                   "            (exp)[7:29]\n"
                   "              type: string\n"
                   "              value: ___CREATE_ONLY___\n"
                   "      (block)[8:5]\n"
                   "        (functioncall)[8:5]\n"
                   "          (prefixexp)[8:5]\n"
                   "            type: var\n"
                   "            (var)[8:5]\n"
                   "              type: simple\n"
                   "              name: error\n"
                   "          (args)[8:10]\n"
                   "            (explist)[8:11]\n"
                   "              (exp)[8:11]\n"
                   "                type: binop\n"
                   "                value: \n"
                   "                (exp)[8:11]\n"
                   "                  type: string\n"
                   "                  value: No init method found for class \n"
                   "                (binop)[8:45]\n"
                   "                  op: CONCAT\n"
                   "                (exp)[8:48]\n"
                   "                  type: prefixexp\n"
                   "                  value: \n"
                   "                  (prefixexp)[8:48]\n"
                   "                    type: var\n"
                   "                    (var)[8:52]\n"
                   "                      type: dot\n"
                   "                      (prefixexp)[8:48]\n"
                   "                        type: var\n"
                   "                        (var)[8:48]\n"
                   "                          type: simple\n"
                   "                          name: self\n"
                   "                      name: name\n";

    ASSERT_EQ(dumpstr, wantstr);
}

TEST(syntax_tree, string) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    compiler c;
    auto result = c.compile_file("./test_string.lua");
    ASSERT_NE(result.chunk, nullptr);

    auto dumpstr = result.chunk->dump();
    LOG(INFO) << "\n" << dumpstr;

    auto wantstr = ""
                   "(block)[1:1]\n"
                   "  (assign)[1:3]\n"
                   "    (varlist)[1:1]\n"
                   "      (var)[1:1]\n"
                   "        type: simple\n"
                   "        name: a\n"
                   "    (explist)[1:5]\n"
                   "      (exp)[1:5]\n"
                   "        type: string\n"
                   "        value: a\n"
                   "  (assign)[2:3]\n"
                   "    (varlist)[2:1]\n"
                   "      (var)[2:1]\n"
                   "        type: simple\n"
                   "        name: b\n"
                   "    (explist)[2:5]\n"
                   "      (exp)[2:5]\n"
                   "        type: string\n"
                   "        value: b\n"
                   "  (assign)[3:3]\n"
                   "    (varlist)[3:1]\n"
                   "      (var)[3:1]\n"
                   "        type: simple\n"
                   "        name: c\n"
                   "    (explist)[3:5]\n"
                   "      (exp)[3:5]\n"
                   "        type: string\n"
                   "        value: 'c'\n"
                   "  (assign)[4:3]\n"
                   "    (varlist)[4:1]\n"
                   "      (var)[4:1]\n"
                   "        type: simple\n"
                   "        name: d\n"
                   "    (explist)[4:5]\n"
                   "      (exp)[4:5]\n"
                   "        type: string\n"
                   "        value: \"d\"\n"
                   "  (assign)[5:3]\n"
                   "    (varlist)[5:1]\n"
                   "      (var)[5:1]\n"
                   "        type: simple\n"
                   "        name: e\n"
                   "    (explist)[5:5]\n"
                   "      (exp)[5:5]\n"
                   "        type: string\n"
                   "        value: \"e\"\n"
                   "  (assign)[6:3]\n"
                   "    (varlist)[6:1]\n"
                   "      (var)[6:1]\n"
                   "        type: simple\n"
                   "        name: f\n"
                   "    (explist)[6:5]\n"
                   "      (exp)[6:5]\n"
                   "        type: string\n"
                   "        value: 'f'\n"
                   "  (assign)[7:3]\n"
                   "    (varlist)[7:1]\n"
                   "      (var)[7:1]\n"
                   "        type: simple\n"
                   "        name: g\n"
                   "    (explist)[7:5]\n"
                   "      (exp)[7:5]\n"
                   "        type: string\n"
                   "        value: g\n"
                   "g\n";

    ASSERT_EQ(dumpstr, wantstr);
}

TEST(syntax_tree, number) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    compiler c;
    auto result = c.compile_file("./test_number.lua");
    ASSERT_NE(result.chunk, nullptr);

    auto dumpstr = result.chunk->dump();
    LOG(INFO) << "\n" << dumpstr;

    auto wantstr = ""
                   "(block)[1:1]\n"
                   "  (assign)[1:2]\n"
                   "    (varlist)[1:1]\n"
                   "      (var)[1:1]\n"
                   "        type: simple\n"
                   "        name: a\n"
                   "    (explist)[1:3]\n"
                   "      (exp)[1:3]\n"
                   "        type: number\n"
                   "        value: 1\n"
                   "  (assign)[2:2]\n"
                   "    (varlist)[2:1]\n"
                   "      (var)[2:1]\n"
                   "        type: simple\n"
                   "        name: b\n"
                   "    (explist)[2:3]\n"
                   "      (exp)[2:3]\n"
                   "        type: true\n"
                   "        value: \n"
                   "  (assign)[3:2]\n"
                   "    (varlist)[3:1]\n"
                   "      (var)[3:1]\n"
                   "        type: simple\n"
                   "        name: c\n"
                   "    (explist)[3:3]\n"
                   "      (exp)[3:3]\n"
                   "        type: false\n"
                   "        value: \n"
                   "  (assign)[4:2]\n"
                   "    (varlist)[4:1]\n"
                   "      (var)[4:1]\n"
                   "        type: simple\n"
                   "        name: d\n"
                   "    (explist)[4:3]\n"
                   "      (exp)[4:3]\n"
                   "        type: nil\n"
                   "        value: \n"
                   "  (assign)[5:2]\n"
                   "    (varlist)[5:1]\n"
                   "      (var)[5:1]\n"
                   "        type: simple\n"
                   "        name: e\n"
                   "    (explist)[5:3]\n"
                   "      (exp)[5:3]\n"
                   "        type: number\n"
                   "        value: 1.123\n"
                   "  (assign)[6:2]\n"
                   "    (varlist)[6:1]\n"
                   "      (var)[6:1]\n"
                   "        type: simple\n"
                   "        name: f\n"
                   "    (explist)[6:3]\n"
                   "      (exp)[6:3]\n"
                   "        type: number\n"
                   "        value: -2.234\n"
                   "  (assign)[7:2]\n"
                   "    (varlist)[7:1]\n"
                   "      (var)[7:1]\n"
                   "        type: simple\n"
                   "        name: g\n"
                   "    (explist)[7:3]\n"
                   "      (exp)[7:3]\n"
                   "        type: number\n"
                   "        value: 1.1e10\n"
                   "  (assign)[8:2]\n"
                   "    (varlist)[8:1]\n"
                   "      (var)[8:1]\n"
                   "        type: simple\n"
                   "        name: h\n"
                   "    (explist)[8:3]\n"
                   "      (exp)[8:3]\n"
                   "        type: number\n"
                   "        value: -1.1e-10\n"
                   "  (assign)[9:2]\n"
                   "    (varlist)[9:1]\n"
                   "      (var)[9:1]\n"
                   "        type: simple\n"
                   "        name: i\n"
                   "    (explist)[9:3]\n"
                   "      (exp)[9:3]\n"
                   "        type: number\n"
                   "        value: 0xff\n"
                   "  (assign)[10:2]\n"
                   "    (varlist)[10:1]\n"
                   "      (var)[10:1]\n"
                   "        type: simple\n"
                   "        name: j\n"
                   "    (explist)[10:3]\n"
                   "      (exp)[10:3]\n"
                   "        type: number\n"
                   "        value: 0x1e.ff\n"
                   "  (assign)[11:2]\n"
                   "    (varlist)[11:1]\n"
                   "      (var)[11:1]\n"
                   "        type: simple\n"
                   "        name: k\n"
                   "    (explist)[11:3]\n"
                   "      (exp)[11:3]\n"
                   "        type: number\n"
                   "        value: -0x3.1ap-4\n";

    ASSERT_EQ(dumpstr, wantstr);
}

TEST(syntax_tree, for_num) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    compiler c;
    auto result = c.compile_file("./test_for_num.lua");
    ASSERT_NE(result.chunk, nullptr);

    auto dumpstr = result.chunk->dump();
    LOG(INFO) << "\n" << dumpstr;

    auto wantstr = ""
                   "(block)[1:1]\n"
                   "  (for_loop)[1:1]\n"
                   "    name: n\n"
                   "    (exp)[1:9]\n"
                   "      type: number\n"
                   "      value: 1\n"
                   "    (exp)[1:12]\n"
                   "      type: number\n"
                   "      value: 10\n"
                   "    (block)[2:5]\n"
                   "      (functioncall)[2:5]\n"
                   "        (prefixexp)[2:5]\n"
                   "          type: var\n"
                   "          (var)[2:5]\n"
                   "            type: simple\n"
                   "            name: print\n"
                   "        (args)[2:10]\n"
                   "          (explist)[2:11]\n"
                   "            (exp)[2:11]\n"
                   "              type: prefixexp\n"
                   "              value: \n"
                   "              (prefixexp)[2:11]\n"
                   "                type: var\n"
                   "                (var)[2:11]\n"
                   "                  type: simple\n"
                   "                  name: n\n"
                   "  (for_loop)[5:1]\n"
                   "    name: m\n"
                   "    (exp)[5:9]\n"
                   "      type: prefixexp\n"
                   "      value: \n"
                   "      (prefixexp)[5:9]\n"
                   "        type: functioncall\n"
                   "        (functioncall)[5:9]\n"
                   "          (prefixexp)[5:9]\n"
                   "            type: var\n"
                   "            (var)[5:9]\n"
                   "              type: simple\n"
                   "              name: ini\n"
                   "          (args)[5:12]\n"
                   "            empty\n"
                   "    (exp)[5:16]\n"
                   "      type: prefixexp\n"
                   "      value: \n"
                   "      (prefixexp)[5:16]\n"
                   "        type: var\n"
                   "        (var)[5:16]\n"
                   "          type: simple\n"
                   "          name: max\n"
                   "    (exp)[5:21]\n"
                   "      type: number\n"
                   "      value: 2\n"
                   "    (block)[6:5]\n"
                   "      (functioncall)[6:5]\n"
                   "        (prefixexp)[6:5]\n"
                   "          type: var\n"
                   "          (var)[6:5]\n"
                   "            type: simple\n"
                   "            name: print\n"
                   "        (args)[6:10]\n"
                   "          (explist)[6:11]\n"
                   "            (exp)[6:11]\n"
                   "              type: prefixexp\n"
                   "              value: \n"
                   "              (prefixexp)[6:11]\n"
                   "                type: var\n"
                   "                (var)[6:11]\n"
                   "                  type: simple\n"
                   "                  name: n\n";

    ASSERT_EQ(dumpstr, wantstr);
}

TEST(syntax_tree, for_in) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    compiler c;
    auto result = c.compile_file("./test_for_in.lua");
    ASSERT_NE(result.chunk, nullptr);

    auto dumpstr = result.chunk->dump();
    LOG(INFO) << "\n" << dumpstr;

    auto wantstr = ""
                   "(block)[1:1]\n"
                   "  (for_in)[1:1]\n"
                   "    (namelist)[1:5]\n"
                   "      name: key\n"
                   "      name: value\n"
                   "    (explist)[1:19]\n"
                   "      (exp)[1:19]\n"
                   "        type: prefixexp\n"
                   "        value: \n"
                   "        (prefixexp)[1:19]\n"
                   "          type: functioncall\n"
                   "          (functioncall)[1:19]\n"
                   "            (prefixexp)[1:19]\n"
                   "              type: var\n"
                   "              (var)[1:19]\n"
                   "                type: simple\n"
                   "                name: pairs\n"
                   "            (args)[1:24]\n"
                   "              (explist)[1:25]\n"
                   "                (exp)[1:25]\n"
                   "                  type: prefixexp\n"
                   "                  value: \n"
                   "                  (prefixexp)[1:25]\n"
                   "                    type: var\n"
                   "                    (var)[1:25]\n"
                   "                      type: simple\n"
                   "                      name: data\n"
                   "    (block)[2:5]\n"
                   "      (functioncall)[2:5]\n"
                   "        (prefixexp)[2:5]\n"
                   "          type: var\n"
                   "          (var)[2:5]\n"
                   "            type: simple\n"
                   "            name: print\n"
                   "        (args)[2:10]\n"
                   "          (explist)[2:11]\n"
                   "            (exp)[2:11]\n"
                   "              type: prefixexp\n"
                   "              value: \n"
                   "              (prefixexp)[2:11]\n"
                   "                type: var\n"
                   "                (var)[2:11]\n"
                   "                  type: simple\n"
                   "                  name: key\n"
                   "            (exp)[2:16]\n"
                   "              type: prefixexp\n"
                   "              value: \n"
                   "              (prefixexp)[2:16]\n"
                   "                type: var\n"
                   "                (var)[2:16]\n"
                   "                  type: simple\n"
                   "                  name: value\n"
                   "  (for_in)[5:1]\n"
                   "    (namelist)[5:5]\n"
                   "      name: key\n"
                   "    (explist)[5:12]\n"
                   "      (exp)[5:12]\n"
                   "        type: prefixexp\n"
                   "        value: \n"
                   "        (prefixexp)[5:12]\n"
                   "          type: functioncall\n"
                   "          (functioncall)[5:12]\n"
                   "            (prefixexp)[5:12]\n"
                   "              type: var\n"
                   "              (var)[5:12]\n"
                   "                type: simple\n"
                   "                name: pairs\n"
                   "            (args)[5:17]\n"
                   "              (explist)[5:18]\n"
                   "                (exp)[5:18]\n"
                   "                  type: prefixexp\n"
                   "                  value: \n"
                   "                  (prefixexp)[5:18]\n"
                   "                    type: var\n"
                   "                    (var)[5:18]\n"
                   "                      type: simple\n"
                   "                      name: data\n"
                   "    (block)[6:5]\n"
                   "      (assign)[6:15]\n"
                   "        (varlist)[6:5]\n"
                   "          (var)[6:9]\n"
                   "            type: square\n"
                   "            (prefixexp)[6:5]\n"
                   "              type: var\n"
                   "              (var)[6:5]\n"
                   "                type: simple\n"
                   "                name: data\n"
                   "            (exp)[6:10]\n"
                   "              type: prefixexp\n"
                   "              value: \n"
                   "              (prefixexp)[6:10]\n"
                   "                type: var\n"
                   "                (var)[6:10]\n"
                   "                  type: simple\n"
                   "                  name: key\n"
                   "        (explist)[6:17]\n"
                   "          (exp)[6:17]\n"
                   "            type: nil\n"
                   "            value: \n";

    ASSERT_EQ(dumpstr, wantstr);
}

TEST(syntax_tree, function) {

    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    compiler c;
    auto result = c.compile_file("./test_function.lua");
    ASSERT_NE(result.chunk, nullptr);

    auto dumpstr = result.chunk->dump();
    LOG(INFO) << "\n" << dumpstr;

    auto wantstr = ""
                   "(block)[1:1]\n"
                   "  (function)[1:1]\n"
                   "    (funcname)[1:10]\n"
                   "      (funcnamelist)[1:10]\n"
                   "        name: func_a\n"
                   "      colon_name: \n"
                   "    (funcbody)[1:16]\n"
                   "      (block)[1:17]\n"
                   "  (function)[4:1]\n"
                   "    (funcname)[4:10]\n"
                   "      (funcnamelist)[4:10]\n"
                   "        name: _G\n"
                   "        name: a\n"
                   "        name: b\n"
                   "      colon_name: func_b\n"
                   "    (funcbody)[4:23]\n"
                   "      (parlist)[4:24]\n"
                   "        (namelist)[4:24]\n"
                   "          name: a\n"
                   "          name: b\n"
                   "          name: c\n"
                   "        var_params: 1\n"
                   "      (block)[5:5]\n"
                   "        (return)[5:5]\n"
                   "          (explist)[5:12]\n"
                   "            (exp)[5:12]\n"
                   "              type: binop\n"
                   "              value: \n"
                   "              (exp)[5:12]\n"
                   "                type: prefixexp\n"
                   "                value: \n"
                   "                (prefixexp)[5:12]\n"
                   "                  type: var\n"
                   "                  (var)[5:12]\n"
                   "                    type: simple\n"
                   "                    name: a\n"
                   "              (binop)[5:14]\n"
                   "                op: PLUS\n"
                   "              (exp)[5:16]\n"
                   "                type: binop\n"
                   "                value: \n"
                   "                (exp)[5:16]\n"
                   "                  type: prefixexp\n"
                   "                  value: \n"
                   "                  (prefixexp)[5:16]\n"
                   "                    type: var\n"
                   "                    (var)[5:16]\n"
                   "                      type: simple\n"
                   "                      name: b\n"
                   "                (binop)[5:18]\n"
                   "                  op: PLUS\n"
                   "                (exp)[5:20]\n"
                   "                  type: prefixexp\n"
                   "                  value: \n"
                   "                  (prefixexp)[5:20]\n"
                   "                    type: var\n"
                   "                    (var)[5:20]\n"
                   "                      type: simple\n"
                   "                      name: c\n"
                   "  (local_function)[8:1]\n"
                   "    name: func_c\n"
                   "    (funcbody)[8:22]\n"
                   "      (parlist)[8:23]\n"
                   "        var_params: 1\n"
                   "      (block)[9:5]\n"
                   "        (local_var)[9:5]\n"
                   "          (namelist)[9:11]\n"
                   "            name: param_list\n"
                   "          (explist)[9:24]\n"
                   "            (exp)[9:24]\n"
                   "              type: tableconstructor\n"
                   "              value: \n"
                   "              (tableconstructor)[9:24]\n"
                   "                (fieldlist)[9:25]\n"
                   "                  (field)[9:25]\n"
                   "                    type: array\n"
                   "                    (exp)[9:25]\n"
                   "                      type: var_params\n"
                   "                      value: \n"
                   "        (for_loop)[10:5]\n"
                   "          name: i\n"
                   "          (exp)[10:13]\n"
                   "            type: number\n"
                   "            value: 1\n"
                   "          (exp)[10:16]\n"
                   "            type: unop\n"
                   "            value: \n"
                   "            (unop)[10:16]\n"
                   "              op: NUMBER_SIGN\n"
                   "            (exp)[10:17]\n"
                   "              type: prefixexp\n"
                   "              value: \n"
                   "              (prefixexp)[10:17]\n"
                   "                type: var\n"
                   "                (var)[10:17]\n"
                   "                  type: simple\n"
                   "                  name: param_list\n"
                   "          (block)[11:9]\n"
                   "            (functioncall)[11:9]\n"
                   "              (prefixexp)[11:9]\n"
                   "                type: var\n"
                   "                (var)[11:9]\n"
                   "                  type: simple\n"
                   "                  name: print\n"
                   "              (args)[11:14]\n"
                   "                (explist)[11:15]\n"
                   "                  (exp)[11:15]\n"
                   "                    type: prefixexp\n"
                   "                    value: \n"
                   "                    (prefixexp)[11:15]\n"
                   "                      type: var\n"
                   "                      (var)[11:25]\n"
                   "                        type: square\n"
                   "                        (prefixexp)[11:15]\n"
                   "                          type: var\n"
                   "                          (var)[11:15]\n"
                   "                            type: simple\n"
                   "                            name: param_list\n"
                   "                        (exp)[11:26]\n"
                   "                          type: prefixexp\n"
                   "                          value: \n"
                   "                          (prefixexp)[11:26]\n"
                   "                            type: var\n"
                   "                            (var)[11:26]\n"
                   "                              type: simple\n"
                   "                              name: i\n";

    ASSERT_EQ(dumpstr, wantstr);
}
