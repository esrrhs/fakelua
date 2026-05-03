#include "compile/compiler.h"
#include "fakelua.h"
#include "gtest/gtest.h"

using namespace fakelua;

TEST(syntax_tree, CompileString) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();
    ASSERT_NE(s, nullptr);

    Compiler c(s);
    const auto chunk = c.CompileString("a = 1", {true}).chunk;
    ASSERT_NE(chunk, nullptr);

    auto dumpstr = chunk->Dump(0);
    LOG_INFO("{}", dumpstr);

    const auto wantstr = ""
                         "(block)[1:1]\n"
                         "  (assign)[1:3]\n"
                         "    (varlist)[1:1]\n"
                         "      (var)[1:1]\n"
                         "        type: simple\n"
                         "        name: a\n"
                         "    (explist)[1:5]\n"
                         "      (exp)[1:5]\n"
                         "        type: number\n"
                         "        value: 1\n";

    ASSERT_EQ(dumpstr, wantstr);
}

TEST(syntax_tree, label) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();
    ASSERT_NE(s, nullptr);

    Compiler c(s);
    const auto chunk = c.CompileFile("./syntax/test_label.lua", {true}).chunk;
    ASSERT_NE(chunk, nullptr);

    auto dumpstr = chunk->Dump(0);
    LOG_INFO("{}", dumpstr);

    const auto wantstr = ""
                         "(block)[2:1]\n"
                         "  aa(label)[2:3]\n"
                         "  bb(label)[4:3]\n";

    ASSERT_EQ(dumpstr, wantstr);
}

TEST(syntax_tree, assign_simple) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();
    ASSERT_NE(s, nullptr);

    Compiler c(s);
    const auto chunk = c.CompileFile("./syntax/test_assign_simple.lua", {true}).chunk;
    ASSERT_NE(chunk, nullptr);

    auto dumpstr = chunk->Dump(0);
    LOG_INFO("{}", dumpstr);

    const auto wantstr = ""
                         "(block)[6:1]\n"
                         "  (assign)[6:3]\n"
                         "    (varlist)[6:1]\n"
                         "      (var)[6:1]\n"
                         "        type: simple\n"
                         "        name: a\n"
                         "    (explist)[6:5]\n"
                         "      (exp)[6:5]\n"
                         "        type: number\n"
                         "        value: 1\n";

    ASSERT_EQ(dumpstr, wantstr);
}

TEST(syntax_tree, assign) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();
    ASSERT_NE(s, nullptr);

    Compiler c(s);
    const auto chunk = c.CompileFile("./syntax/test_assign.lua", {true}).chunk;
    ASSERT_NE(chunk, nullptr);

    auto dumpstr = chunk->Dump(0);
    LOG_INFO("{}", dumpstr);

    const auto wantstr = ""
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
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();
    ASSERT_NE(s, nullptr);

    Compiler c(s);
    const auto chunk = c.CompileFile("./syntax/test_function_call.lua", {true}).chunk;
    ASSERT_NE(chunk, nullptr);

    auto dumpstr = chunk->Dump(0);
    LOG_INFO("{}", dumpstr);

    const auto wantstr = ""
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
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();
    ASSERT_NE(s, nullptr);

    Compiler c(s);
    const auto chunk = c.CompileFile("./syntax/test_break.lua", {true}).chunk;
    ASSERT_NE(chunk, nullptr);

    auto dumpstr = chunk->Dump(0);
    LOG_INFO("{}", dumpstr);

    const auto wantstr = ""
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
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();
    ASSERT_NE(s, nullptr);

    Compiler c(s);
    const auto chunk = c.CompileFile("./syntax/test_continue.lua", {true}).chunk;
    ASSERT_NE(chunk, nullptr);

    auto dumpstr = chunk->Dump(0);
    LOG_INFO("{}", dumpstr);

    const auto wantstr = ""
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
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();
    ASSERT_NE(s, nullptr);

    Compiler c(s);
    const auto chunk = c.CompileFile("./syntax/test_do_end.lua", {true}).chunk;
    ASSERT_NE(chunk, nullptr);

    auto dumpstr = chunk->Dump(0);
    LOG_INFO("{}", dumpstr);

    const auto wantstr = ""
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
                         "            (tableconstructor)[1:21]\n";

    ASSERT_EQ(dumpstr, wantstr);
}

