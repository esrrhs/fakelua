#include "compile/compiler.h"
#include "fakelua.h"
#include "state/const_string.h"
#include "state/state.h"
#include "var/var_multi.h"
#include "var/var_string.h"
#include "var/var_table.h"
#include "var/var_type.h"
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
    JitterRunHelper([](State *s, JITType type, bool debug_mode) { CompileFile(s, "./jit/test_empty_file.lua", {.debug_mode = debug_mode}); });
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

TEST(jitter, multi_return) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_multi_return.lua", {.debug_mode = debug_mode});
        CVar ret;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret.type_, static_cast<int>(VarType::Multi));
        VarMulti *m = ret.data_.m;
        ASSERT_EQ(m->GetCount(), 5);
        ASSERT_EQ(m->GetVars()[0].type_, static_cast<int>(VarType::Int));
        ASSERT_EQ(m->GetVars()[0].data_.i, 1);
        ASSERT_EQ(m->GetVars()[1].type_, static_cast<int>(VarType::Float));
        ASSERT_DOUBLE_EQ(m->GetVars()[1].data_.f, 2.3);
        ASSERT_EQ(m->GetVars()[2].type_, static_cast<int>(VarType::Bool));
        ASSERT_FALSE(m->GetVars()[2].data_.b);
        ASSERT_EQ(m->GetVars()[3].type_, static_cast<int>(VarType::Bool));
        ASSERT_TRUE(m->GetVars()[3].data_.b);
        ASSERT_EQ(m->GetVars()[4].type_, static_cast<int>(VarType::StringId));
    });
}

TEST(jitter, multi_return_call) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_multi_return_call.lua", {.debug_mode = debug_mode});
        CVar ret;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret.type_, static_cast<int>(VarType::Multi));
        VarMulti *m = ret.data_.m;
        ASSERT_EQ(m->GetCount(), 3);
        ASSERT_EQ(m->GetVars()[0].type_, static_cast<int>(VarType::StringId));
        ASSERT_EQ(m->GetVars()[1].type_, static_cast<int>(VarType::Int));
        ASSERT_EQ(m->GetVars()[1].data_.i, 1);
        ASSERT_EQ(m->GetVars()[2].type_, static_cast<int>(VarType::Float));
        ASSERT_DOUBLE_EQ(m->GetVars()[2].data_.f, 2.3);
    });
}

TEST(jitter, multi_return_call_ex) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_multi_return_call_ex.lua", {.debug_mode = debug_mode});
        CVar ret;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret.type_, static_cast<int>(VarType::Multi));
        VarMulti *m = ret.data_.m;
        ASSERT_EQ(m->GetCount(), 3);
        ASSERT_EQ(m->GetVars()[0].type_, static_cast<int>(VarType::StringId));
        ASSERT_EQ(m->GetVars()[1].type_, static_cast<int>(VarType::Int));
        ASSERT_EQ(m->GetVars()[1].data_.i, 1);
        ASSERT_EQ(m->GetVars()[2].type_, static_cast<int>(VarType::Float));
        ASSERT_DOUBLE_EQ(m->GetVars()[2].data_.f, 2.4);
    });
}

TEST(jitter, multi_return_sub) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_multi_return_sub.lua", {.debug_mode = debug_mode});
        CVar ret;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret.type_, static_cast<int>(VarType::Multi));
        VarMulti *m = ret.data_.m;
        ASSERT_EQ(m->GetCount(), 3);
        ASSERT_EQ(m->GetVars()[0].type_, static_cast<int>(VarType::StringId));
        ASSERT_EQ(m->GetVars()[1].type_, static_cast<int>(VarType::Int));
        ASSERT_EQ(m->GetVars()[1].data_.i, 1);
        ASSERT_EQ(m->GetVars()[2].type_, static_cast<int>(VarType::Float));
        ASSERT_DOUBLE_EQ(m->GetVars()[2].data_.f, 2.3);
    });
}

TEST(jitter, multi_return_multi) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_multi_return_multi.lua", {.debug_mode = debug_mode});
        CVar ret;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret.type_, static_cast<int>(VarType::Multi));
        VarMulti *m = ret.data_.m;
        ASSERT_EQ(m->GetCount(), 5);
        ASSERT_EQ(m->GetVars()[0].type_, static_cast<int>(VarType::Int));
        ASSERT_EQ(m->GetVars()[0].data_.i, 1);
        ASSERT_EQ(m->GetVars()[1].type_, static_cast<int>(VarType::Int));
        ASSERT_EQ(m->GetVars()[1].data_.i, 2);
        ASSERT_EQ(m->GetVars()[2].type_, static_cast<int>(VarType::Int));
        ASSERT_EQ(m->GetVars()[2].data_.i, 3);
        ASSERT_EQ(m->GetVars()[3].type_, static_cast<int>(VarType::Int));
        ASSERT_EQ(m->GetVars()[3].data_.i, 4);
        ASSERT_EQ(m->GetVars()[4].type_, static_cast<int>(VarType::Int));
        ASSERT_EQ(m->GetVars()[4].data_.i, 5);
    });
}

TEST(jitter, multi_return_multi_ex) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_multi_return_multi_ex.lua", {.debug_mode = debug_mode});
        CVar ret;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret.type_, static_cast<int>(VarType::Multi));
        VarMulti *m = ret.data_.m;
        ASSERT_EQ(m->GetCount(), 4);
        ASSERT_EQ(m->GetVars()[0].type_, static_cast<int>(VarType::Int));
        ASSERT_EQ(m->GetVars()[0].data_.i, 1);
        ASSERT_EQ(m->GetVars()[1].type_, static_cast<int>(VarType::Int));
        ASSERT_EQ(m->GetVars()[1].data_.i, 2);
        ASSERT_EQ(m->GetVars()[2].type_, static_cast<int>(VarType::Int));
        ASSERT_EQ(m->GetVars()[2].data_.i, 3);
        ASSERT_EQ(m->GetVars()[3].type_, static_cast<int>(VarType::Int));
        ASSERT_EQ(m->GetVars()[3].data_.i, 6);
    });
}

TEST(jitter, multi_return_table_expand) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_multi_return_table_expand.lua", {.debug_mode = debug_mode});
        CVar ret;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret.type_, static_cast<int>(VarType::Table));
        VarTable *t = ret.data_.t;
        ASSERT_EQ(t->Size(), 4);
        ASSERT_EQ(t->Get(Var(int64_t(1))).GetInt(), 1);
        ASSERT_EQ(t->Get(Var(int64_t(2))).GetInt(), 2);
        ASSERT_EQ(t->Get(Var(int64_t(3))).GetInt(), 3);
        ASSERT_EQ(t->Get(Var(int64_t(4))).GetInt(), 4);
    });
}

