#include "compile/compiler.h"
#include "fakelua.h"
#include "gtest/gtest.h"

#include <functional>
#include <lua.hpp>

using namespace fakelua;

static void ClosureRunHelper(const std::function<void(State *, JITType, bool)> &f) {
    const auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    for (const auto type: AllJitTypes()) {
        f(s, type, true);
        f(s, type, false);
    }
    FakeluaDeleteState(s);
}

TEST(closure, test_closure_counter) {
    ClosureRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./closure/test_closure_counter.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 239);
    });
}

TEST(closure, test_closure_shared) {
    ClosureRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./closure/test_closure_shared.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 500);
    });
}

TEST(closure, test_closure_loop) {
    ClosureRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./closure/test_closure_loop.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 150);
    });
}

TEST(closure, test_closure_nested) {
    ClosureRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./closure/test_closure_nested.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 34);
    });
}

TEST(closure, test_closure_recursion) {
    ClosureRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./closure/test_closure_recursion.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 120);
    });
}

TEST(closure, test_closure_higher_order) {
    ClosureRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./closure/test_closure_higher_order.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 30);
    });
}

TEST(closure, test_closure_for_in) {
    ClosureRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./closure/test_closure_for_in.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 60);
    });
}

TEST(closure, test_closure_syntax_expr) {
    ClosureRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./closure/test_closure_syntax_expr.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 185);
    });
}

TEST(closure, test_closure_method_call) {
    ClosureRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./closure/test_closure_method_call.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 200);
    });
}

TEST(closure, test_closure_func_alias) {
    ClosureRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./closure/test_closure_func_alias.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 90);
    });
}

TEST(closure, test_closure_generic_for_in) {
    ClosureRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./closure/test_closure_generic_for_in.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 180);
    });
}

TEST(closure, test_closure_tail_self) {
    ClosureRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./closure/test_closure_tail_self.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 210);
    });
}

TEST(closure, test_closure_tail_mutual) {
    ClosureRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./closure/test_closure_tail_mutual.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 2);
    });
}

TEST(closure, test_closure_tail_vararg) {
    ClosureRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./closure/test_closure_tail_vararg.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 66);
    });
}

TEST(closure, test_closure_tail_upvalue) {
    ClosureRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./closure/test_closure_tail_upvalue.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 62);
    });
}

TEST(closure, test_closure_tail_values) {
    ClosureRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./closure/test_closure_tail_values.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 3);
    });
}

// INTERP 专用：n=100000 若仍按 C 栈递归 InterpreterExecute，会在默认 8MiB 栈上溢出。
TEST(closure, test_closure_tail_deep_interp) {
    const auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig cfg;
    cfg.debug_mode = false;
    cfg.disable_jit[JIT_TCC] = true;
    cfg.disable_jit[JIT_GCC] = true;
    CompileFile(s, "./closure/test_closure_tail_deep.lua", cfg);
    int ret = 0;
    Call(s, JIT_INTERP, "test", ret, 100000);
    ASSERT_EQ(ret, 100000);
    FakeluaDeleteState(s);
}