TEST(syntax_tree, while) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();
    ASSERT_NE(s, nullptr);

    Compiler c(s);
    const auto chunk = c.CompileFile("./syntax/test_while.lua", {true}).chunk;
    ASSERT_NE(chunk, nullptr);

    auto dumpstr = chunk->Dump(0);
    LOG_INFO("{}", dumpstr);

    const auto wantstr = ""
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
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();
    ASSERT_NE(s, nullptr);

    Compiler c(s);
    const auto chunk = c.CompileFile("./syntax/test_repeat.lua", {true}).chunk;
    ASSERT_NE(chunk, nullptr);

    auto dumpstr = chunk->Dump(0);
    LOG_INFO("{}", dumpstr);

    const auto wantstr = ""
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
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();
    ASSERT_NE(s, nullptr);

    Compiler c(s);
    const auto chunk = c.CompileFile("./syntax/test_if.lua", {true}).chunk;
    ASSERT_NE(chunk, nullptr);

    auto dumpstr = chunk->Dump(0);
    LOG_INFO("{}", dumpstr);

    const auto wantstr = ""
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
                         "        (elseif)[2:54]\n"
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
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();
    ASSERT_NE(s, nullptr);

    Compiler c(s);
    const auto chunk = c.CompileFile("./syntax/test_string.lua", {true}).chunk;
    ASSERT_NE(chunk, nullptr);

    auto dumpstr = chunk->Dump(0);
    LOG_INFO("{}", dumpstr);

    const auto wantstr = ""
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
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();
    ASSERT_NE(s, nullptr);

    Compiler c(s);
    const auto chunk = c.CompileFile("./syntax/test_number.lua", {true}).chunk;
    ASSERT_NE(chunk, nullptr);

    auto dumpstr = chunk->Dump(0);
    LOG_INFO("{}", dumpstr);

    const auto wantstr = ""
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
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();
    ASSERT_NE(s, nullptr);

    Compiler c(s);
    const auto chunk = c.CompileFile("./syntax/test_for_num.lua", {true}).chunk;
    ASSERT_NE(chunk, nullptr);

    auto dumpstr = chunk->Dump(0);
    LOG_INFO("{}", dumpstr);

    const auto wantstr = ""
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
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();
    ASSERT_NE(s, nullptr);

    Compiler c(s);
    const auto chunk = c.CompileFile("./syntax/test_for_in.lua", {true}).chunk;
    ASSERT_NE(chunk, nullptr);

    auto dumpstr = chunk->Dump(0);
    LOG_INFO("{}", dumpstr);

    const auto wantstr = ""
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
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();
    ASSERT_NE(s, nullptr);

    Compiler c(s);
    const auto chunk = c.CompileFile("./syntax/test_function.lua", {true}).chunk;
    ASSERT_NE(chunk, nullptr);

    auto dumpstr = chunk->Dump(0);
    LOG_INFO("{}", dumpstr);

    const auto wantstr = "(block)[1:1]\n"
                         "  (function)[1:1]\n"
                         "    (funcname)[1:10]\n"
                         "      (funcnamelist)[1:10]\n"
                         "        name: func_a\n"
                         "      ColonName: \n"
                         "    (funcbody)[1:16]\n"
                         "      (block)[1:17]\n"
                         "  (function)[4:1]\n"
                         "    (funcname)[4:10]\n"
                         "      (funcnamelist)[4:10]\n"
                         "        name: _G\n"
                         "        name: a\n"
                         "        name: b\n"
                         "      ColonName: func_b\n"
                         "    (funcbody)[4:23]\n"
                         "      (parlist)[4:24]\n"
                         "        (namelist)[4:24]\n"
                         "          name: a\n"
                         "          name: b\n"
                         "          name: c\n"
                         "        VarParams: 1\n"
                         "      (block)[5:5]\n"
                         "        (return)[5:5]\n"
                         "          (explist)[5:12]\n"
                         "            (exp)[5:12]\n"
                         "              type: binop\n"
                         "              value: \n"
                         "              (exp)[5:12]\n"
                         "                type: binop\n"
                         "                value: \n"
                         "                (exp)[5:12]\n"
                         "                  type: prefixexp\n"
                         "                  value: \n"
                         "                  (prefixexp)[5:12]\n"
                         "                    type: var\n"
                         "                    (var)[5:12]\n"
                         "                      type: simple\n"
                         "                      name: a\n"
                         "                (binop)[5:14]\n"
                         "                  op: PLUS\n"
                         "                (exp)[5:16]\n"
                         "                  type: prefixexp\n"
                         "                  value: \n"
                         "                  (prefixexp)[5:16]\n"
                         "                    type: var\n"
                         "                    (var)[5:16]\n"
                         "                      type: simple\n"
                         "                      name: b\n"
                         "              (binop)[5:18]\n"
                         "                op: PLUS\n"
                         "              (exp)[5:20]\n"
                         "                type: prefixexp\n"
                         "                value: \n"
                         "                (prefixexp)[5:20]\n"
                         "                  type: var\n"
                         "                  (var)[5:20]\n"
                         "                    type: simple\n"
                         "                    name: c\n"
                         "  (local_function)[8:1]\n"
                         "    name: func_c\n"
                         "    (funcbody)[8:22]\n"
                         "      (parlist)[8:23]\n"
                         "        VarParams: 1\n"
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
                         "                      type: VarParams\n"
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