TEST(jitter, multi_return_table_truncate) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_multi_return_table_truncate.lua", {.debug_mode = debug_mode});
        CVar ret;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret.type_, static_cast<int>(VarType::Table));
        VarTable *t = ret.data_.t;
        ASSERT_EQ(t->Size(), 4);
        ASSERT_EQ(t->Get(Var(int64_t(1))).GetInt(), 1);
        ASSERT_EQ(t->Get(Var(int64_t(2))).GetInt(), 2);
        ASSERT_EQ(t->Get(Var(int64_t(3))).GetInt(), 3);
        ASSERT_EQ(t->Get(Var(int64_t(4))).GetInt(), 5);
    });
}

TEST(jitter, multi_return_table_keyed_val) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_multi_return_table_keyed_val.lua", {.debug_mode = debug_mode});
        CVar ret;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret.type_, static_cast<int>(VarType::Table));
        VarTable *t = ret.data_.t;
        Var key_a;
        key_a.SetConstString(s, "a");
        ASSERT_EQ(t->Get(key_a).GetInt(), 3);
        Var key_b;
        key_b.SetConstString(s, "b");
        ASSERT_EQ(t->Get(key_b).type_, static_cast<int>(VarType::Nil));
    });
}

TEST(jitter, multi_return_table_keyed_exp) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_multi_return_table_keyed_exp.lua", {.debug_mode = debug_mode});
        CVar ret;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret.type_, static_cast<int>(VarType::Table));
        VarTable *t = ret.data_.t;
        ASSERT_EQ(t->Get(Var(int64_t(3))).GetString()->Str(), "hello");
        ASSERT_EQ(t->Get(Var(int64_t(4))).type_, static_cast<int>(VarType::Nil));
    });
}

TEST(jitter, multi_return_table_indexing) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_multi_return_table_indexing.lua", {.debug_mode = debug_mode});
        CVar ret;
        Call(s, type, "test_get", ret);
        ASSERT_EQ(ret.type_, static_cast<int>(VarType::StringId));
        ASSERT_EQ(ConstString::GetString(ret.data_.i), "val3");

        Call(s, type, "test_set", ret);
        ASSERT_EQ(ret.type_, static_cast<int>(VarType::Table));
        VarTable *t = ret.data_.t;
        ASSERT_EQ(t->Get(Var(int64_t(3))).GetString()->Str(), "inserted");
        ASSERT_EQ(t->Get(Var(int64_t(4))).type_, static_cast<int>(VarType::Nil));
    });
}

TEST(jitter, multi_return_expr_arith) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_multi_return_expr_arith.lua", {.debug_mode = debug_mode});
        CVar ret;

        // 加法
        Call(s, type, "test_plus", ret);
        ASSERT_EQ(ret.type_, static_cast<int>(VarType::Int));
        ASSERT_EQ(ret.data_.i, 15);// 5 + 10

        // 减法
        Call(s, type, "test_minus", ret);
        ASSERT_EQ(ret.type_, static_cast<int>(VarType::Int));
        ASSERT_EQ(ret.data_.i, -90);// 10 - 100

        // 等于比较
        Call(s, type, "test_compare_eq", ret);
        ASSERT_EQ(ret.type_, static_cast<int>(VarType::Bool));
        ASSERT_TRUE(ret.data_.b);// 10 == 10

        // 逻辑非
        Call(s, type, "test_logical_not", ret);
        ASSERT_EQ(ret.type_, static_cast<int>(VarType::Bool));
        ASSERT_FALSE(ret.data_.b);// not 10

        // 长度运算符
        Call(s, type, "test_unop_len", ret);
        ASSERT_EQ(ret.type_, static_cast<int>(VarType::Int));
        ASSERT_EQ(ret.data_.i, 5);// #"hello"

        // string concatenation
        Call(s, type, "test_concat", ret);
        ASSERT_EQ(ret.type_, static_cast<int>(VarType::String));
        ASSERT_EQ(reinterpret_cast<Var &>(ret).GetString()->Str(), "hello_suffix");

        // logical and
        Call(s, type, "test_logical_and", ret);
        ASSERT_EQ(ret.type_, static_cast<int>(VarType::StringId));
        ASSERT_EQ(ConstString::GetString(ret.data_.i), "yes");

        // logical or
        Call(s, type, "test_logical_or", ret);
        ASSERT_EQ(ret.type_, static_cast<int>(VarType::Bool));
        ASSERT_TRUE(ret.data_.b);
    });
}

TEST(jitter, multi_return_expr_control) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_multi_return_expr_control.lua", {.debug_mode = debug_mode});
        CVar ret;

        // if条件
        Call(s, type, "test_if", ret);
        ASSERT_EQ(ret.type_, static_cast<int>(VarType::Int));
        ASSERT_EQ(ret.data_.i, 1);

        // while条件
        Call(s, type, "test_while", ret);
        ASSERT_EQ(ret.type_, static_cast<int>(VarType::Int));
        ASSERT_EQ(ret.data_.i, 3);

        // for参数
        Call(s, type, "test_for", ret);
        ASSERT_EQ(ret.type_, static_cast<int>(VarType::Int));
        ASSERT_EQ(ret.data_.i, 15);// 1 + 2 + 3 + 4 + 5
    });
}

TEST(jitter, multi_return_expr_assign) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_multi_return_expr_assign.lua", {.debug_mode = debug_mode});
        CVar ret;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret.type_, static_cast<int>(VarType::Multi));
        VarMulti *m = ret.data_.m;
        ASSERT_EQ(m->GetCount(), 2);
        ASSERT_EQ(m->GetVars()[0].type_, static_cast<int>(VarType::Int));
        ASSERT_EQ(m->GetVars()[0].data_.i, 10);
        ASSERT_EQ(m->GetVars()[1].type_, static_cast<int>(VarType::Int));
        ASSERT_EQ(m->GetVars()[1].data_.i, 100);
    });
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

