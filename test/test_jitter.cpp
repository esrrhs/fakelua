#include "compile/compiler.h"
#include "fakelua.h"
#include "var/var_string.h"
#include "var/var_table.h"
#include "gtest/gtest.h"

using namespace fakelua;

static std::vector<JITType> GetSupportedJitTypes() {
    return {JIT_TCC, JIT_GCC};
}

static void JitterRunHelper(const std::function<void(State *, JITType, bool)> &f) {
    const auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    for (const auto type: GetSupportedJitTypes()) {
        f(s, type, true);
        f(s, type, false);
    }
    FakeluaDeleteState(s);
}

TEST(jitter, empty_file) {
    JitterRunHelper(
            [](State *s, JITType type, bool debug_mode) { CompileFile(s, "./jit/test_empty_file.lua", {.debug_mode = debug_mode}); });
}

TEST(jitter, empty_func) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_empty_func.lua", {.debug_mode = debug_mode});
        CVar ret;
        const auto v = reinterpret_cast<Var &>(ret);
        Call(s, type, "test", ret);
        ASSERT_EQ(v.Type(), VarType::Nil);
    });
}

TEST(jitter, empty_local_func) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_empty_local_func.lua", {.debug_mode = debug_mode});
        CVar ret;
        const auto v = reinterpret_cast<Var &>(ret);
        Call(s, type, "test", ret);
        ASSERT_EQ(v.Type(), VarType::Nil);
    });
}

// Multi-return is not supported yet, these tests verify the exception is thrown
TEST(jitter, multi_return) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_multi_return.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, multi_return_call) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_multi_return_call.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, multi_return_call_ex) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_multi_return_call_ex.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, multi_return_sub) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_multi_return_sub.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, multi_return_multi) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_multi_return_multi.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, multi_return_multi_ex) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_multi_return_multi_ex.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, multi_name) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_multi_name_func.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, multi_col_name) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_multi_col_name_func.lua", {.debug_mode = true});
            },
            std::exception);
}

// Global const variable definitions
TEST(jitter, const_define) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_const_define.lua", {.debug_mode = debug_mode});
        int i = 0;
        Call(s, type, "test", i);
        ASSERT_EQ(i, 1);
    });
}

TEST(jitter, multi_const_define) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_multi_const_define.lua", {.debug_mode = debug_mode});

        CVar ret0 = {};
        auto v = reinterpret_cast<Var &>(ret0);
        int ret1 = 0;
        bool ret2 = false;
        bool ret3 = false;
        std::string ret4;
        double ret5;
        Call(s, type, "test0", ret0);
        Call(s, type, "test1", ret1);
        Call(s, type, "test2", ret2);
        Call(s, type, "test3", ret3);
        Call(s, type, "test4", ret4);
        Call(s, type, "test5", ret5);

        ASSERT_EQ(v.Type(), VarType::Nil);
        ASSERT_EQ(ret1, 1);
        ASSERT_EQ(ret2, false);
        ASSERT_EQ(ret3, true);
        ASSERT_EQ(ret4, "test");
        ASSERT_NEAR(ret5, 2.3, 0.001);
    });
}

TEST(jitter, empty_func_with_params) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_empty_func_with_params.lua", {.debug_mode = debug_mode});

        int ret1 = 0;
        bool ret2 = false;
        std::string ret3;
        double ret4 = 0;
        Call(s, type, "test1", ret1, 2.3, "test", true, 1);
        Call(s, type, "test2", ret2, 2.3, "test", true, 1);
        Call(s, type, "test3", ret3, 2.3, "test", true, 1);
        Call(s, type, "test4", ret4, 2.3, "test", true, 1);
        ASSERT_EQ(ret1, 1);
        ASSERT_EQ(ret2, true);
        ASSERT_EQ(ret3, "test");
        ASSERT_NEAR(ret4, 2.3, 0.001);
    });
}

// Variadic functions (...) are not supported yet, these tests verify the exception is thrown
TEST(jitter, variadic_func) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_variadic_func.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, variadic_func_with_params) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_variadic_func_with_params.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, string) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileString(s, "function test() return 1 end", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 1);
    });
}
//
TEST(jitter, local_define) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_local_define.lua", {.debug_mode = debug_mode});
        CVar ret;
        const auto v = reinterpret_cast<Var &>(ret);
        Call(s, type, "test", ret);
        ASSERT_EQ(v.Type(), VarType::Nil);
    });
}

TEST(jitter, local_define_with_values) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_local_define_with_value.lua", {.debug_mode = debug_mode});
        bool ret1 = false;
        int ret2 = 0;
        int ret3 = 0;
        std::string ret4;
        CVar ret5;
        const auto v5 = reinterpret_cast<Var &>(ret5);
        Call(s, type, "test1", ret1, true, 2);
        Call(s, type, "test2", ret2, true, 2);
        Call(s, type, "test3", ret3, true, 2);
        Call(s, type, "test4", ret4, true, 2);
        Call(s, type, "test5", ret5, true, 2);
        ASSERT_EQ(ret1, true);
        ASSERT_EQ(ret2, 2);
        ASSERT_EQ(ret3, 1);
        ASSERT_EQ(ret4, "test");
        ASSERT_EQ(v5.Type(), VarType::Nil);
    });
}
//
TEST(jitter, test_assign) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_assign.lua", {.debug_mode = debug_mode});
        int a = 0;
        std::string b;
        Call(s, type, "test1", a, true, 1.1);
        Call(s, type, "test2", b, true, 1.1);
        ASSERT_EQ(a, 1);
        ASSERT_EQ(b, "2");
    });
}
//
// Mismatched var/exp counts (e.g. a, b = 1) are rejected by PreprocessSplitAssign
TEST(jitter, test_assign_not_match) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_assign_not_match.lua", {.debug_mode = true});
            },
            std::exception);
}

// Variadic assignment (local a, b = ...) is not supported, these tests verify the exception is thrown
TEST(jitter, test_assign_variadic_match) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_assign_variadic.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, test_assign_variadic_no_match) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_assign_variadic_no_match.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, test_assign_variadic_empty) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_assign_variadic_no_match.lua", {.debug_mode = true});
            },
            std::exception);
}

// Table constructors in global variable initialization are not supported
TEST(jitter, test_const_table) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_const_table.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, test_const_nested_table) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_const_nested_table.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, test_local_table) {
    std::vector<VarInterface *> tmp;
    auto newfunc = [&]() {
        auto ret = new SimpleVarImpl();
        tmp.push_back(ret);
        return ret;
    };
    JitterRunHelper([&](State *s, JITType type, bool debug_mode) {
        SetVarInterfaceNewFunc(s, newfunc);
        CompileFile(s, "./jit/test_local_table.lua", {.debug_mode = debug_mode});
        VarInterface *t1 = nullptr;
        VarInterface *t2 = nullptr;
        VarInterface *t3 = nullptr;
        Call(s, type, "test1", t1);
        Call(s, type, "test2", t2);
        Call(s, type, "test3", t3);
        ASSERT_NE(t1, nullptr);
        ASSERT_EQ(t1->ViGetType(), VarInterface::Type::TABLE);
        ASSERT_NE(t2, nullptr);
        ASSERT_EQ(t2->ViGetType(), VarInterface::Type::TABLE);
        ASSERT_NE(t3, nullptr);
        ASSERT_EQ(t3->ViGetType(), VarInterface::Type::TABLE);

        // need sort kv
        dynamic_cast<SimpleVarImpl *>(t1)->ViSortTable();
        dynamic_cast<SimpleVarImpl *>(t2)->ViSortTable();
        dynamic_cast<SimpleVarImpl *>(t3)->ViSortTable();
        ASSERT_EQ(t1->ViToString(0),
                  "table:\n\t[1] = 1\n\t[2] = 2\n\t[3] = 3\n\t[4] = 4\n\t[5] = 5\n\t[6] = 6\n\t[7] = 7\n\t[8] = 8\n\t[9] = 9\n\t[10] = 10");
        ASSERT_EQ(t2->ViToString(0), "table:\n\t[\"a\"] = 1\n\t[\"b\"] = 2\n\t[\"c\"] = 3");
        ASSERT_EQ(t3->ViToString(0), "table:\n\t[1] = 1\n\t[2] = 3\n\t[3] = 5\n\t[\"b\"] = 2\n\t[\"d\"] = 4");
    });
    for (auto &i: tmp) {
        delete i;
    }
}

