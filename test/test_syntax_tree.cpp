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

TEST(syntax_tree, continue1) {
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
                   "                value: \"2\"\n"
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
                   "                value: \"___CREATE_ONLY___\"\n"
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
                   "              value: \"___CREATE_ONLY___\"\n"
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
                   "                  value: \"No init method found for class \"\n"
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