TEST(jitter, variadic_func) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_variadic_func.lua", {.debug_mode = debug_mode});
        CVar ret;
        Call(s, type, "run_test", ret);
        ASSERT_EQ(ret.type_, static_cast<int>(VarType::Multi));
        auto m = ret.data_.m;
        ASSERT_EQ(m->GetCount(), 2);
        ASSERT_EQ(m->GetVars()[0].type_, static_cast<int>(VarType::Int));
        ASSERT_EQ(m->GetVars()[0].data_.i, 10);
        ASSERT_EQ(m->GetVars()[1].type_, static_cast<int>(VarType::Int));
        ASSERT_EQ(m->GetVars()[1].data_.i, 20);
    });
}

TEST(jitter, variadic_func_with_params) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_variadic_func_with_params.lua", {.debug_mode = debug_mode});
        CVar ret;
        Call(s, type, "run_test", ret);
        ASSERT_EQ(ret.type_, static_cast<int>(VarType::Multi));
        auto m = ret.data_.m;
        ASSERT_EQ(m->GetCount(), 3);
        ASSERT_EQ(m->GetVars()[0].type_, static_cast<int>(VarType::Int));
        ASSERT_EQ(m->GetVars()[0].data_.i, 30);
        ASSERT_EQ(m->GetVars()[1].type_, static_cast<int>(VarType::Int));
        ASSERT_EQ(m->GetVars()[1].data_.i, 20);
        ASSERT_EQ(m->GetVars()[2].type_, static_cast<int>(VarType::Int));
        ASSERT_EQ(m->GetVars()[2].data_.i, 10);
    });
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
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_assign_variadic.lua", {.debug_mode = debug_mode});
        CVar ret;
        Call(s, type, "run_test", ret);
        ASSERT_EQ(ret.type_, static_cast<int>(VarType::Multi));
        auto m = ret.data_.m;
        ASSERT_EQ(m->GetCount(), 2);
        ASSERT_EQ(m->GetVars()[0].data_.i, 20);
        ASSERT_EQ(m->GetVars()[1].data_.i, 10);
    });
}

TEST(jitter, test_assign_variadic_no_match) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_assign_variadic_no_match.lua", {.debug_mode = debug_mode});
        CVar ret;
        Call(s, type, "run_test", ret);
        ASSERT_EQ(ret.type_, static_cast<int>(VarType::Multi));
        auto m = ret.data_.m;
        ASSERT_EQ(m->GetCount(), 2);
        ASSERT_EQ(m->GetVars()[0].data_.i, 1);
        ASSERT_EQ(m->GetVars()[1].data_.i, 10);
    });
}

TEST(jitter, test_assign_variadic_empty) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_assign_variadic_no_match.lua", {.debug_mode = debug_mode});
        CVar ret;
        Call(s, type, "run_empty_test", ret);
        ASSERT_EQ(ret.type_, static_cast<int>(VarType::Multi));
        auto m = ret.data_.m;
        ASSERT_EQ(m->GetCount(), 2);
        ASSERT_EQ(m->GetVars()[0].data_.i, 1);
        ASSERT_EQ(m->GetVars()[1].type_, static_cast<int>(VarType::Nil));
    });
}

TEST(jitter, test_const_table) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_const_table.lua", {.debug_mode = debug_mode});
        CVar ret;
        Call(s, type, "test", ret);

        ASSERT_EQ(ret.type_, static_cast<int>(VarType::Multi));
        VarMulti *m = ret.data_.m;
        ASSERT_EQ(m->GetCount(), 3);

        CVar ret1 = m->GetVars()[0];
        ASSERT_EQ(ret1.type_, static_cast<int>(VarType::Table));
        VarTable *t1 = ret1.data_.t;
        ASSERT_EQ(t1->Size(), 10);
        for (int i = 1; i <= 10; ++i) {
            ASSERT_EQ(t1->Get(Var(int64_t(i))).GetInt(), i);
        }

        CVar ret2 = m->GetVars()[1];
        ASSERT_EQ(ret2.type_, static_cast<int>(VarType::Table));
        VarTable *t2 = ret2.data_.t;
        ASSERT_EQ(t2->Size(), 3);
        Var key_a, key_b, key_c;
        key_a.SetConstString(s, "a");
        key_b.SetConstString(s, "b");
        key_c.SetConstString(s, "c");
        ASSERT_EQ(t2->Get(key_a).GetInt(), 1);
        ASSERT_EQ(t2->Get(key_b).GetInt(), 2);
        ASSERT_EQ(t2->Get(key_c).GetInt(), 3);

        CVar ret3 = m->GetVars()[2];
        ASSERT_EQ(ret3.type_, static_cast<int>(VarType::Table));
        VarTable *t3 = ret3.data_.t;
        ASSERT_EQ(t3->Size(), 5);
        ASSERT_EQ(t3->Get(Var(int64_t(1))).GetInt(), 1);
        ASSERT_EQ(t3->Get(Var(int64_t(2))).GetInt(), 3);
        ASSERT_EQ(t3->Get(Var(int64_t(3))).GetInt(), 5);
        Var key_d;
        key_d.SetConstString(s, "d");
        ASSERT_EQ(t3->Get(key_b).GetInt(), 2);
        ASSERT_EQ(t3->Get(key_d).GetInt(), 4);
    });
}

TEST(jitter, test_const_nested_table) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_const_nested_table.lua", {.debug_mode = debug_mode});
        CVar ret;
        Call(s, type, "test", ret);

        ASSERT_EQ(ret.type_, static_cast<int>(VarType::Table));
        VarTable *t = ret.data_.t;

        Var key_array;
        key_array.SetConstString(s, "array");
        Var val_array = t->Get(key_array);
        ASSERT_EQ(val_array.Type(), VarType::Table);
        VarTable *t_array = val_array.GetTable();
        ASSERT_EQ(t_array->Size(), 3);
        ASSERT_EQ(t_array->Get(Var(int64_t(1))).GetInt(), 1);
        ASSERT_EQ(t_array->Get(Var(int64_t(2))).GetInt(), 2);
        ASSERT_EQ(t_array->Get(Var(int64_t(3))).GetInt(), 3);

        Var key_map;
        key_map.SetConstString(s, "map");
        Var val_map = t->Get(key_map);
        ASSERT_EQ(val_map.Type(), VarType::Table);
        VarTable *t_map = val_map.GetTable();
        ASSERT_EQ(t_map->Size(), 3);
        Var key_a, key_b, key_c;
        key_a.SetConstString(s, "a");
        key_b.SetConstString(s, "b");
        key_c.SetConstString(s, "c");
        ASSERT_EQ(t_map->Get(key_a).GetInt(), 1);
        ASSERT_EQ(t_map->Get(key_b).GetInt(), 2);
        ASSERT_EQ(t_map->Get(key_c).GetInt(), 3);
    });
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
        ASSERT_EQ(t1->ViToString(0), "table:\n\t[1] = 1\n\t[2] = 2\n\t[3] = 3\n\t[4] = 4\n\t[5] = 5\n\t[6] = 6\n\t[7] = 7\n\t[8] = 8\n\t[9] = 9\n\t[10] = 10");
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
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_local_table_with_variadic.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "run_test", ret);
        ASSERT_EQ(ret, 30);
    });
}