TEST(jitter, test_local_nested_table) {
    std::vector<VarInterface *> tmp;
    auto newfunc = [&]() {
        auto ret = new SimpleVarImpl();
        tmp.push_back(ret);
        return ret;
    };
    JitterRunHelper([&](State *s, JITType type, bool debug_mode) {
        SetVarInterfaceNewFunc(s, newfunc);
        CompileFile(s, "./jit/test_local_nested_table.lua", {.debug_mode = debug_mode});
        VarInterface *t = nullptr;
        Call(s, type, "test", t);
        ASSERT_NE(t, nullptr);
        ASSERT_EQ(t->ViGetType(), VarInterface::Type::TABLE);

        // need sort kv
        dynamic_cast<SimpleVarImpl *>(t)->ViSortTable();
        ASSERT_EQ(t->ViToString(0),
                  "table:\n\t[\"array\"] = table:\n\t\t[1] = 1\n\t\t[2] = 2\n\t\t[3] = 3\n\t[\"map\"] = table:\n\t\t[\"a\"] "
                  "= 1\n\t\t[\"b\"] = 2\n\t\t[\"c\"] = 3");
    });
    for (auto &i: tmp) {
        delete i;
    }
}

// Variadic (...) in table constructors is not supported yet, these tests verify the exception is thrown
TEST(jitter, test_local_table_with_variadic) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_local_table_with_variadic.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, test_local_table_with_variadic_no_end) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_local_table_with_variadic_no_end.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, test_local_table_with_variadic_no_end_replace) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_local_table_with_variadic_no_end_replace.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, compile_empty_string) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) { CompileString(s, "", {.debug_mode = debug_mode}); });
}

TEST(jitter, test_assign_simple_var) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_assign_simple_var.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 0, 1);
        ASSERT_EQ(ret, 1);
    });
}

TEST(jitter, test_const_define_simple_var) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_const_define_simple_var.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, test_binop_plus) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_binop_plus.lua", {.debug_mode = debug_mode});
        int ret1 = 0;
        double ret2 = 0;
        Call(s, type, "test1", ret1, 1, 2);
        Call(s, type, "test2", ret2, 1.1, 2.2);
        ASSERT_EQ(ret1, 3);
        ASSERT_NEAR(ret2, 3.3, 0.001);
        ret1 = 0;
        ret2 = 0;
        Call(s, type, "test1", ret1, 1, 2);
        Call(s, type, "test2", ret2, 1.1, 2.2);
        ASSERT_EQ(ret1, 3);
        ASSERT_NEAR(ret2, 3.3, 0.001);
    });
}

TEST(jitter, test_const_binop_plus) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_const_binop_plus.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, test_binop_minus) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_binop_minus.lua", {.debug_mode = debug_mode});
        int ret1 = 0;
        double ret2 = 0;
        Call(s, type, "test1", ret1, 1, 2);
        Call(s, type, "test2", ret2, 2, 1.2);
        ASSERT_EQ(ret1, -1);
        ASSERT_NEAR(ret2, 0.8, 0.001);
        ret1 = 0;
        ret2 = 0;
        Call(s, type, "test1", ret1, 1, 2);
        Call(s, type, "test2", ret2, 2, 1.2);
        ASSERT_EQ(ret1, -1);
        ASSERT_NEAR(ret2, 0.8, 0.001);
    });
}

TEST(jitter, test_const_binop_minus) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_const_binop_minus.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, test_binop_star) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_binop_star.lua", {.debug_mode = debug_mode});
        int ret1 = 0;
        double ret2 = 0;
        Call(s, type, "test1", ret1, 1, 2);
        Call(s, type, "test2", ret2, 2, 1.2);
        ASSERT_EQ(ret1, 4);
        ASSERT_NEAR(ret2, 1.4, 0.001);
        ret1 = 0;
        ret2 = 0;
        Call(s, type, "test1", ret1, 1, 2);
        Call(s, type, "test2", ret2, 2, 1.2);
        ASSERT_EQ(ret1, 4);
        ASSERT_NEAR(ret2, 1.4, 0.001);
    });
}

TEST(jitter, test_const_binop_star) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_const_binop_star.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, test_empty_return) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_empty_return.lua", {.debug_mode = debug_mode});
        CVar ret;
        const auto v = reinterpret_cast<Var &>(ret);
        Call(s, type, "test", ret);
        ASSERT_EQ(v.Type(), VarType::Nil);
    });
}

TEST(jitter, test_empty_func_no_return) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_empty_func_no_return.lua", {.debug_mode = debug_mode});
        CVar ret;
        const auto v = reinterpret_cast<Var &>(ret);
        Call(s, type, "test", ret);
        ASSERT_EQ(v.Type(), VarType::Nil);
    });
}

TEST(jitter, test_binop_slash) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_binop_slash.lua", {.debug_mode = debug_mode});
        double ret1 = 0;
        double ret2 = 0;
        Call(s, type, "test1", ret1, 1, 2);
        Call(s, type, "test2", ret2, 2.4, 1.2);
        ASSERT_NEAR(ret1, 2.5, 0.001);
        ASSERT_NEAR(ret2, 1, 0.001);
        ret1 = 0;
        ret2 = 0;
        Call(s, type, "test1", ret1, 1, 2);
        Call(s, type, "test2", ret2, 2.4, 1.2);
        ASSERT_NEAR(ret1, 2.5, 0.001);
        ASSERT_NEAR(ret2, 1, 0.001);
    });
}

TEST(jitter, test_const_binop_slash) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_const_binop_slash.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, test_binop_double_slash) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_binop_double_slash.lua", {.debug_mode = debug_mode});
        int ret1 = 0;
        double ret2 = 0;
        Call(s, type, "test1", ret1, 3, 2);
        Call(s, type, "test2", ret2, 2.4, 1.2);
        ASSERT_EQ(ret1, 3);
        ASSERT_NEAR(ret2, 1.0, 0.001);
    });
}

TEST(jitter, test_const_binop_double_slash) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_const_binop_double_slash.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, test_binop_pow) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_binop_pow.lua", {.debug_mode = debug_mode});
        double ret1 = 0;
        double ret2 = 0;
        Call(s, type, "test1", ret1, 3, 2);
        Call(s, type, "test2", ret2, 2.4, 1.2);
        ASSERT_NEAR(ret1, 11, 0.001);
        ASSERT_NEAR(ret2, 1.859258955601, 0.001);
    });
}

