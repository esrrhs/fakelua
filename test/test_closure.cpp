#include "compile/compiler.h"
#include "fakelua.h"
#include "gtest/gtest.h"

#include <lua.hpp>
#include <functional>

using namespace fakelua;

static std::vector<JITType> GetSupportedJitTypes() {
    return {JIT_TCC, JIT_GCC};
}

static void ClosureRunHelper(const std::function<void(State *, JITType, bool)> &f) {
    const auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    for (const auto type: GetSupportedJitTypes()) {
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