TEST(jitter, test_local_table_with_variadic_no_end) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_local_table_with_variadic_no_end.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "run_test", ret);
        ASSERT_EQ(ret, 60);
    });
}

TEST(jitter, test_local_table_with_variadic_no_end_replace) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_local_table_with_variadic_no_end_replace.lua", {.debug_mode = debug_mode});
        CVar ret;
        Call(s, type, "run_test", ret);
        ASSERT_EQ(ret.type_, static_cast<int>(VarType::Multi));
        auto m = ret.data_.m;
        ASSERT_EQ(m->GetCount(), 2);
        ASSERT_EQ(m->GetVars()[0].data_.i, 30);
        ASSERT_EQ(m->GetVars()[1].data_.i, 20);
    });
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
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_const_define_simple_var.lua", {.debug_mode = debug_mode});
        int a = 0, b = 0, c = 0;
        Call(s, type, "get_a", a);
        Call(s, type, "get_b", b);
        Call(s, type, "get_c", c);
        ASSERT_EQ(a, 1);
        ASSERT_EQ(b, 1);
        ASSERT_EQ(c, 1);
    });
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
    FakeluaStateGuard sg;
    EXPECT_NO_THROW(CompileFile(sg.GetState(), "./jit/test_const_binop_plus.lua", {.debug_mode = true}));
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
    FakeluaStateGuard sg;
    EXPECT_NO_THROW(CompileFile(sg.GetState(), "./jit/test_const_binop_minus.lua", {.debug_mode = true}));
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
    FakeluaStateGuard sg;
    EXPECT_NO_THROW(CompileFile(sg.GetState(), "./jit/test_const_binop_star.lua", {.debug_mode = true}));
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
    FakeluaStateGuard sg;
    EXPECT_NO_THROW(CompileFile(sg.GetState(), "./jit/test_const_binop_slash.lua", {.debug_mode = true}));
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
    FakeluaStateGuard sg;
    EXPECT_NO_THROW(CompileFile(sg.GetState(), "./jit/test_const_binop_double_slash.lua", {.debug_mode = true}));
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
    FakeluaStateGuard sg;
    EXPECT_NO_THROW(CompileFile(sg.GetState(), "./jit/test_const_binop_pow.lua", {.debug_mode = true}));
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
    FakeluaStateGuard sg;
    EXPECT_NO_THROW(CompileFile(sg.GetState(), "./jit/test_const_binop_mod.lua", {.debug_mode = true}));
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
    FakeluaStateGuard sg;
    EXPECT_NO_THROW(CompileFile(sg.GetState(), "./jit/test_const_binop_bitand.lua", {.debug_mode = true}));
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
    FakeluaStateGuard sg;
    EXPECT_NO_THROW(CompileFile(sg.GetState(), "./jit/test_const_binop_xor.lua", {.debug_mode = true}));
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
    FakeluaStateGuard sg;
    EXPECT_NO_THROW(CompileFile(sg.GetState(), "./jit/test_const_binop_bitor.lua", {.debug_mode = true}));
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
    FakeluaStateGuard sg;
    EXPECT_NO_THROW(CompileFile(sg.GetState(), "./jit/test_const_binop_right_shift.lua", {.debug_mode = true}));
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
    FakeluaStateGuard sg;
    EXPECT_NO_THROW(CompileFile(sg.GetState(), "./jit/test_const_binop_left_shift.lua", {.debug_mode = true}));
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
    FakeluaStateGuard sg;
    EXPECT_NO_THROW(CompileFile(sg.GetState(), "./jit/test_const_binop_concat.lua", {.debug_mode = true}));
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
    FakeluaStateGuard sg;
    EXPECT_NO_THROW(CompileFile(sg.GetState(), "./jit/test_const_binop_less.lua", {.debug_mode = true}));
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
    FakeluaStateGuard sg;
    EXPECT_NO_THROW(CompileFile(sg.GetState(), "./jit/test_const_binop_less_equal.lua", {.debug_mode = true}));
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
    FakeluaStateGuard sg;
    EXPECT_NO_THROW(CompileFile(sg.GetState(), "./jit/test_const_binop_more.lua", {.debug_mode = true}));
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
    FakeluaStateGuard sg;
    EXPECT_NO_THROW(CompileFile(sg.GetState(), "./jit/test_const_binop_more_equal.lua", {.debug_mode = true}));
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
    FakeluaStateGuard sg;
    EXPECT_NO_THROW(CompileFile(sg.GetState(), "./jit/test_const_binop_equal.lua", {.debug_mode = true}));
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
    FakeluaStateGuard sg;
    EXPECT_NO_THROW(CompileFile(sg.GetState(), "./jit/test_const_binop_not_equal.lua", {.debug_mode = true}));
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
    FakeluaStateGuard sg;
    EXPECT_NO_THROW(CompileFile(sg.GetState(), "./jit/test_const_binop_and.lua", {.debug_mode = true}));
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
    FakeluaStateGuard sg;
    EXPECT_NO_THROW(CompileFile(sg.GetState(), "./jit/test_const_binop_or.lua", {.debug_mode = true}));
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
    FakeluaStateGuard sg;
    EXPECT_NO_THROW(CompileFile(sg.GetState(), "./jit/test_const_unop_minus.lua", {.debug_mode = true}));
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
    FakeluaStateGuard sg;
    EXPECT_NO_THROW(CompileFile(sg.GetState(), "./jit/test_const_unop_not.lua", {.debug_mode = true}));
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
    FakeluaStateGuard sg;
    EXPECT_NO_THROW(CompileFile(sg.GetState(), "./jit/test_const_unop_len.lua", {.debug_mode = true}));
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
    FakeluaStateGuard sg;
    EXPECT_NO_THROW(CompileFile(sg.GetState(), "./jit/test_const_unop_bitnot.lua", {.debug_mode = true}));
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
    JitterRunHelper([](State *s, JITType type, bool debug_mode) { CompileFile(s, "./jit/test_var_func_call.lua", {.debug_mode = debug_mode}); });
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
    JitterRunHelper([](State *s, JITType type, bool debug_mode) { EXPECT_NO_THROW(CompileFile(s, "./jit/test_for_loop_zero_step_dynamic.lua", {.debug_mode = debug_mode})); });
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