TEST(jitter, test_const_binop_pow) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_const_binop_pow.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, test_binop_mod) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_binop_mod.lua", {.debug_mode = debug_mode});
        int ret1 = 0;
        double ret2 = 0;
        Call(s, type, "test1", ret1, 3, 2);
        Call(s, type, "test2", ret2, 2.4, 1.2);
        ASSERT_EQ(ret1, 3);
        ASSERT_NEAR(ret2, -1.0, 0.001);
    });
}

TEST(jitter, test_const_binop_mod) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_const_binop_mod.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, test_binop_bitand) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_binop_bitand.lua", {.debug_mode = debug_mode});
        int ret1 = 0;
        int ret2 = 0;
        Call(s, type, "test1", ret1, 3, 2);
        Call(s, type, "test2", ret2, 4, 12);
        ASSERT_EQ(ret1, 0);
        ASSERT_EQ(ret2, 0);
    });
}

TEST(jitter, test_const_binop_bitand) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_const_binop_bitand.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, test_binop_xor) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_binop_xor.lua", {.debug_mode = debug_mode});
        int ret1 = 0;
        int ret2 = 0;
        Call(s, type, "test1", ret1, 3, 2);
        Call(s, type, "test2", ret2, 4, 12);
        ASSERT_EQ(ret1, 7);
        ASSERT_EQ(ret2, 15);
    });
}

TEST(jitter, test_const_binop_xor) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_const_binop_xor.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, test_binop_bitor) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_binop_bitor.lua", {.debug_mode = debug_mode});
        int ret1 = 0;
        int ret2 = 0;
        Call(s, type, "test1", ret1, 3, 2);
        Call(s, type, "test2", ret2, 4, 12);
        ASSERT_EQ(ret1, 7);
        ASSERT_EQ(ret2, 15);
    });
}

TEST(jitter, test_const_binop_bitor) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_const_binop_bitor.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, test_binop_right_shift) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_binop_right_shift.lua", {.debug_mode = debug_mode});
        int ret1 = 0;
        int ret2 = 0;
        Call(s, type, "test1", ret1, 3, 2);
        Call(s, type, "test2", ret2, 4, 12);
        ASSERT_EQ(ret1, 1);
        ASSERT_EQ(ret2, 0);
    });
}

TEST(jitter, test_const_binop_right_shift) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_const_binop_right_shift.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, test_binop_left_shift) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_binop_left_shift.lua", {.debug_mode = debug_mode});
        int ret1 = 0;
        int ret2 = 0;
        Call(s, type, "test1", ret1, 3, 2);
        Call(s, type, "test2", ret2, 4, 12);
        ASSERT_EQ(ret1, 20);
        ASSERT_EQ(ret2, 8192);
    });
}

TEST(jitter, test_const_binop_left_shift) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_const_binop_left_shift.lua", {.debug_mode = true});
            },
            std::exception);
}

// Lua 5.4: 移位量 >= 64 时结果为 0
TEST(jitter, test_binop_left_shift_overflow) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_binop_left_shift_overflow.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, -1, 64);
        ASSERT_EQ(ret, 0);
        Call(s, type, "test", ret, -1, 100);
        ASSERT_EQ(ret, 0);
        Call(s, type, "test", ret, -1, -64);
        ASSERT_EQ(ret, 0);
        // 位移 63 不是溢出，仍应正常工作
        Call(s, type, "test", ret, 1, 63);
        ASSERT_EQ(ret, static_cast<int>(static_cast<int64_t>(1LL << 63)));
    });
}

TEST(jitter, test_binop_right_shift_overflow) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_binop_right_shift_overflow.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, -1, 64);
        ASSERT_EQ(ret, 0);
        Call(s, type, "test", ret, -1, 100);
        ASSERT_EQ(ret, 0);
        Call(s, type, "test", ret, -1, -64);
        ASSERT_EQ(ret, 0);
    });
}

TEST(jitter, test_binop_concat) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_binop_concat.lua", {.debug_mode = debug_mode});
        std::string ret;
        Call(s, type, "test", ret, 3, 1.2, true, "test");
        ASSERT_EQ(ret, "31.2truetest");
    });
}

TEST(jitter, test_const_binop_concat) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_const_binop_concat.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, test_binop_less) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_binop_less.lua", {.debug_mode = debug_mode});
        bool ret1 = false;
        bool ret2 = false;
        Call(s, type, "test1", ret1, 3, 1.2);
        Call(s, type, "test2", ret2, 1, 10);
        ASSERT_FALSE(ret1);
        ASSERT_TRUE(ret2);
    });
}

TEST(jitter, test_const_binop_less) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_const_binop_less.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, test_binop_less_equal) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_binop_less_equal.lua", {.debug_mode = debug_mode});
        bool ret1 = false;
        bool ret2 = false;
        Call(s, type, "test1", ret1, 3, 1.2);
        Call(s, type, "test2", ret2, 10, 10);
        ASSERT_FALSE(ret1);
        ASSERT_TRUE(ret2);
    });
}

TEST(jitter, test_const_binop_less_equal) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_const_binop_less_equal.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, test_binop_more) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_binop_more.lua", {.debug_mode = debug_mode});
        bool ret1 = false;
        bool ret2 = false;
        Call(s, type, "test1", ret1, 3, 1.2);
        Call(s, type, "test2", ret2, 1, 10);
        ASSERT_TRUE(ret1);
        ASSERT_FALSE(ret2);
    });
}

TEST(jitter, test_const_binop_more) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_const_binop_more.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, test_binop_more_equal) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_binop_more_equal.lua", {.debug_mode = debug_mode});
        bool ret1 = false;
        bool ret2 = false;
        Call(s, type, "test1", ret1, 1, 1.2);
        Call(s, type, "test2", ret2, 10, 10);
        ASSERT_FALSE(ret1);
        ASSERT_TRUE(ret2);
    });
}

TEST(jitter, test_const_binop_more_equal) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_const_binop_more_equal.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, test_binop_equal) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_binop_equal.lua", {.debug_mode = debug_mode});
        bool ret1 = false;
        bool ret2 = false;
        Call(s, type, "test1", ret1, 1, 1.2);
        Call(s, type, "test2", ret2, "10", "10");
        ASSERT_FALSE(ret1);
        ASSERT_TRUE(ret2);
    });
}

TEST(jitter, test_const_binop_equal) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_const_binop_equal.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, test_binop_not_equal) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_binop_not_equal.lua", {.debug_mode = debug_mode});
        bool ret1 = false;
        bool ret2 = false;
        Call(s, type, "test1", ret1, 1, 1.2);
        Call(s, type, "test2", ret2, "10", "10");
        ASSERT_TRUE(ret1);
        ASSERT_FALSE(ret2);
    });
}

TEST(jitter, test_const_binop_not_equal) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_const_binop_not_equal.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, test_binop_and) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_binop_and.lua", {.debug_mode = debug_mode});
        float ret1 = 0;
        CVar ret2 = {};
        auto v = reinterpret_cast<Var &>(ret2);
        Call(s, type, "test1", ret1, 1, 1.2);
        Call(s, type, "test2", ret2, nullptr, "10");
        ASSERT_NEAR(ret1, 1.2, 0.001);
        ASSERT_EQ(v.Type(), VarType::Nil);
    });
}