TEST(syntax_tree, var) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();
    ASSERT_NE(s, nullptr);

    Compiler c(s);
    const auto chunk = c.CompileFile("./syntax/test_var.lua", {true}).chunk;
    ASSERT_NE(chunk, nullptr);

    auto dumpstr = chunk->Dump(0);
    LOG_INFO("{}", dumpstr);

    const auto wantstr = ""
                         "(block)[1:1]\n"
                         "  (local_var)[1:1]\n"
                         "    (namelist)[1:7]\n"
                         "      name: any\n"
                         "  (local_var)[2:1]\n"
                         "    (namelist)[2:7]\n"
                         "      name: a\n"
                         "      name: b\n"
                         "      name: c\n"
                         "    (explist)[2:17]\n"
                         "      (exp)[2:17]\n"
                         "        type: number\n"
                         "        value: 1\n"
                         "      (exp)[2:20]\n"
                         "        type: string\n"
                         "        value: 2\n"
                         "      (exp)[2:25]\n"
                         "        type: prefixexp\n"
                         "        value: \n"
                         "        (prefixexp)[2:25]\n"
                         "          type: functioncall\n"
                         "          (functioncall)[2:25]\n"
                         "            (prefixexp)[2:25]\n"
                         "              type: var\n"
                         "              (var)[2:25]\n"
                         "                type: simple\n"
                         "                name: func\n"
                         "            (args)[2:29]\n"
                         "              empty\n";

    ASSERT_EQ(dumpstr, wantstr);
}