TEST(jitter, test_table_construct) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_table_construct.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test_table_construct", ret);
        ASSERT_EQ(ret, 6);
    });
}

TEST(jitter, test_math_specializations) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_math_specializations.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test_math_specializations", ret, -5);
        ASSERT_EQ(ret, 6);
    });
}

TEST(jitter, test_bitwise_float) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_bitwise_float.lua", {.debug_mode = debug_mode});
        int64_t ret = 0;
        Call(s, type, "test_bitwise_float", ret, 5.0);
        ASSERT_EQ(ret, 1);
    });
}

TEST(jitter, test_bitnot_float) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_bitnot_float.lua", {.debug_mode = debug_mode});
        int64_t ret = 0;
        Call(s, type, "test_bitnot_float", ret, 5.0);
        ASSERT_EQ(ret, ~5);
    });
}

TEST(jitter, test_dynamic_le) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_dynamic_le.lua", {.debug_mode = debug_mode});
        bool ret_b = false;
        Call(s, type, "test_dynamic_le", ret_b, 5, 6);
        ASSERT_TRUE(ret_b);
    });
}

TEST(jitter, test_more_coverage) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_more_coverage.lua", {.debug_mode = debug_mode});
        double ret = 0.0;
        Call(s, type, "test_more_coverage", ret, 5, 5.0);
        ASSERT_DOUBLE_EQ(ret, 41.0);
    });
}

TEST(jitter, test_shadow_coverage) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_shadow_coverage.lua", {.debug_mode = debug_mode});
        int64_t ret = 0;
        Call(s, type, "test_shadow_coverage", ret, 5);
        ASSERT_EQ(ret, 6);
    });
}

TEST(jitter, test_table_spec) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_table_spec.lua", {.debug_mode = debug_mode});
        int64_t ret = 0;
        Call(s, type, "test_table_spec", ret, 5);
        ASSERT_EQ(ret, 2);
    });
}

TEST(jitter, test_for_no_step) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_for_no_step.lua", {.debug_mode = debug_mode});
        int64_t ret = 0;
        Call(s, type, "test_for_no_step", ret, 5);
        ASSERT_EQ(ret, 15);
    });
}

TEST(jitter, test_spec_call_arg_count_error) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_spec_call_arg_count_error.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, test_set_table_arg_count_error) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_set_table_arg_count_error1.lua", {.debug_mode = true});
            },
            std::exception);
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_set_table_arg_count_error2.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, test_complete) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_complete.lua", {.debug_mode = debug_mode, .record_c_code = true});
        double ret_d = 0.0;
        Call(s, type, "test_complete", ret_d, 5, 5.0);
        ASSERT_DOUBLE_EQ(ret_d, 120.0);
    });
}

TEST(jitter, test_specs_helper) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_specs_helper.lua", {.debug_mode = debug_mode});
        double ret_d2 = 0.0;
        Call(s, type, "test_specs_helper", ret_d2, 5);
        ASSERT_DOUBLE_EQ(ret_d2, 112.0);
    });
}

TEST(jitter, test_dup_const_error) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_dup_const_error.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, test_dup_param_error) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_dup_param_error.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, test_shadow_const_error) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_shadow_const_error.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, test_native_binop) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_native_binop.lua", {.debug_mode = debug_mode});
        double ret_d = 0.0;
        Call(s, type, "test_entry", ret_d, 10, 3, 10.0, 3.0);
        ASSERT_DOUBLE_EQ(ret_d, 2.0);
    });
}

// --- Focused individual test cases (one scenario per test) ---

TEST(jitter, table_constructor_string_int_generic_keys) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_table_constructor_string_int_generic_keys.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, "a", 2, true);
        ASSERT_EQ(ret, 6);
    });
}

TEST(jitter, math_spec_dynamic_call_fallback) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_math_spec_dynamic_call_fallback.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, -5);
        ASSERT_EQ(ret, 6);
    });
}

TEST(jitter, math_spec_string_arg_call) {
    // Verifies that calling a math-specialized function with a string literal arg compiles correctly.
    // The function itself would fail at runtime (arithmetic on string), so we only verify compilation.
    JitterRunHelper([](State *s, JITType type, bool debug_mode) { CompileFile(s, "./jit/test_math_spec_string_arg_call.lua", {.debug_mode = debug_mode}); });
}

TEST(jitter, bitwise_and_on_float_param) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_bitwise_and_on_float_param.lua", {.debug_mode = debug_mode});
        int64_t ret = 0;
        Call(s, type, "test", ret, 5.0);
        ASSERT_EQ(ret, 1);
    });
}

TEST(jitter, bitnot_on_float_param) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_bitnot_on_float_param.lua", {.debug_mode = debug_mode});
        int64_t ret = 0;
        Call(s, type, "test", ret, 5.0);
        ASSERT_EQ(ret, ~5LL);
    });
}

TEST(jitter, dynamic_le_comparison) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_dynamic_le_comparison.lua", {.debug_mode = debug_mode});
        bool ret = false;
        Call(s, type, "test", ret, 5, 6);
        ASSERT_TRUE(ret);
        Call(s, type, "test", ret, 6, 5);
        ASSERT_FALSE(ret);
    });
}

TEST(jitter, table_field_access_in_arith) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_table_field_access_in_arith.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 5);
        ASSERT_EQ(ret, 6);
    });
}

TEST(jitter, global_const_int_in_expr) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_global_const_int_in_expr.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 3);
        ASSERT_EQ(ret, 45);
    });
}

TEST(jitter, bitnot_on_specialized_int) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_bitnot_on_specialized_int.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 5);
        ASSERT_EQ(ret, ~5);
    });
}

TEST(jitter, bitnot_on_specialized_float) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_bitnot_on_specialized_float.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 5.0);
        ASSERT_EQ(ret, ~5);
    });
}

TEST(jitter, shadow_local_in_do_block) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_shadow_local_in_do_block.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 99);
        ASSERT_EQ(ret, 6);
    });
}

