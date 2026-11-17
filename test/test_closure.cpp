#include "compile/compiler.h"
#include "fakelua.h"
#include "jit/vm.h"
#include "test_jit.h"
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

TEST(closure, opaque_fixed_arity_dispatch_four_through_thirty_two) {
    FakeluaStateGuard state;
    RegisterNativeFunction(state.GetState(), "opaque_closure", 1, false, [](State *, CVar *args, int) { return args[0]; });
    CompileFile(state.GetState(), "./closure/test_closure_fixed_arity_dispatch.lua", {});
    for (int arity = 4; arity <= 32; ++arity) {
        SCOPED_TRACE(arity);
        for (auto backend: AllJitTypes()) {
            SCOPED_TRACE(JitTypeName(backend));
            int64_t result = 0;
            Call(state.GetState(), backend, "dispatch_arity_" + std::to_string(arity), result);
            EXPECT_EQ(result, arity * (arity + 1) / 2);
        }
    }
}

TEST(closure, callbacks_preserve_arguments_across_backends) {
    FakeluaStateGuard state;
    CompileString(state.GetState(), R"lua(
        function make_fixed_callback()
            return function(a, b) return a + b end
        end
        function make_vararg_callback()
            return function(a, ...) return a + select("#", ...) end
        end
        function invoke_fixed_callback(fn) return fn(2, 3) end
        function invoke_vararg_callback(fn) return fn(2, 3, 4, 5) end
    )lua",
                  {});
    // Keep the closure alive in the same arena across the two low-level entry
    // calls; the convenience Call API intentionally resets temporary storage.
    for (auto producer: AllJitTypes()) {
        for (auto consumer: AllJitTypes()) {
            SCOPED_TRACE(::testing::Message() << "producer=" << JitTypeName(producer) << " consumer=" << JitTypeName(consumer));
            for (bool vararg: {false, true}) {
                SCOPED_TRACE(vararg);
                CVar callback = CallByNameArgs(state.GetState(), producer, vararg ? "make_vararg_callback" : "make_fixed_callback", 0, nullptr);
                CVar result = CallByNameArgs(state.GetState(), consumer, vararg ? "invoke_vararg_callback" : "invoke_fixed_callback", 1, &callback);
                EXPECT_EQ(inter::CVarToInteger(result, -1), 5);
            }
        }
    }
}

TEST(closure, captured_loop_values_survive_continue_and_multireturn_assignment) {
    FakeluaStateGuard state;
    RegisterNativeFunction(state.GetState(), "opaque_loop_value", 1, false, [](State *, CVar *args, int) { return args[0]; });
    CompileFile(state.GetState(), "./closure/test_closure_loop_capture.lua", {});
    for (auto backend: AllJitTypes()) {
        SCOPED_TRACE(JitTypeName(backend));
        int64_t result = 0;
        Call(state.GetState(), backend, "test_loop_capture_contracts", result);
        EXPECT_EQ(result, 1);
    }
}

TEST(test_closure, mutable_generic_loop_capture_continue_break_and_nested_forward_goto) {
    FakeluaStateGuard state;
    CompileFile(state.GetState(), "./closure/test_closure_mutable_generic_loop.lua", {});
    int64_t result = 0;
    CallAll(state.GetState(), "test_mutable_generic_loop", result);
    EXPECT_EQ(result, 1);
}

TEST(test_closure, generic_iterator_capture_and_multivalue_prefix_declarations) {
    FakeluaStateGuard state;
    CompileFile(state.GetState(), "./closure/test_closure_mutable_generic_loop.lua", {});
    int64_t result = 0;
    CallAll(state.GetState(), "test_generic_iterator_capture_and_multivalue_prefix_declarations", result);
    EXPECT_EQ(result, 1);
}