TEST(syntax_tree, var_attr) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();
    ASSERT_NE(s, nullptr);

    Compiler c(s);
    const auto chunk = c.CompileFile("./syntax/test_var_attr.lua", {true}).chunk;
    ASSERT_NE(chunk, nullptr);

    auto dumpstr = chunk->Dump(0);
    LOG_INFO("{}", dumpstr);

    const auto wantstr = ""
                         "(block)[1:1]\n"
                         "  (local_var)[1:1]\n"
                         "    (namelist)[1:7]\n"
                         "      name: any\n"
                         "      attrib: const\n"
                         "  (local_var)[2:1]\n"
                         "    (namelist)[2:7]\n"
                         "      name: a\n"
                         "      name: b\n"
                         "      attrib: const\n"
                         "      name: c\n"
                         "    (explist)[2:25]\n"
                         "      (exp)[2:25]\n"
                         "        type: number\n"
                         "        value: 1\n"
                         "      (exp)[2:28]\n"
                         "        type: string\n"
                         "        value: 2\n"
                         "      (exp)[2:33]\n"
                         "        type: prefixexp\n"
                         "        value: \n"
                         "        (prefixexp)[2:33]\n"
                         "          type: functioncall\n"
                         "          (functioncall)[2:33]\n"
                         "            (prefixexp)[2:33]\n"
                         "              type: var\n"
                         "              (var)[2:33]\n"
                         "                type: simple\n"
                         "                name: func\n"
                         "            (args)[2:37]\n"
                         "              empty\n";

    ASSERT_EQ(dumpstr, wantstr);
}

TEST(syntax_tree, function_call_args) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();
    ASSERT_NE(s, nullptr);

    Compiler c(s);
    const auto chunk = c.CompileFile("./syntax/test_function_call_args.lua", {true}).chunk;
    ASSERT_NE(chunk, nullptr);

    auto dumpstr = chunk->Dump(0);
    LOG_INFO("{}", dumpstr);

    const auto wantstr = ""
                         "(block)[2:1]\n"
                         "  (functioncall)[2:1]\n"
                         "    (prefixexp)[2:1]\n"
                         "      type: var\n"
                         "      (var)[2:1]\n"
                         "        type: simple\n"
                         "        name: a\n"
                         "    (args)[2:3]\n"
                         "      (tableconstructor)[2:3]\n"
                         "        (fieldlist)[2:4]\n"
                         "          (field)[2:4]\n"
                         "            type: object\n"
                         "            name: b\n"
                         "            (exp)[2:6]\n"
                         "              type: number\n"
                         "              value: 1\n"
                         "  (functioncall)[3:1]\n"
                         "    (prefixexp)[3:1]\n"
                         "      type: var\n"
                         "      (var)[3:1]\n"
                         "        type: simple\n"
                         "        name: c\n"
                         "    (args)[3:3]\n"
                         "      (exp)[3:3]\n"
                         "        type: string\n"
                         "        value: d\n";

    ASSERT_EQ(dumpstr, wantstr);
}

TEST(syntax_tree, constructor) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();
    ASSERT_NE(s, nullptr);

    Compiler c(s);
    const auto chunk = c.CompileFile("./syntax/test_constructor.lua", {true}).chunk;
    ASSERT_NE(chunk, nullptr);

    auto dumpstr = chunk->Dump(0);
    LOG_INFO("{}", dumpstr);

    const auto wantstr = "(block)[2:1]\n"
                         "  (assign)[2:3]\n"
                         "    (varlist)[2:1]\n"
                         "      (var)[2:1]\n"
                         "        type: simple\n"
                         "        name: a\n"
                         "    (explist)[2:5]\n"
                         "      (exp)[2:5]\n"
                         "        type: tableconstructor\n"
                         "        value: \n"
                         "        (tableconstructor)[2:5]\n"
                         "          (fieldlist)[3:5]\n"
                         "            (field)[3:5]\n"
                         "              type: array\n"
                         "              (exp)[3:6]\n"
                         "                type: number\n"
                         "                value: 2\n"
                         "              (exp)[3:11]\n"
                         "                type: number\n"
                         "                value: 1\n"
                         "            (field)[4:5]\n"
                         "              type: object\n"
                         "              name: a\n"
                         "              (exp)[4:9]\n"
                         "                type: number\n"
                         "                value: 2\n";

    ASSERT_EQ(dumpstr, wantstr);
}

