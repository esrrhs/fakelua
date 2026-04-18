#include "compile/compiler.h"
#include "fakelua.h"
#include "var/var_string.h"
#include "var/var_table.h"
#include "gtest/gtest.h"

using namespace fakelua;

static void InferRunHelper(const std::function<void(State *, JITType, bool)> &f) {
    const auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    for (const auto type: {JIT_TCC, JIT_GCC}) {
        f(s, type, true);
        f(s, type, false);
    }
    FakeluaDeleteState(s);
}

// Full inference success: all ints, constant for-loop bounds give T_INT loop var.
// sum(1..10) = 55; i and sum must stay T_INT throughout.
TEST(infer, test_infer_typed_int_for) {
    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_typed_int_for.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 55);
    });
}

// Full inference success: float literal propagates as T_FLOAT through addition.
TEST(infer, test_infer_typed_float_local) {
    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_typed_float_local.lua", {.debug_mode = debug_mode});
        double ret = 0;
        Call(s, type, "test", ret);
        ASSERT_NEAR(ret, 1.5, 0.001);
    });
}

// Full inference success: INT + FLOAT promotes the result to T_FLOAT.
TEST(infer, test_infer_int_float_promotion) {
    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_int_float_promotion.lua", {.debug_mode = debug_mode});
        double ret = 0;
        Call(s, type, "test", ret);
        ASSERT_NEAR(ret, 4.5, 0.001);
    });
}

// Mid-way degradation: a function-call result is T_DYNAMIC, so the binop
// and the local variable degrade to T_DYNAMIC and use CVar arithmetic.
TEST(infer, test_infer_degrade_func_call) {
    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_degrade_func_call.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 3);
    });
}

// Mid-way degradation: a parameter-bound for loop makes i T_DYNAMIC, which
// causes sum to degrade from T_INT to T_DYNAMIC during the loop body.
// The compiler must still produce correct CVar arithmetic for the fallback path.
TEST(infer, test_infer_degrade_param) {
    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_degrade_param.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 10);
        ASSERT_EQ(ret, 55);
    });
}

// Stable inference: reassigning an int variable with another int expression
// keeps MergeType(T_INT, T_INT) == T_INT; the declaration stays int64_t.
TEST(infer, test_infer_reassign_stable) {
    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_reassign_stable.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 3);
    });
}

// ForLoop with explicit integer step: begin=1, end=9, step=2 are all T_INT,
// so the loop variable stays T_INT and the typed int64_t for-loop path is used.
// Exercises the  !ExpStep() || ExpStep()->EvalType() == T_INT  branch.
TEST(infer, test_infer_for_step_int) {
    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_for_step_int.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 25); // 1+3+5+7+9
    });
}

// ForLoop with a T_DYNAMIC step (parameter): even though begin and end are T_INT,
// the step is T_DYNAMIC so all_int=false, the loop variable is T_DYNAMIC, and the
// CVar for-loop path is taken.  The accumulator degrades mid-way.
TEST(infer, test_infer_for_step_dynamic) {
    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_for_step_dynamic.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 2);
        ASSERT_EQ(ret, 25); // 1+3+5+7+9 with step=2
    });
}

// MergeType(T_INT, T_FLOAT) = T_DYNAMIC: x starts T_INT then is reassigned a
// float literal, which hits the different-non-unknown-types branch and degrades
// x to T_DYNAMIC.  The post-pass updates the declaration.
TEST(infer, test_infer_reassign_int_to_float) {
    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_reassign_int_to_float.lua", {.debug_mode = debug_mode});
        double ret = 0;
        Call(s, type, "test", ret);
        ASSERT_NEAR(ret, 1.5, 0.001);
    });
}

// If body processed with new_scope=true: the assignment x = n inside the if block
// calls env_.Update, which walks outer scopes and degrades x from T_INT to T_DYNAMIC.
TEST(infer, test_infer_if_scope_degrade) {
    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_if_scope_degrade.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 2);
        ASSERT_EQ(ret, 2);
        Call(s, type, "test", ret, 0);
        ASSERT_EQ(ret, 5);
    });
}

// While body processed with new_scope=true: the assignment x = n inside the while
// loop body degrades x from T_INT to T_DYNAMIC across the scope boundary.
TEST(infer, test_infer_while_scope_degrade) {
    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_while_scope_degrade.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 0);
        ASSERT_EQ(ret, 1); // loop ran: n=0→x=0, n=1→x=1, n=2 exits
        Call(s, type, "test", ret, 3);
        ASSERT_EQ(ret, 5); // loop skipped: x unchanged
    });
}

// Type pollution: a is initially T_INT but later mutated to "hello" (T_DYNAMIC).
// b = a + 5 captures a's current value (10) at declaration time; the compiler
// must still emit valid C even though a becomes a CVar.  b == 15.
TEST(infer, test_infer_type_pollution) {
    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_type_pollution.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 15);
    });
}

// Bottom-up / mixed-type: pure numeric expression using *, - and // (not PLUS)
// forces T_DYNAMIC, as does a call to unknown_func().  Both are compiled via
// CVar arithmetic.  dynamic_res = 100 + unknown_func() * 2 = 110.
TEST(infer, test_infer_bottom_up) {
    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_bottom_up.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 110);
    });
}

// do...end shadowing: inner `local val = "inner"` uses the `local` keyword and
// therefore creates a brand-new variable scoped to the do...end block; it does
// NOT mutate the outer val = 100.  After the block res = val + 1 = 101.
TEST(infer, test_infer_shadowing) {
    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_shadowing.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 101);
    });
}

// Cross-scope mutation: `state = "error"` (no local!) inside an if block deep
// inside a for loop mutates the outer state, degrading it from T_INT to
// T_DYNAMIC.  After the loop state holds the string "error".
TEST(infer, test_infer_mutation) {
    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_mutation.lua", {.debug_mode = debug_mode});
        std::string ret;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, "error");
    });
}

// Assign-to-CVar via stale T_INT EvalType: sum is initialised as T_INT but
// degraded to T_DYNAMIC by  sum = "done"  in an else branch that the
// single-pass walk sees AFTER recording EvalType = T_INT on the assign node
// for  sum = sum + i  in the then branch.  Without the fix CGen emits
//   CVar sum = (int64_t expression)  -- a C type error.
// With the fix typed_native_vars_ is consulted instead of EvalType.
TEST(infer, test_infer_assign_degraded_var) {
    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_assign_degraded_var.lua", {.debug_mode = debug_mode});
        std::string ret;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, "done");
    });
}

// Return with stale T_INT EvalType: the return expression has EvalType = T_INT
// (set by the single-pass walk before a later T_DYNAMIC mutation appears in
// source), but the variable is declared as CVar.  In subsequent loop
// iterations where the variable already holds a string, the old code emitted
//   return (CVar){VAR_INT, x.data_.i}  -- returning a garbage integer.
// With the fix CompileExp is always used so the CVar is returned directly.
TEST(infer, test_infer_return_stale_type) {
    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_return_stale_type.lua", {.debug_mode = debug_mode});
        std::string ret;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, "modified");
    });
}

// InferPrefixExp "exp" branch: (a + 2) is a PrefixExp of type "exp" inside an
// Exp of type "prefixexp".  InferPrefixExp delegates to InferNode on the inner
// expression, yielding T_INT + T_INT = T_INT, so b is specialised as int64_t.
TEST(infer, test_infer_paren_exp) {
    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_paren_exp.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 5);
    });
}