TEST(jitter, test_binop_and_bool) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_binop_and_bool.lua", {.debug_mode = debug_mode});
        bool ret1 = false;
        bool ret2 = false;
        Call(s, type, "test1", ret1, true, false);
        Call(s, type, "test2", ret2, true, false);
        ASSERT_FALSE(ret1);
        ASSERT_TRUE(ret2);
    });
}

TEST(jitter, test_binop_and_or) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_binop_and_or.lua", {.debug_mode = debug_mode});
        int ret1 = 0;
        float ret2 = 0;
        Call(s, type, "test1", ret1, 1, 2, 3);
        Call(s, type, "test2", ret2, nullptr, 2, 3);
        ASSERT_EQ(ret1, 3);
        ASSERT_EQ(ret2, 4);
    });
}

TEST(jitter, test_const_binop_and) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_const_binop_and.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, test_binop_or) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_binop_or.lua", {.debug_mode = debug_mode});
        int ret1 = 0;
        int ret2 = 0;
        Call(s, type, "test1", ret1, 1, 1.2);
        Call(s, type, "test2", ret2, nullptr, 10);
        ASSERT_EQ(ret1, 3);
        ASSERT_EQ(ret2, 9);
    });
}

TEST(jitter, test_const_binop_or) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_const_binop_or.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, test_unop_minus) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_unop_minus.lua", {.debug_mode = debug_mode});
        float ret = 0;
        Call(s, type, "test", ret, 2, 2.2);
        ASSERT_NEAR(ret, -3.4, 0.001);
    });
}

TEST(jitter, test_const_unop_minus) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_const_unop_minus.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, test_unop_not) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_unop_not.lua", {.debug_mode = debug_mode});
        bool ret1 = false;
        bool ret2 = false;
        Call(s, type, "test1", ret1, 2);
        Call(s, type, "test2", ret2, nullptr);
        ASSERT_FALSE(ret1);
        ASSERT_TRUE(ret2);
    });
}

TEST(jitter, test_const_unop_not) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_const_unop_not.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, test_unop_len) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_unop_len.lua", {.debug_mode = debug_mode});
        int ret1 = 0;
        int ret2 = 0;
        Call(s, type, "test1", ret1, "abc");
        Call(s, type, "test2", ret2, "123");
        ASSERT_EQ(ret1, 3);
        ASSERT_EQ(ret2, 3);
    });
}

TEST(jitter, test_const_unop_len) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_const_unop_len.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, test_unop_bitnot) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_unop_bitnot.lua", {.debug_mode = debug_mode});
        int ret1 = 0;
        int ret2 = 0;
        Call(s, type, "test1", ret1, 123);
        Call(s, type, "test2", ret2, -123);
        ASSERT_EQ(ret1, -124);
        ASSERT_EQ(ret2, 122);
    });
}

TEST(jitter, test_const_unop_bitnot) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_const_unop_bitnot.lua", {.debug_mode = true});
            },
            std::exception);
}

//
TEST(jitter, test_local_func_call) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_local_func_call.lua", {.debug_mode = debug_mode});
        bool ret = false;
        Call(s, type, "test", ret, 2, 1);
        ASSERT_TRUE(ret);
        Call(s, type, "test", ret, 1, 2);
        ASSERT_FALSE(ret);
    });
}

TEST(jitter, test_global_func_call) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_global_func_call.lua", {.debug_mode = debug_mode});
        bool ret = false;
        Call(s, type, "test", ret, 2, 1);
        ASSERT_TRUE(ret);
    });
}

TEST(jitter, test_other_func_call) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_other_func_call_inner.lua", {.debug_mode = debug_mode});
        CompileFile(s, "./jit/test_other_func_call.lua", {.debug_mode = debug_mode});
        bool ret = false;
        Call(s, type, "test", ret, 2, 1);
        ASSERT_TRUE(ret);
        Call(s, type, "test", ret, 1, 2);
        ASSERT_FALSE(ret);
    });
}

TEST(jitter, test_assign_table_var) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_assign_table_var.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, std::string("key"), 42);
        ASSERT_EQ(ret, 42);
        Call(s, type, "test", ret, 1, 99);
        ASSERT_EQ(ret, 99);
    });
}

TEST(jitter, test_do_block) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_do_block.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, true, 1.1);
        ASSERT_EQ(ret, 1);
    });
}
TEST(jitter, test_while) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_while.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 1, "a");
        ASSERT_EQ(ret, 3);
        std::string ret2;
        Call(s, type, "test2", ret2, 1, "a");
        ASSERT_EQ(ret2, "a22");
    });
}
TEST(jitter, test_repeat) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_repeat.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 1, std::string("a"));
        ASSERT_EQ(ret, 3);
        std::string ret2;
        Call(s, type, "test2", ret2, 1, std::string("a"));
        ASSERT_EQ(ret2, "a22");
    });
}
TEST(jitter, test_while_double) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_while_double.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 3, 4);
        ASSERT_EQ(ret, 18);
    });
}
TEST(jitter, test_repeat_double) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_repeat_double.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 4, 3);
        ASSERT_EQ(ret, 18);
    });
}
//
TEST(jitter, test_if) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_if.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 5, 6);
        ASSERT_EQ(ret, 4);
        Call(s, type, "test", ret, 5, 2);
        ASSERT_EQ(ret, 3);
        Call(s, type, "test", ret, 3, 2);
        ASSERT_EQ(ret, 2);
        Call(s, type, "test", ret, 0, 2);
        ASSERT_EQ(ret, 0);
    });
}

TEST(jitter, test_if_simple) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_if_simple.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 5, 6);
        ASSERT_EQ(ret, 4);
        Call(s, type, "test", ret, 0, 2);
        ASSERT_EQ(ret, 0);
    });
}

TEST(jitter, test_if_elseif) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_if_elseif.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 5, 5);
        ASSERT_EQ(ret, 4);
        Call(s, type, "test", ret, 4, 3);
        ASSERT_EQ(ret, 3);
        Call(s, type, "test", ret, 3, 2);
        ASSERT_EQ(ret, 2);
        Call(s, type, "test", ret, 2, 1);
        ASSERT_EQ(ret, 1);
        Call(s, type, "test", ret, 0, 0);
        ASSERT_EQ(ret, 0);
    });
}

TEST(jitter, test_if_elseif_normal) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_if_elseif_normal.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 5, 5);
        ASSERT_EQ(ret, 4);
        Call(s, type, "test", ret, 4, 3);
        ASSERT_EQ(ret, 3);
        Call(s, type, "test", ret, 3, 2);
        ASSERT_EQ(ret, 2);
        Call(s, type, "test", ret, 2, 1);
        ASSERT_EQ(ret, 1);
        Call(s, type, "test", ret, 0, 0);
        ASSERT_EQ(ret, 0);
    });
}

TEST(jitter, test_if_elseif_return) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_if_elseif_return.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 5, 5);
        ASSERT_EQ(ret, 4);
        Call(s, type, "test", ret, 4, 3);
        ASSERT_EQ(ret, 3);
        Call(s, type, "test", ret, 3, 2);
        ASSERT_EQ(ret, 2);
        Call(s, type, "test", ret, 2, 1);
        ASSERT_EQ(ret, 1);
        Call(s, type, "test", ret, 0, 0);
        ASSERT_EQ(ret, 0);
    });
}

TEST(jitter, test_if_else) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_if_else.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 5, 6);
        ASSERT_EQ(ret, 4);
        Call(s, type, "test", ret, 0, 2);
        ASSERT_EQ(ret, 0);
    });
}