TEST(syntax_tree, function_exp) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();
    ASSERT_NE(s, nullptr);

    Compiler c(s);
    const auto chunk = c.CompileFile("./syntax/test_function_exp.lua", {true}).chunk;
    ASSERT_NE(chunk, nullptr);

    auto dumpstr = chunk->Dump(0);
    LOG_INFO("{}", dumpstr);

    const auto wantstr = "(block)[2:1]\n"
                         "  (assign)[2:3]\n"
                         "    (varlist)[2:1]\n"
                         "      (var)[2:1]\n"
                         "        type: simple\n"
                         "        name: a\n"
                         "    (explist)[2:5]\n"
                         "      (exp)[2:5]\n"
                         "        type: functiondef\n"
                         "        value: \n"
                         "        (functiondef)[2:5]\n"
                         "          (funcbody)[2:13]\n"
                         "            (parlist)[2:14]\n"
                         "              (namelist)[2:14]\n"
                         "                name: b\n"
                         "            (block)[2:15]\n";

    ASSERT_EQ(dumpstr, wantstr);
}

TEST(syntax_tree, binop) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();
    ASSERT_NE(s, nullptr);

    Compiler c(s);
    const auto chunk = c.CompileFile("./syntax/test_binop.lua", {true}).chunk;
    ASSERT_NE(chunk, nullptr);

    auto dumpstr = chunk->Dump(0);
    LOG_INFO("{}", dumpstr);

    const auto wantstr = "(block)[2:1]\n"
                         "  (assign)[2:3]\n"
                         "    (varlist)[2:1]\n"
                         "      (var)[2:1]\n"
                         "        type: simple\n"
                         "        name: a\n"
                         "    (explist)[2:5]\n"
                         "      (exp)[2:5]\n"
                         "        type: binop\n"
                         "        value: \n"
                         "        (exp)[2:5]\n"
                         "          type: prefixexp\n"
                         "          value: \n"
                         "          (prefixexp)[2:5]\n"
                         "            type: exp\n"
                         "            (exp)[2:6]\n"
                         "              type: binop\n"
                         "              value: \n"
                         "              (exp)[2:6]\n"
                         "                type: prefixexp\n"
                         "                value: \n"
                         "                (prefixexp)[2:6]\n"
                         "                  type: var\n"
                         "                  (var)[2:6]\n"
                         "                    type: simple\n"
                         "                    name: b\n"
                         "              (binop)[2:8]\n"
                         "                op: POW\n"
                         "              (exp)[2:10]\n"
                         "                type: prefixexp\n"
                         "                value: \n"
                         "                (prefixexp)[2:10]\n"
                         "                  type: var\n"
                         "                  (var)[2:10]\n"
                         "                    type: simple\n"
                         "                    name: c\n"
                         "        (binop)[2:13]\n"
                         "          op: XOR\n"
                         "        (exp)[2:15]\n"
                         "          type: prefixexp\n"
                         "          value: \n"
                         "          (prefixexp)[2:15]\n"
                         "            type: var\n"
                         "            (var)[2:15]\n"
                         "              type: simple\n"
                         "              name: d\n";

    ASSERT_EQ(dumpstr, wantstr);
}

TEST(syntax_tree, test_binop_order1) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();
    ASSERT_NE(s, nullptr);

    Compiler c(s);
    const auto chunk = c.CompileFile("./syntax/test_binop_order1.lua", {true}).chunk;
    ASSERT_NE(chunk, nullptr);

    auto dumpstr = chunk->Dump(0);
    LOG_INFO("{}", dumpstr);

    const auto wantstr = "(block)[2:1]\n"
                         "  (assign)[2:3]\n"
                         "    (varlist)[2:1]\n"
                         "      (var)[2:1]\n"
                         "        type: simple\n"
                         "        name: a\n"
                         "    (explist)[2:5]\n"
                         "      (exp)[2:5]\n"
                         "        type: binop\n"
                         "        value: \n"
                         "        (exp)[2:5]\n"
                         "          type: binop\n"
                         "          value: \n"
                         "          (exp)[2:5]\n"
                         "            type: prefixexp\n"
                         "            value: \n"
                         "            (prefixexp)[2:5]\n"
                         "              type: var\n"
                         "              (var)[2:5]\n"
                         "                type: simple\n"
                         "                name: b\n"
                         "          (binop)[2:7]\n"
                         "            op: SLASH\n"
                         "          (exp)[2:9]\n"
                         "            type: number\n"
                         "            value: 2\n"
                         "        (binop)[2:11]\n"
                         "          op: PLUS\n"
                         "        (exp)[2:13]\n"
                         "          type: number\n"
                         "          value: 1\n";

    ASSERT_EQ(dumpstr, wantstr);
}