TEST(jitter, table_dynamic_key_constructor) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_table_dynamic_key_constructor.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 7);
        ASSERT_EQ(ret, 2);
    });
}

TEST(jitter, for_loop_no_explicit_step) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_for_loop_no_explicit_step.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 5);
        ASSERT_EQ(ret, 15);
    });
}

TEST(jitter, spec_call_with_mixed_type_args) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_spec_call_with_mixed_type_args.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 5);
        ASSERT_EQ(ret, 11);
    });
}

TEST(jitter, spec_call_non_math_return) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_spec_call_non_math_return.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 5);
        ASSERT_EQ(ret, 10);
    });
}

TEST(jitter, spec_call_dynamic_return) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_spec_call_dynamic_return.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 5);
        ASSERT_EQ(ret, 10);
    });
}

TEST(jitter, spec_call_dynamic_arg) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_spec_call_dynamic_arg.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 5);
        ASSERT_EQ(ret, 10);
    });
}

TEST(jitter, bitwise_expr_as_cvar) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_bitwise_expr_as_cvar.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 5);
        ASSERT_EQ(ret, 5 & 1);
    });
}

TEST(jitter, upvalue_in_specialized_func) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_upvalue_in_specialized_func.lua", {.debug_mode = debug_mode});
        double ret = 0.0;
        Call(s, type, "test", ret, 5.0);
        ASSERT_DOUBLE_EQ(ret, 36.0);
    });
}

TEST(jitter, bool_return_from_spec_func) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_bool_return_from_spec_func.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 5);
        ASSERT_EQ(ret, 1);
        Call(s, type, "test", ret, -1);
        ASSERT_EQ(ret, 0);
    });
}

TEST(jitter, spec_func_returning_int) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_spec_func_returning_int.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 5);
        ASSERT_EQ(ret, 11);
    });
}

TEST(jitter, spec_func_returning_float) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_spec_func_returning_float.lua", {.debug_mode = debug_mode});
        double ret = 0.0;
        Call(s, type, "test", ret, 5.0);
        ASSERT_DOUBLE_EQ(ret, 11.0);
    });
}

TEST(jitter, set_table_generic_fallback) {
    std::vector<VarInterface *> tmp;
    auto newfunc = [&]() {
        auto ret = new SimpleVarImpl();
        tmp.push_back(ret);
        return ret;
    };
    JitterRunHelper([&](State *s, JITType type, bool debug_mode) {
        SetVarInterfaceNewFunc(s, newfunc);
        CompileFile(s, "./jit/test_set_table_generic_fallback.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, "mykey", 42);
        ASSERT_EQ(ret, 42);
    });
    for (auto &i: tmp) {
        delete i;
    }
}

TEST(jitter, fallback_assign_to_spec_var) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_fallback_assign_to_spec_var.lua", {.debug_mode = debug_mode});
        double ret = 0.0;
        Call(s, type, "test", ret, 5, 5.0);
        ASSERT_DOUBLE_EQ(ret, 30.0);
    });
}

TEST(jitter, arith_ops_in_spec_func) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_arith_ops_in_spec_func.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 3, 3.0);
        ASSERT_EQ(ret, 2);
    });
}

TEST(jitter, global_const_int_assign_to_local) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_global_const_int_assign_to_local.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 5);
        ASSERT_EQ(ret, 100);
    });
}

TEST(jitter, dynamic_for_loop_bounds) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_dynamic_for_loop_bounds.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 0);
        ASSERT_EQ(ret, 15);
    });
}

TEST(jitter, bool_expr_parens_and_not) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_bool_expr_parens_and_not.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 5);
        ASSERT_EQ(ret, 1);
        Call(s, type, "test", ret, -1);
        ASSERT_EQ(ret, 10);
    });
}

TEST(jitter, dynamic_le_with_spec_float) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_dynamic_le_with_spec_float.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 5.0);
        ASSERT_EQ(ret, 1);
    });
}

TEST(jitter, bitnot_on_dynamic_expr) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_bitnot_on_dynamic_expr.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 5);
        ASSERT_EQ(ret, ~5);
    });
}

TEST(jitter, duplicate_const_define_error) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_duplicate_const_define_error.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, duplicate_func_param_error) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_duplicate_func_param_error.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, shadow_global_const_error) {
    EXPECT_THROW(
            {
                FakeluaStateGuard sg;
                CompileFile(sg.GetState(), "./jit/test_shadow_global_const_error.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, test_math_spec_too_few_args) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) { EXPECT_THROW({ CompileFile(s, "./jit/test_math_spec_too_few_args.lua", {.debug_mode = debug_mode}); }, std::exception); });
}

TEST(jitter, test_shadow_typed_local) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_shadow_typed_local.lua", {.debug_mode = debug_mode});
        double ret = 0.0;
        Call(s, type, "test_shadow", ret, 1.0);
        EXPECT_DOUBLE_EQ(ret, 1.0);
    });
}

TEST(jitter, test_math_spec_float_global) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_math_spec_float_global.lua", {.debug_mode = debug_mode});
        double ret = 0.0;
        Call(s, type, "test_global", ret, 1.0);
        EXPECT_DOUBLE_EQ(ret, 2.5);
    });
}

TEST(jitter, test_math_spec_non_spec_call) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_math_spec_non_spec_call.lua", {.debug_mode = debug_mode});
        double ret = 0.0;
        Call(s, type, "test_call", ret, 1.0);
        EXPECT_DOUBLE_EQ(ret, 3.0);
        double ret2 = 0.0;
        Call(s, type, "test_call_non_native", ret2, 1.0);
        EXPECT_DOUBLE_EQ(ret2, 3.0);
    });
}

TEST(jitter, test_math_spec_non_bool_cond) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_math_spec_non_bool_cond.lua", {.debug_mode = debug_mode});
        double ret = 0.0;
        Call(s, type, "test_cond", ret, 1.0);
        EXPECT_DOUBLE_EQ(ret, 2.0);
    });
}

TEST(jitter, test_math_spec_unsupported_op) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_math_spec_unsupported_op.lua", {.debug_mode = debug_mode});
        double ret = 0.0;
        Call(s, type, "test_unsupported", ret, 1.0, 2.0, 3.0);
        EXPECT_DOUBLE_EQ(ret, 1.0);
    });
}

TEST(jitter, test_table_construct_unsupported_key) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_table_construct_unsupported_key.lua", {.debug_mode = debug_mode});
        double ret = 0.0;
        Call(s, type, "test_construct", ret, "abc");
        EXPECT_DOUBLE_EQ(ret, 42.0);
    });
}

