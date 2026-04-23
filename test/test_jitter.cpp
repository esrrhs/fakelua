#include "compile/compiler.h"
#include "fakelua.h"
#include "var/var_string.h"
#include "var/var_table.h"
#include "gtest/gtest.h"

using namespace fakelua;

// TCC JIT has issues on macOS arm64, so we skip TCC tests on Apple platforms.
static std::vector<JITType> GetSupportedJitTypes() {
#ifdef __APPLE__
    return {JIT_GCC};
#else
    return {JIT_TCC, JIT_GCC};
#endif
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
//
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
        ASSERT_EQ(t->ViToString(0), "table:\n\t[\"array\"] = table:\n\t\t[1] = 1\n\t\t[2] = 2\n\t\t[3] = 3\n\t[\"map\"] = table:\n\t\t[\"a\"] "
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
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_var_func_call.lua", {.debug_mode = debug_mode});
    });
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