TEST(jitter, test_while_return) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_while_return.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 4, 4);
        ASSERT_EQ(ret, 4);
        Call(s, type, "test", ret, 2, 4);
        ASSERT_EQ(ret, 2);
    });
}
TEST(jitter, test_repeat_return) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_repeat_return.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 4, 4);
        ASSERT_EQ(ret, 4);
    });
}
TEST(jitter, test_while_break) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_while_break.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 4, 4);
        ASSERT_EQ(ret, 11);
    });
}
TEST(jitter, test_repeat_break) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_repeat_break.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 4, 4);
        ASSERT_EQ(ret, 11);
    });
}
TEST(jitter, test_while_if_return) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_while_if_return.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 3, 4);
        ASSERT_EQ(ret, 3);
        Call(s, type, "test", ret, 4, 4);
        ASSERT_EQ(ret, 4);
    });
}
TEST(jitter, test_repeat_if_return) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_repeat_if_return.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 3, 4);
        ASSERT_EQ(ret, 3);
        Call(s, type, "test", ret, 4, 4);
        ASSERT_EQ(ret, 4);
    });
}
TEST(jitter, test_for_loop) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_for_loop.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 3, 5);
        ASSERT_EQ(ret, 8);
    });
}
TEST(jitter, test_for_loop_default) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_for_loop_default.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 3, 5);
        ASSERT_EQ(ret, 12);
    });
}
TEST(jitter, test_for_loop_return) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_for_loop_return.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 3, 3);
        ASSERT_EQ(ret, 3);
        Call(s, type, "test", ret, 4, 3);
        ASSERT_EQ(ret, 0);
    });
}
TEST(jitter, test_while_just_break) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_while_just_break.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 3, 4);
        ASSERT_EQ(ret, 4);
        Call(s, type, "test", ret, 5, 4);
        ASSERT_EQ(ret, 5);
    });
}
TEST(jitter, test_repeat_just_break) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_repeat_just_break.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 3, 4);
        ASSERT_EQ(ret, 4);
        Call(s, type, "test", ret, 5, 4);
        ASSERT_EQ(ret, 4);
    });
}
TEST(jitter, test_for_loop_double) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_for_loop_double.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 3, 5);
        ASSERT_EQ(ret, 21);
    });
}
TEST(jitter, test_for_loop_break) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_for_loop_break.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 3, 5);
        ASSERT_EQ(ret, 4);
    });
}
TEST(jitter, test_for_loop_if_return) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_for_loop_if_return.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 3, 5);
        ASSERT_EQ(ret, 3);
        Call(s, type, "test", ret, 6, 10);
        ASSERT_EQ(ret, 10);
    });
}
TEST(jitter, test_for_loop_just_break) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_for_loop_just_break.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 3, 5);
        ASSERT_EQ(ret, 5);
        Call(s, type, "test", ret, 5, 3);
        ASSERT_EQ(ret, 5);
    });
}
TEST(jitter, test_for_in) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_for_in.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 3, 4);
        ASSERT_EQ(ret, 32);
    });
}
TEST(jitter, test_for_in_double) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_for_in_double.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 3, 4);
        ASSERT_EQ(ret, 320);
    });
}
TEST(jitter, test_for_in_break) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_for_in_break.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 3, 4);
        ASSERT_EQ(ret, 30);
    });
}
TEST(jitter, test_for_in_if_return) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_for_in_if_return.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 3, 4);
        ASSERT_EQ(ret, 3);
        Call(s, type, "test", ret, 5, 4);
        ASSERT_EQ(ret, 4);
    });
}
TEST(jitter, test_for_in_just_break) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_for_in_just_break.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 3, 4);
        ASSERT_EQ(ret, 3);
    });
}
TEST(jitter, test_for_in_return) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_for_in_return.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 3, 4);
        ASSERT_EQ(ret, 3);
    });
}
TEST(jitter, test_for_in_return_fallback) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_for_in_return_fallback.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 3, 4);
        ASSERT_EQ(ret, 0);
    });
}
//
//
TEST(jitter, test_local_func_call_table_construct) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_local_func_call_table_construct.lua", {.debug_mode = debug_mode});
        bool ret = false;
        Call(s, type, "test", ret, 2, 1);
        ASSERT_TRUE(ret);
        Call(s, type, "test", ret, 1, 2);
        ASSERT_FALSE(ret);
    });
}

TEST(jitter, test_local_func_call_string) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_local_func_call_string.lua", {.debug_mode = debug_mode});
        std::string ret;
        Call(s, type, "test", ret, 2, 1);
        ASSERT_EQ(ret, "test_test");
    });
}

// Dynamic function call via local variable: compiles successfully, but will naturally
// fail at runtime since the local variable name "c" is used as the literal function name.
TEST(jitter, test_var_func_call) {
    JitterRunHelper(
            [](State *s, JITType type, bool debug_mode) { CompileFile(s, "./jit/test_var_func_call.lua", {.debug_mode = debug_mode}); });
}

// Dynamic function call via table index (c[k](a,b)) is not supported.
TEST(jitter, test_table_var_func_call) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_table_var_func_call.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, test_empty_func_call) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_empty_func_call.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 2, 2);
        ASSERT_EQ(ret, 1);
    });
}

TEST(jitter, test_table_get_set) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_table_get_set.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 1, 2);
        ASSERT_EQ(ret, 3);
    });
}


TEST(jitter, test_table_stringid_dynamic_get) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_table_stringid_dynamic_get.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, std::string("abc"));
        ASSERT_EQ(ret, 11);
    });
}

TEST(jitter, test_table_stringid_dynamic_set) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_table_stringid_dynamic_set.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, std::string("abc"), 99);
        ASSERT_EQ(ret, 99);
    });
}

TEST(jitter, test_table_float_int_key_alias_set) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_table_float_int_key_alias_set.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 5);
    });
}

TEST(jitter, test_table_float_int_key_alias_get) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_table_float_int_key_alias_get.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 7);
    });
}

TEST(jitter, test_table_empty_get) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_table_empty_get.lua", {.debug_mode = debug_mode});
        CVar ret = {};
        const auto v = reinterpret_cast<Var &>(ret);
        Call(s, type, "test", ret);
        ASSERT_EQ(v.Type(), VarType::Nil);
    });
}

TEST(jitter, test_table_quick_delete) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_table_quick_delete.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 3);
    });
}

TEST(jitter, test_table_hash_delete_head) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_table_hash_delete_head.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 170);
    });
}

TEST(jitter, test_table_hash_delete_chain) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_table_hash_delete_chain.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 10);
    });
}

// Bug 4: Int and Float with equal mathematical value must compare as equal in JIT code.
TEST(jitter, test_binop_equal_int_float) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_binop_equal_int_float.lua", {.debug_mode = debug_mode});
        bool eq = false;
        // 1 (int) == 1.0 (float) -> true
        Call(s, type, "test_int_eq_float", eq, 1, 1.0);
        ASSERT_TRUE(eq);
        // 1.0 (float) == 1 (int) -> true
        eq = false;
        Call(s, type, "test_float_eq_int", eq, 1.0, 1);
        ASSERT_TRUE(eq);
        // 1 (int) == 1.5 (float) -> false
        eq = true;
        Call(s, type, "test_int_neq_float", eq, 1, 1.5);
        ASSERT_FALSE(eq);
    });
}

// Bug 5: Two multi-assign statements on the same source line must not generate
// duplicate temp variable names, causing compile errors.
TEST(jitter, test_assign_same_line_multi) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_assign_same_line_multi.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 10);// 1 + 2 + 3 + 4 = 10
    });
}