TEST(jitter, test_math_spec_non_native_arg) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_math_spec_non_native_arg.lua", {.debug_mode = debug_mode});
        double ret = 0.0;
        Call(s, type, "test_sin", ret, 1.0, "abc");
        EXPECT_DOUBLE_EQ(ret, 4.0);
    });
}

TEST(jitter, test_math_spec_unsupported_unop) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_math_spec_unsupported_unop.lua", {.debug_mode = debug_mode});
        double ret = 0.0;
        Call(s, type, "test_unsupported_unop", ret, 1.0);
        EXPECT_DOUBLE_EQ(ret, 1.0);
    });
}

TEST(jitter, test_math_spec_args_not_list) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_math_spec_args_not_list.lua", {.debug_mode = debug_mode});
        double ret = 0.0;
        Call(s, type, "test_args_not_list", ret, 1.0);
        EXPECT_DOUBLE_EQ(ret, 1.0);
    });
}

TEST(jitter, test_math_call_non_spec_unsupported_arg) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_math_call_non_spec_unsupported_arg.lua", {.debug_mode = debug_mode});
        double ret = 0.0;
        Call(s, type, "test_non_spec_unsupported", ret, "abc");
        EXPECT_DOUBLE_EQ(ret, 3.0);
    });
}

// ============================================================================
// Lua 5.4 comprehensive compatibility tests
// Inspired by lua-5.4.7-tests/ patterns, adapted for FakeLua subset
// ============================================================================

TEST(jitter, test_operators) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_operators.lua", {.debug_mode = debug_mode});
        int64_t ret = 0;
        Call(s, type, "test_precedence", ret);
        EXPECT_EQ(ret, 1);
        Call(s, type, "test_logical", ret);
        EXPECT_EQ(ret, 1);
        Call(s, type, "test_string_concat", ret);
        EXPECT_EQ(ret, 1);
        Call(s, type, "test_complex_expr", ret);
        EXPECT_EQ(ret, 1);
        Call(s, type, "test_numeric_edge", ret);
        EXPECT_EQ(ret, 1);
        Call(s, type, "test_bitwise", ret);
        EXPECT_EQ(ret, 1);
    });
}

TEST(jitter, test_control_flow) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_control_flow.lua", {.debug_mode = debug_mode});
        int64_t ret = 0;
        Call(s, type, "test_nested_for", ret);
        EXPECT_EQ(ret, 1);
        Call(s, type, "test_repeat_break", ret);
        EXPECT_EQ(ret, 1);
        Call(s, type, "test_while_complex", ret);
        EXPECT_EQ(ret, 1);
        Call(s, type, "test_if_elseif_chain", ret);
        EXPECT_EQ(ret, 1);
        Call(s, type, "test_break_nested", ret);
        EXPECT_EQ(ret, 1);
        Call(s, type, "test_repeat_complex", ret);
        EXPECT_EQ(ret, 1);
        Call(s, type, "test_for_step", ret);
        EXPECT_EQ(ret, 1);
        Call(s, type, "test_for_break", ret);
        EXPECT_EQ(ret, 1);
        Call(s, type, "test_deep_if", ret);
        EXPECT_EQ(ret, 1);
    });
}

TEST(jitter, test_functions) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_functions.lua", {.debug_mode = debug_mode});
        int64_t ret = 0;
        Call(s, type, "test_tail_recursion", ret);
        EXPECT_EQ(ret, 1);
        Call(s, type, "test_local_function", ret);
        EXPECT_EQ(ret, 1);
        Call(s, type, "test_multi_params", ret);
        EXPECT_EQ(ret, 1);
        Call(s, type, "test_fibonacci", ret);
        EXPECT_EQ(ret, 1);
        Call(s, type, "test_gcd_func", ret);
        EXPECT_EQ(ret, 1);
        Call(s, type, "test_multi_return_via_table", ret);
        EXPECT_EQ(ret, 1);
        Call(s, type, "test_recursive_power", ret);
        EXPECT_EQ(ret, 1);
    });
}

TEST(jitter, test_tables) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_tables.lua", {.debug_mode = debug_mode});
        int64_t ret = 0;
        Call(s, type, "test_table_basic", ret);
        EXPECT_EQ(ret, 1);
        Call(s, type, "test_table_string_keys", ret);
        EXPECT_EQ(ret, 1);
        Call(s, type, "test_table_buildup", ret);
        EXPECT_EQ(ret, 1);
        Call(s, type, "test_table_mixed", ret);
        EXPECT_EQ(ret, 1);
        Call(s, type, "test_table_length", ret);
        EXPECT_EQ(ret, 1);
        Call(s, type, "test_table_sort", ret);
        EXPECT_EQ(ret, 1);
        Call(s, type, "test_table_matrix", ret);
        EXPECT_EQ(ret, 1);
        Call(s, type, "test_table_sieve", ret);
        EXPECT_EQ(ret, 1);
        Call(s, type, "test_table_fib", ret);
        EXPECT_EQ(ret, 1);
        Call(s, type, "test_table_concat", ret);
        EXPECT_EQ(ret, 1);
        Call(s, type, "test_table_nested", ret);
        EXPECT_EQ(ret, 1);
    });
}

TEST(jitter, test_edge_cases) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_edge_cases.lua", {.debug_mode = debug_mode});
        int64_t ret = 0;
        Call(s, type, "test_mixed_arithmetic", ret);
        EXPECT_EQ(ret, 1);
        Call(s, type, "test_integer_wrap", ret);
        EXPECT_EQ(ret, 1);
        Call(s, type, "test_chained_comparison", ret);
        EXPECT_EQ(ret, 1);
        Call(s, type, "test_complex_logical", ret);
        EXPECT_EQ(ret, 1);
        Call(s, type, "test_local_scope", ret);
        EXPECT_EQ(ret, 1);
        Call(s, type, "test_for_edge", ret);
        EXPECT_EQ(ret, 1);
        Call(s, type, "test_string_basic", ret);
        EXPECT_EQ(ret, 1);
        Call(s, type, "test_ternary_pattern", ret);
        EXPECT_EQ(ret, 1);
        Call(s, type, "test_destructuring", ret);
        EXPECT_EQ(ret, 1);
        Call(s, type, "test_operator_stress", ret);
        EXPECT_EQ(ret, 1);
        Call(s, type, "test_sum_digits_func", ret);
        EXPECT_EQ(ret, 1);
        Call(s, type, "test_reverse_number_func", ret);
        EXPECT_EQ(ret, 1);
    });
}