TEST(syntax_tree, test_binop_order2) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();
    ASSERT_NE(s, nullptr);

    Compiler c(s);
    const auto chunk = c.CompileFile("./syntax/test_binop_order2.lua", {true}).chunk;
    ASSERT_NE(chunk, nullptr);

    auto dumpstr = chunk->Dump(0);
    LOG_INFO("{}", dumpstr);

    const auto wantstr = "(block)[2:1]\n"
                         "  (assign)[2:3]\n"
                         "    (varlist)[2:1]\n"
                         "      (var)[2:1]\n"
                         "        type: simple\n"
                         "        name: a\n"
                         "    (explist)[2:5]\n"
                         "      (exp)[2:5]\n"
                         "        type: binop\n"
                         "        value: \n"
                         "        (exp)[2:5]\n"
                         "          type: binop\n"
                         "          value: \n"
                         "          (exp)[2:5]\n"
                         "            type: prefixexp\n"
                         "            value: \n"
                         "            (prefixexp)[2:5]\n"
                         "              type: var\n"
                         "              (var)[2:5]\n"
                         "                type: simple\n"
                         "                name: a\n"
                         "          (binop)[2:7]\n"
                         "            op: PLUS\n"
                         "          (exp)[2:9]\n"
                         "            type: prefixexp\n"
                         "            value: \n"
                         "            (prefixexp)[2:9]\n"
                         "              type: var\n"
                         "              (var)[2:9]\n"
                         "                type: simple\n"
                         "                name: i\n"
                         "        (binop)[2:11]\n"
                         "          op: LESS\n"
                         "        (exp)[2:13]\n"
                         "          type: binop\n"
                         "          value: \n"
                         "          (exp)[2:13]\n"
                         "            type: binop\n"
                         "            value: \n"
                         "            (exp)[2:13]\n"
                         "              type: prefixexp\n"
                         "              value: \n"
                         "              (prefixexp)[2:13]\n"
                         "                type: var\n"
                         "                (var)[2:13]\n"
                         "                  type: simple\n"
                         "                  name: b\n"
                         "            (binop)[2:15]\n"
                         "              op: SLASH\n"
                         "            (exp)[2:17]\n"
                         "              type: number\n"
                         "              value: 2\n"
                         "          (binop)[2:19]\n"
                         "            op: PLUS\n"
                         "          (exp)[2:21]\n"
                         "            type: number\n"
                         "            value: 1\n";

    ASSERT_EQ(dumpstr, wantstr);
}

TEST(syntax_tree, test_binop_order3) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();
    ASSERT_NE(s, nullptr);

    Compiler c(s);
    const auto chunk = c.CompileFile("./syntax/test_binop_order3.lua", {true}).chunk;
    ASSERT_NE(chunk, nullptr);

    auto dumpstr = chunk->Dump(0);
    LOG_INFO("{}", dumpstr);

    const auto wantstr = "(block)[2:1]\n"
                         "  (assign)[2:3]\n"
                         "    (varlist)[2:1]\n"
                         "      (var)[2:1]\n"
                         "        type: simple\n"
                         "        name: a\n"
                         "    (explist)[2:5]\n"
                         "      (exp)[2:5]\n"
                         "        type: binop\n"
                         "        value: \n"
                         "        (exp)[2:5]\n"
                         "          type: number\n"
                         "          value: 5\n"
                         "        (binop)[2:7]\n"
                         "          op: PLUS\n"
                         "        (exp)[2:9]\n"
                         "          type: binop\n"
                         "          value: \n"
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
                         "                  name: x\n"
                         "            (binop)[2:11]\n"
                         "              op: POW\n"
                         "            (exp)[2:13]\n"
                         "              type: number\n"
                         "              value: 2\n"
                         "          (binop)[2:15]\n"
                         "            op: STAR\n"
                         "          (exp)[2:17]\n"
                         "            type: number\n"
                         "            value: 8\n";

    ASSERT_EQ(dumpstr, wantstr);
}