// Float for-loop: exercises the typed_float_for code path in CompileStmtForLoop.
// begin=1.0, end=2.0, step=0.5 => iterations at 1.0, 1.5, 2.0 => sum = 4.5
TEST(jitter, test_for_loop_float) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_for_loop_float.lua", {.debug_mode = debug_mode});
        double ret = 0.0;
        Call(s, type, "test", ret);
        ASSERT_DOUBLE_EQ(ret, 4.5);
    });
}

// Unary minus applied to a float parameter: exercises the float branch of
// CompileExp for unary minus (UnopMinus on a float Var).
TEST(jitter, test_unop_minus_float) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_unop_minus_float.lua", {.debug_mode = debug_mode});
        double ret = 0.0;
        Call(s, type, "test", ret, 3.5);
        ASSERT_DOUBLE_EQ(ret, -3.5);
    });
}

// T_DYNAMIC subtraction: operands from table lookups are T_DYNAMIC,
// exercises OpSub in CompileBinop CVar path (c_gen.cpp).
TEST(jitter, test_dynamic_arith_sub) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_dynamic_arith_sub.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 10, 3);
        ASSERT_EQ(ret, 7);
    });
}

// T_DYNAMIC division: operands from table lookups are T_DYNAMIC,
// exercises OpDiv in CompileBinop CVar path (c_gen.cpp).
TEST(jitter, test_dynamic_arith_div) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_dynamic_arith_div.lua", {.debug_mode = debug_mode});
        double ret = 0.0;
        Call(s, type, "test", ret, 8.0, 2.0);
        ASSERT_DOUBLE_EQ(ret, 4.0);
    });
}

// T_DYNAMIC floor division: operands from table lookups are T_DYNAMIC,
// exercises OpFloorDiv in CompileBinop CVar path (c_gen.cpp).
TEST(jitter, test_dynamic_arith_floor_div) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_dynamic_arith_floor_div.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 8, 3);
        ASSERT_EQ(ret, 2);
    });
}

// T_DYNAMIC power: operands from table lookups are T_DYNAMIC,
// exercises OpPow in CompileBinop CVar path (c_gen.cpp).
TEST(jitter, test_dynamic_arith_pow) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_dynamic_arith_pow.lua", {.debug_mode = debug_mode});
        double ret = 0.0;
        Call(s, type, "test", ret, 2.0, 3.0);
        ASSERT_DOUBLE_EQ(ret, 8.0);
    });
}

// T_DYNAMIC bitwise AND: operands from table lookups are T_DYNAMIC,
// exercises OpBitAnd in CompileBinop CVar path (c_gen.cpp).
TEST(jitter, test_dynamic_arith_bitand) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_dynamic_arith_bitand.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 12, 10);
        ASSERT_EQ(ret, 8);
    });
}

// T_DYNAMIC bitwise OR: operands from table lookups are T_DYNAMIC,
// exercises OpBitOr in CompileBinop CVar path (c_gen.cpp).
TEST(jitter, test_dynamic_arith_bitor) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_dynamic_arith_bitor.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 12, 10);
        ASSERT_EQ(ret, 14);
    });
}

// T_DYNAMIC bitwise XOR: operands from table lookups are T_DYNAMIC,
// exercises OpBitXor in CompileBinop CVar path (c_gen.cpp).
TEST(jitter, test_dynamic_arith_bitxor) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_dynamic_arith_bitxor.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 12, 10);
        ASSERT_EQ(ret, 6);
    });
}

// T_DYNAMIC right shift: operands from table lookups are T_DYNAMIC,
// exercises OpRightShift in CompileBinop CVar path (c_gen.cpp).
TEST(jitter, test_dynamic_arith_rshift) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_dynamic_arith_rshift.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 8, 1);
        ASSERT_EQ(ret, 4);
    });
}

// T_DYNAMIC left shift: operands from table lookups are T_DYNAMIC,
// exercises OpLeftShift in CompileBinop CVar path (c_gen.cpp).
TEST(jitter, test_dynamic_arith_lshift) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_dynamic_arith_lshift.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 2, 3);
        ASSERT_EQ(ret, 16);
    });
}

// Untyped for loop with explicit dynamic step: helper() returns T_DYNAMIC,
// so the step expression is not typed. This exercises c_gen.cpp lines 2011-2012
// (the ExpStep != nullptr path in the CVar for-loop). Iterations: 1,3,5,7,9 => 5.
TEST(jitter, test_for_loop_dynamic_step) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_for_loop_dynamic_step.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 5);
    });
}

// Typed-int for-loop with step=0 must throw "'for' step is zero" at compile time.
TEST(jitter, test_for_loop_zero_step_int) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_for_loop_zero_step_int.lua", {.debug_mode = true});
            },
            std::exception);
}

// Typed-float for-loop with step=0.0 must throw "'for' step is zero" at compile time.
TEST(jitter, test_for_loop_zero_step_float) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_for_loop_zero_step_float.lua", {.debug_mode = true});
            },
            std::exception);
}

// Dynamic for-loop with step=0 via a function call: the step cannot be known at
// compile time so a runtime guard is emitted in the generated C code.  TCC-compiled
// code cannot propagate C++ exceptions back through its frames, so the runtime error
// cannot be caught in a unit test.  This test verifies only that the Lua code
// compiles without error.
TEST(jitter, test_for_loop_zero_step_dynamic) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        EXPECT_NO_THROW(CompileFile(s, "./jit/test_for_loop_zero_step_dynamic.lua", {.debug_mode = debug_mode}));
    });
}

// ForIn with only 1 loop variable (key only, no value variable).
// Exercises c_gen.cpp CompileStmtForIn lines 2138-2142 (dummy val path).
// Keys 1+2+3 = 6.
TEST(jitter, test_for_in_key_only) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_for_in_key_only.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 10, 20);
        ASSERT_EQ(ret, 6);
    });
}

// Top-level bare local variable (no initializer): exercises c_gen.cpp
// BuildLocalVarExtensions line 797 (the continue that skips the bare local).
TEST(jitter, test_top_level_bare_local) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_top_level_bare_local.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 1);
    });
}

// T_DYNAMIC unary minus: exercises OpUnaryMinus in CompileExp CVar path
// (c_gen.cpp line 2497). x is a T_DYNAMIC table lookup; -x uses OpUnaryMinus.
// -(10) = -10.
TEST(jitter, test_dynamic_unop_minus) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_dynamic_unop_minus.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 10);
        ASSERT_EQ(ret, -10);
    });
}

// Typed-int for-loop: loop variable i shadowed by local i inside the body.
// Without the fix, the generated C would have two declarations of i in the
// same scope, causing a compilation error.
// i=1: inner_i=2; i=2: inner_i=4; i=3: inner_i=6  => sum=12
TEST(jitter, test_for_loop_shadow_local) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_for_loop_shadow_local.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 3);
        ASSERT_EQ(ret, 12);
    });
}

// Typed-float for-loop: loop variable i shadowed by local i inside the body.
// i=1.0: inner_i=1.5; i=2.0: inner_i=2.5; i=3.0: inner_i=3.5  => sum=7.5
TEST(jitter, test_for_loop_float_shadow_local) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_for_loop_float_shadow_local.lua", {.debug_mode = debug_mode});
        double ret = 0.0;
        Call(s, type, "test", ret);
        ASSERT_DOUBLE_EQ(ret, 7.5);
    });
}