TEST(jitter, test_32params) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_32params.lua", {.debug_mode = debug_mode});
        int64_t ret = 0;
        Call(s, type, "test_32params", ret, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1);
        ASSERT_EQ(ret, 32);
    });
}

// ===========================================================================
// C++ 直接调用变参 Lua 函数的测试
//
// 约定：
//   - vararg 参数直接传递，Call() 自动打包
//   - 多返回值用 std::tie 自动解包
//   - 调用使用 Call 而非 FakeluaCallByName
// ===========================================================================

// 场景1：纯变参求和——空变参返回 0
TEST(jitter, vararg_from_cpp_sum_no_args) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_vararg_sum.lua", {.debug_mode = debug_mode});
        int64_t ret = 0;
        Call(s, type, "sum", ret);
        ASSERT_EQ(ret, 0);
    });
}

TEST(jitter, vararg_from_cpp_sum_one_arg) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_vararg_sum.lua", {.debug_mode = debug_mode});
        int64_t ret = 0;
        Call(s, type, "sum", ret, 42LL);
        ASSERT_EQ(ret, 42);
    });
}

TEST(jitter, vararg_from_cpp_sum_multi_args) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_vararg_sum.lua", {.debug_mode = debug_mode});
        int64_t ret = 0;
        Call(s, type, "sum", ret, 10LL, 20LL, 30LL);
        ASSERT_EQ(ret, 60);
    });
}

TEST(jitter, vararg_from_cpp_prefix_and_vararg) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_vararg_prefix_and_vararg.lua", {.debug_mode = debug_mode});
        int64_t a = 0, b = 0, c = 0;
        Call(s, type, "prefix_and_vararg", std::tie(a, b, c), 1LL, 2LL, 3LL);
        ASSERT_EQ(a, 1);
        ASSERT_EQ(b, 2);
        ASSERT_EQ(c, 3);
    });
}

TEST(jitter, vararg_from_cpp_prefix_only) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_vararg_prefix_and_vararg.lua", {.debug_mode = debug_mode});
        int64_t a = 0;
        Call(s, type, "prefix_and_vararg", std::tie(a), 99LL);
        ASSERT_EQ(a, 99);
    });
}

TEST(jitter, vararg_from_cpp_or_default_empty) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_vararg_or_default.lua", {.debug_mode = debug_mode});
        int64_t ret = 0;
        Call(s, type, "vararg_or_default", ret);
        ASSERT_EQ(ret, -1);
    });
}

TEST(jitter, vararg_from_cpp_or_default_with_arg) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_vararg_or_default.lua", {.debug_mode = debug_mode});
        int64_t ret = 0;
        Call(s, type, "vararg_or_default", ret, 7LL);
        ASSERT_EQ(ret, 7);
    });
}

TEST(jitter, test_const_complex_expr) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_const_complex_expr.lua", {.debug_mode = debug_mode});
        CVar ret;
        Call(s, type, "test", ret);

        ASSERT_EQ(ret.type_, static_cast<int>(VarType::Multi));
        VarMulti *m = ret.data_.m;
        ASSERT_EQ(m->GetCount(), 4);

        // a = 30
        ASSERT_EQ(m->GetVars()[0].type_, static_cast<int>(VarType::Int));
        ASSERT_EQ(m->GetVars()[0].data_.i, 30);

        // b = -30
        ASSERT_EQ(m->GetVars()[1].type_, static_cast<int>(VarType::Int));
        ASSERT_EQ(m->GetVars()[1].data_.i, -30);

        // c = -20
        ASSERT_EQ(m->GetVars()[2].type_, static_cast<int>(VarType::Int));
        ASSERT_EQ(m->GetVars()[2].data_.i, -20);

        // d = { val = -20 }
        ASSERT_EQ(m->GetVars()[3].type_, static_cast<int>(VarType::Table));
        VarTable *t = m->GetVars()[3].data_.t;
        ASSERT_EQ(t->Size(), 1);
        Var key_val;
        key_val.SetConstString(s, "val");
        ASSERT_EQ(t->Get(key_val).GetInt(), -20);
    });
}

// ============================================================================
// goto/label 合法场景
// ============================================================================

TEST(jitter, goto_forward_skip) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_goto_forward_skip.lua", {.debug_mode = debug_mode});
        int64_t ret = 0;
        Call(s, type, "test_goto_forward_skip", ret);
        ASSERT_EQ(ret, 100);
    });
}

TEST(jitter, goto_backward_loop) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_goto_backward_loop.lua", {.debug_mode = debug_mode});
        int64_t ret = 0;
        Call(s, type, "test_goto_backward_loop", ret);
        ASSERT_EQ(ret, 5);
    });
}

TEST(jitter, goto_label_only) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_goto_label_only.lua", {.debug_mode = debug_mode});
        int64_t ret = 0;
        Call(s, type, "test_label_no_goto", ret);
        ASSERT_EQ(ret, 42);
    });
}

TEST(jitter, goto_continue_simulation) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_goto_continue.lua", {.debug_mode = debug_mode});
        int64_t ret = 0;
        Call(s, type, "test_goto_continue", ret);
        // 奇数之和 1+3+5+7+9 = 25
        ASSERT_EQ(ret, 25);
    });
}

TEST(jitter, goto_break_out_of_nested_loops) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_goto_break_out.lua", {.debug_mode = debug_mode});
        int64_t ret = 0;
        Call(s, type, "test_goto_break_out", ret);
        ASSERT_EQ(ret, 21);
    });
}

TEST(jitter, goto_in_if_else) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_goto_in_if_else.lua", {.debug_mode = debug_mode});
        int64_t ret = 0;
        Call(s, type, "test_goto_in_if_else", ret);
        ASSERT_EQ(ret, 0);
    });
}

TEST(jitter, test_table_negative_int_key) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_table_negative_int_key.lua", {.debug_mode = debug_mode});
        int64_t ret = 0;
        Call(s, type, "test_negative_int_key", ret);
        ASSERT_EQ(ret, 1);
        Call(s, type, "test_negative_int_key_write", ret);
        ASSERT_EQ(ret, 1);
        Call(s, type, "test_negative_int_key_mixed", ret);
        ASSERT_EQ(ret, 1);
    });
}