TEST(syntax_tree, test_binop_order4) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();
    ASSERT_NE(s, nullptr);

    Compiler c(s);
    const auto chunk = c.CompileFile("./syntax/test_binop_order4.lua", {true}).chunk;
    ASSERT_NE(chunk, nullptr);

    auto dumpstr = chunk->Dump(0);
    LOG_INFO("{}", dumpstr);

    const auto wantstr = "(block)[2:1]\n"
                         "  (assign)[2:3]\n"
                         "    (varlist)[2:1]\n"
                         "      (var)[2:1]\n"
                         "        type: simple\n"
                         "        name: a\n"
                         "    (explist)[2:5]\n"
                         "      (exp)[2:5]\n"
                         "        type: binop\n"
                         "        value: \n"
                         "        (exp)[2:5]\n"
                         "          type: binop\n"
                         "          value: \n"
                         "          (exp)[2:5]\n"
                         "            type: prefixexp\n"
                         "            value: \n"
                         "            (prefixexp)[2:5]\n"
                         "              type: var\n"
                         "              (var)[2:5]\n"
                         "                type: simple\n"
                         "                name: a\n"
                         "          (binop)[2:7]\n"
                         "            op: LESS\n"
                         "          (exp)[2:9]\n"
                         "            type: prefixexp\n"
                         "            value: \n"
                         "            (prefixexp)[2:9]\n"
                         "              type: var\n"
                         "              (var)[2:9]\n"
                         "                type: simple\n"
                         "                name: y\n"
                         "        (binop)[2:11]\n"
                         "          op: AND\n"
                         "        (exp)[2:15]\n"
                         "          type: binop\n"
                         "          value: \n"
                         "          (exp)[2:15]\n"
                         "            type: prefixexp\n"
                         "            value: \n"
                         "            (prefixexp)[2:15]\n"
                         "              type: var\n"
                         "              (var)[2:15]\n"
                         "                type: simple\n"
                         "                name: y\n"
                         "          (binop)[2:17]\n"
                         "            op: LESS_EQUAL\n"
                         "          (exp)[2:20]\n"
                         "            type: prefixexp\n"
                         "            value: \n"
                         "            (prefixexp)[2:20]\n"
                         "              type: var\n"
                         "              (var)[2:20]\n"
                         "                type: simple\n"
                         "                name: z\n";

    ASSERT_EQ(dumpstr, wantstr);
}

TEST(syntax_tree, test_binop_order5) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();
    ASSERT_NE(s, nullptr);

    Compiler c(s);
    const auto chunk = c.CompileFile("./syntax/test_binop_order5.lua", {true}).chunk;
    ASSERT_NE(chunk, nullptr);

    auto dumpstr = chunk->Dump(0);
    LOG_INFO("{}", dumpstr);

    const auto wantstr = "(block)[1:1]\n"
                         "  (assign)[1:2]\n"
                         "    (varlist)[1:1]\n"
                         "      (var)[1:1]\n"
                         "        type: simple\n"
                         "        name: a\n"
                         "    (explist)[1:3]\n"
                         "      (exp)[1:3]\n"
                         "        type: unop\n"
                         "        value: \n"
                         "        (unop)[1:3]\n"
                         "          op: MINUS\n"
                         "        (exp)[1:4]\n"
                         "          type: binop\n"
                         "          value: \n"
                         "          (exp)[1:4]\n"
                         "            type: prefixexp\n"
                         "            value: \n"
                         "            (prefixexp)[1:4]\n"
                         "              type: var\n"
                         "              (var)[1:4]\n"
                         "                type: simple\n"
                         "                name: x\n"
                         "          (binop)[1:5]\n"
                         "            op: POW\n"
                         "          (exp)[1:6]\n"
                         "            type: number\n"
                         "            value: 2\n";

    ASSERT_EQ(dumpstr, wantstr);
}