// Dynamic for-loop (step from helper() forces CVar path): loop variable i
// shadowed by local i inside the body.
// i=1,3,5 (step=2); inner_i=2,6,10  => sum=18
TEST(jitter, test_for_loop_dynamic_shadow_local) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_for_loop_dynamic_shadow_local.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 18);
    });
}

// For-in loop: loop variable k shadowed by local k inside the body.
// t={10,20,30}; k=1,2,3; inner_k=2,4,6  => sum=12
TEST(jitter, test_for_in_shadow_local) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_for_in_shadow_local.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 12);
    });
}

TEST(jitter, test_table_zero_key) {
    std::vector<VarInterface *> tmp;
    auto newfunc = [&]() {
        auto ret = new SimpleVarImpl();
        tmp.push_back(ret);
        return ret;
    };
    JitterRunHelper([&](State *s, JITType type, bool debug_mode) {
        SetVarInterfaceNewFunc(s, newfunc);
        CompileFile(s, "./jit/test_table_zero_key.lua", {.debug_mode = debug_mode});
        VarInterface *t = nullptr;
        Call(s, type, "test", t);
        ASSERT_NE(t, nullptr);
        ASSERT_EQ(t->ViGetType(), VarInterface::Type::TABLE);
        ASSERT_EQ(t->ViGetTableSize(), 1);
        auto kv = t->ViGetTableKv(0);
        ASSERT_EQ(kv.first->ViGetType(), VarInterface::Type::INT);
        ASSERT_EQ(kv.first->ViGetInt(), 0);
        ASSERT_EQ(kv.second->ViGetType(), VarInterface::Type::INT);
        ASSERT_EQ(kv.second->ViGetInt(), 100);
    });
    for (auto &i: tmp) {
        delete i;
    }
}

TEST(jitter, coverage_c_gen) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        // 1. Table constructor with string/int/generic keys
        CompileString(s, R"(
            function test_table_construct()
                local t = { ["a"] = 1, [2] = 2, [true] = 3 }
                return t["a"] + t[2] + t[true]
            end
        )",
                      {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test_table_construct", ret);
        ASSERT_EQ(ret, 6);

        // 2. Math parameters with dynamic arguments fallback (CompileExp fallback)
        // and complex callee syntax:
        // - my_abs(foo()) + 1: T_INT specialized math, but foo() is not native-compilable
        // - my_abs "string": args is kString, not kExpList
        CompileString(s, R"(
            local function foo()
                return 5
            end
            local function my_abs(n)
                return n + 0 -- promotes n to math param
            end
            function test_math_specializations(y)
                local val1 = my_abs(foo()) + 1          -- native_expr.empty() -> fallback to CompileExp
                return val1
            end
            function test_math_string_arg()
                local val4 = my_abs "5"
                return val4
            end
        )",
                      {.debug_mode = debug_mode});
        ret = 0;
        Call(s, type, "test_math_specializations", ret, -5);
        ASSERT_EQ(ret, 6);// my_abs(5)+1 (6)
    });
}

TEST(jitter, coverage_c_gen_more) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        // 1. Bitwise operations on specialized float
        CompileString(s, R"(
            function test_bitwise_float(x)
                return x & 1
            end
            function test_bitnot_float(x)
                return ~x
            end
            function test_dynamic_le(a, b)
                return a <= b
            end
        )",
                      {.debug_mode = debug_mode});
        int64_t ret = 0;
        Call(s, type, "test_bitwise_float", ret, 5.0);
        ASSERT_EQ(ret, 1);
        Call(s, type, "test_bitnot_float", ret, 5.0);
        ASSERT_EQ(ret, ~5);
        bool ret_b = false;
        Call(s, type, "test_dynamic_le", ret_b, 5, 6);
        ASSERT_TRUE(ret_b);

        // 2. Extra coverage for c_gen.cpp
        CompileString(s, R"(
            local MY_CONST = 42
            local function my_add(x, y)
                return x + y
            end
            local function my_abs(n)
                return n + 0
            end
            function test_more_coverage(x, xf)
                -- Table variable access (VarKind != kSimple)
                local t = { a = 1 }
                local val1 = t.a + x
                
                -- Global constant of type T_INT
                local val2 = x + MY_CONST
                
                -- Unary bitnot on specialized int
                local val3 = 0
                val3 = ~x
                
                -- Unary bitnot on specialized float (degraded)
                local val4 = 0.0
                val4 = ~xf
                
                -- Unary not
                local val5 = not t
                
                -- Calling specialized function with dynamic argument
                local glob = 10
                local val7 = my_abs(glob)
                
                -- TryCompileNativeBoolExpr branches
                if x then end
                if -x then end
                if not x then end
                if 1 then end
                if x > 0 and xf then end
                if x + 1 then end
                
                return val1 + val2 + val3 + val4
            end
            
            function test_string_arg_error()
                return my_abs "5"
            end
            
            function test_shadow_coverage(x)
                local res = 0
                do
                    local x = 6
                    res = x
                end
                return res
            end
            
            function test_table_spec(x)
                local t = { [x] = 2 }
                return t[x]
            end
            
            function test_for_no_step(x)
                local sum = 0
                for i = 1, x do
                    sum = sum + i
                end
                return sum
            end
        )",
                      {.debug_mode = debug_mode});
        Call(s, type, "test_more_coverage", ret, 5, 5.0);
        ASSERT_EQ(ret, 41);
        Call(s, type, "test_shadow_coverage", ret, 5);
        ASSERT_EQ(ret, 6);
        Call(s, type, "test_table_spec", ret, 5);
        ASSERT_EQ(ret, 2);
        Call(s, type, "test_for_no_step", ret, 5);
        ASSERT_EQ(ret, 15);
    });
}

TEST(jitter, test_spec_call_arg_count_error) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileString(sg.GetState(), R"(
                    local function my_add(x, y)
                        return x + y
                    end
                    function test_fewer_args(x)
                        return my_add(x)
                    end
                )",
                              {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, test_set_table_arg_count_error) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileString(sg.GetState(), R"(
                    function test(t, k)
                        FAKELUA_SET_TABLE(t, k)
                    end
                )",
                              {.debug_mode = true});
            },
            std::exception);
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileString(sg.GetState(), R"(
                    function test(t)
                        FAKELUA_SET_TABLE "hello"
                    end
                )",
                              {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, coverage_c_gen_complete) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileString(s, R"(
            local MY_CONST_2 = 100
            local upvalue_int = 10
            local upvalue_float = 20.0

            local function foo(v)
                return v
            end
            local function my_abs(n)
                return n + 0
            end
            local function my_mixed_func(n, name)
                return n + 1
            end
            local function helper_spec_mixed(x)
                return x + my_mixed_func(x, "hello")
            end
            local function helper_spec_str(x)
                return x + my_abs "5"
            end
            local function non_math_helper(x)
                return x
            end
            local function helper_spec_non_math(x)
                return x + non_math_helper(5)
            end
            local function my_abs_dynamic(n)
                if n > 0 then
                    return n
                else
                    return "not a number"
                end
            end
            local function helper_spec_dynamic_ret(x)
                return x + my_abs_dynamic(5)
            end
            local function helper_spec_dynamic_arg(x)
                return x + my_abs(foo(x))
            end
            local function helper_bitwise_cvar(x)
                return foo(x & 1)
            end
            local function test_bitwise_cvar(y)
                local temp = y + 0.0
                return helper_bitwise_cvar(y)
            end

            local function helper_upvalue_spec(x)
                local dummy = x + 1.0
                return dummy + upvalue_int + upvalue_float
            end

            local function helper_bool_ret(x)
                return x > 0
            end
            local function test_bool_ret_spec(x)
                local dummy = x + 1
                return helper_bool_ret(x)
            end

            local function helper_user_func(x)
                return x + 1
            end
            local function test_user_func_spec(x)
                local dummy = x + 1
                return x + helper_user_func(x)
            end

            local function helper_user_func_float(x)
                return x + 1.0
            end
            local function test_user_func_spec_float(x)
                local dummy = x + 1.0
                return x + helper_user_func_float(x)
            end

            local function test_set_table_fallback(t, k, v)
                FAKELUA_SET_TABLE(t, k, v)
            end

            local function helper_default_float(x)
                local dummy = x + 1.0
                if x > 0 then
                    return x + 1.0
                else
                    error("error")
                end
            end
            local function helper_default_int(x)
                local dummy = x + 1
                if x > 0 then
                    return x + 1
                end
            end

            -- Helper to compile specialized statements
            local function helper_complete_spec(x, xf)
                -- Force math param recognition
                local dummy = x + 1
                local dummy2 = xf + 1.0

                -- Parentheses and not unwrapping
                if ((x > 0)) then end
                if not (x > 0) then end

                -- Fallback assignment to specialized variable
                x = foo(10)
                xf = foo(20.0)

                -- Arithmetic operators
                local v_mul = x * x
                local v_div = x / x
                local v_pow = x ^ x
                local v_fdiv_int = x // x
                local v_fdiv_float = xf // xf
                local v_mod_int = x % x
                local v_mod_float = xf % xf

                -- 1. Global constant T_INT assignment to specialized variable
                local val_const = 0
                val_const = MY_CONST_2

                -- 2. Comparison with dynamic call where left is specialized and right is dynamic
                local x_local = 5
                if x_local > my_abs(foo(x)) then end

                -- 3. Local variable shadowing
                local y = x + 1
                local res = 0
                do
                    local y = x + 2
                    res = y
                end

                -- 4. Dynamic for-loop without step
                local sum = 0
                for i = foo(1), foo(5) do
                    sum = sum + i
                end

                -- 5. Table constructor with specialized int key
                local t = { [x] = 2 }

                -- 6. Dynamic <= comparison
                local le_val = xf <= foo(xf)

                -- 7. Dynamic ~ bitnot
                local not_val = ~foo(x)

                return val_const + res + sum + t[x] + (le_val and 1 or 0) + not_val + dummy - dummy + dummy2 - dummy2 + v_mul - v_mul + v_div - v_div + v_pow - v_pow + v_fdiv_int - v_fdiv_int + v_fdiv_float - v_fdiv_float + v_mod_int - v_mod_int + v_mod_float - v_mod_float
            end

            function test_complete(x, xf)
                local temp = x + 0
                local tempf = xf + 0.0
                return helper_complete_spec(x, xf) + test_bitwise_cvar(xf)
            end

            local function helper_specs_all(x)
                -- Force math param recognition
                local dummy = x + 1

                local val1 = helper_spec_mixed(x)
                local val2 = 0
                if false then
                    val2 = helper_spec_str(x)
                end
                local val3 = helper_spec_non_math(x)
                local val4 = helper_spec_dynamic_ret(x)
                local val5 = helper_spec_dynamic_arg(x)
                local val6 = helper_upvalue_spec(x)
                local val7 = 0
                if test_bool_ret_spec(x) then
                    val7 = 1
                end
                local val8 = test_user_func_spec(x)
                local val9 = test_user_func_spec_float(x)

                -- Fallback default returns
                local val10 = helper_default_float(x) or 0.0
                local val11 = helper_default_int(x) or 0

                -- Unary minus and length
                local val12 = -x
                local val13 = 0
                if false then
                    val13 = #val1
                end

                -- Fallback while loop
                while foo(1) do
                    break
                end

                -- Trigger math.abs compiling fallback with native expression fail
                if false then
                    local b = x > 0
                    local res_dyn = my_abs(b & 1)
                end

                local tbl = {}
                test_set_table_fallback(tbl, "mykey", 123)

                return val1 + val2 + val3 + val4 + val5 + val6 + val7 + val8 + val9 + val10 + val11 + val12 - val12 + val13 - val13 + dummy - dummy
            end

            function test_specs_helper(y)
                local temp = y + 0
                return helper_specs_all(y)
            end
        )",
                      {.debug_mode = debug_mode, .record_c_code = true});
        double ret_d = 0.0;
        Call(s, type, "test_complete", ret_d, 5, 5.0);
        ASSERT_DOUBLE_EQ(ret_d, 120.0);

        double ret_d2 = 0.0;
        Call(s, type, "test_specs_helper", ret_d2, 5);
        ASSERT_DOUBLE_EQ(ret_d2, 112.0);
    });
}

TEST(jitter, coverage_c_gen_compiler_errors_simple) {
    EXPECT_THROW({
        FakeluaStateGuard sg;
        CompileString(sg.GetState(), R"(
            local MY_CONST = 10
            local MY_CONST = 20
        )", {.debug_mode = true});
    }, std::exception);
    EXPECT_THROW({
        FakeluaStateGuard sg;
        CompileString(sg.GetState(), R"(
            function test_dup(x, x)
            end
        )", {.debug_mode = true});
    }, std::exception);
    EXPECT_THROW({
        FakeluaStateGuard sg;
        CompileString(sg.GetState(), R"(
            local MY_CONST = 10
            function test()
                local MY_CONST = 20
            end
        )", {.debug_mode = true});
    }, std::exception);
}

TEST(jitter, coverage_c_gen_native_arith_binop) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileString(s, R"(
            local function foo(x)
                return x
            end

            local function test_native_binop_int(x, y)
                local dummy1 = x + 1
                local dummy2 = y + 1
                
                local v_add = foo(x + y)
                local v_sub = foo(x - y)
                local v_mul = foo(x * y)
                local v_div = foo(x / y)
                local v_pow = foo(x ^ y)
                local v_fdiv = foo(x // y)
                local v_mod = foo(x % y)
                local v_and = foo(x & y)
                local v_or = foo(x | y)
                local v_xor = foo(x ~ y)
                local v_shl = foo(x << y)
                local v_shr = foo(x >> y)
                return 1
            end
            
            local function test_native_binop_float(x, y)
                local dummy1 = x + 1.0
                local dummy2 = y + 1.0
                
                local v_add = foo(x + y)
                local v_sub = foo(x - y)
                local v_mul = foo(x * y)
                local v_div = foo(x / y)
                local v_pow = foo(x ^ y)
                local v_fdiv = foo(x // y)
                local v_mod = foo(x % y)
                return 1
            end

            function test_entry(x, y, xf, yf)
                local temp1 = x + 0
                local temp2 = y + 0
                local temp3 = xf + 0.0
                local temp4 = yf + 0.0
                return test_native_binop_int(x, y) + test_native_binop_float(xf, yf)
            end
        )", {.debug_mode = debug_mode});
        double ret_d = 0.0;
        Call(s, type, "test_entry", ret_d, 10, 3, 10.0, 3.0);
        ASSERT_DOUBLE_EQ(ret_d, 2.0);
    });
}