TEST(syntax_tree, test_binop_order6) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();
    ASSERT_NE(s, nullptr);

    Compiler c(s);
    const auto chunk = c.CompileFile("./syntax/test_binop_order6.lua", {true}).chunk;
    ASSERT_NE(chunk, nullptr);

    auto dumpstr = chunk->Dump(0);
    LOG_INFO("{}", dumpstr);

    const auto wantstr = "(block)[1:1]\n"
                         "  (assign)[1:2]\n"
                         "    (varlist)[1:1]\n"
                         "      (var)[1:1]\n"
                         "        type: simple\n"
                         "        name: a\n"
                         "    (explist)[1:3]\n"
                         "      (exp)[1:3]\n"
                         "        type: binop\n"
                         "        value: \n"
                         "        (exp)[1:3]\n"
                         "          type: prefixexp\n"
                         "          value: \n"
                         "          (prefixexp)[1:3]\n"
                         "            type: var\n"
                         "            (var)[1:3]\n"
                         "              type: simple\n"
                         "              name: x\n"
                         "        (binop)[1:4]\n"
                         "          op: POW\n"
                         "        (exp)[1:5]\n"
                         "          type: binop\n"
                         "          value: \n"
                         "          (exp)[1:5]\n"
                         "            type: prefixexp\n"
                         "            value: \n"
                         "            (prefixexp)[1:5]\n"
                         "              type: var\n"
                         "              (var)[1:5]\n"
                         "                type: simple\n"
                         "                name: y\n"
                         "          (binop)[1:6]\n"
                         "            op: POW\n"
                         "          (exp)[1:7]\n"
                         "            type: prefixexp\n"
                         "            value: \n"
                         "            (prefixexp)[1:7]\n"
                         "              type: var\n"
                         "              (var)[1:7]\n"
                         "                type: simple\n"
                         "                name: z\n";

    ASSERT_EQ(dumpstr, wantstr);
}

TEST(syntax_tree, test_empty) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();
    ASSERT_NE(s, nullptr);

    Compiler c(s);
    const auto chunk = c.CompileFile("./syntax/test_empty.lua", {true}).chunk;
    ASSERT_NE(chunk, nullptr);

    auto dumpstr = chunk->Dump(0);
    LOG_INFO("{}", dumpstr);

    const auto wantstr = "(block)[1:1]\n"
                         "  (empty)[1:1]\n"
                         "  (empty)[2:1]\n"
                         "  (empty)[3:1]\n";

    ASSERT_EQ(dumpstr, wantstr);
}

// compiler.cpp line 46: parse failure throws when the Lua source is syntactically invalid.
TEST(syntax_tree, parse_failure_throws) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();
    ASSERT_NE(s, nullptr);

    Compiler c(s);
    EXPECT_THROW(c.CompileString("function @invalid", {}), std::exception);
}

// preprocessor.cpp line 81: var count != exp count in assignment.
TEST(syntax_tree, preprocess_assign_count_mismatch) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();
    ASSERT_NE(s, nullptr);

    Compiler c(s);
    EXPECT_THROW(c.CompileString("a, b = 1", {}), std::exception);
}

// preprocessor.cpp line 246: colon method definition is not supported.
TEST(syntax_tree, preprocess_colon_method_unsupported) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();
    ASSERT_NE(s, nullptr);

    Compiler c(s);
    EXPECT_THROW(c.CompileString("function a:b() end", {}), std::exception);
}

// preprocessor.cpp line 250: multi-part function name is not supported.
TEST(syntax_tree, preprocess_multipart_funcname_unsupported) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();
    ASSERT_NE(s, nullptr);

    Compiler c(s);
    EXPECT_THROW(c.CompileString("function a.b.c() end", {}), std::exception);
}
