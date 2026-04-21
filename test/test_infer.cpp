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

// Compile a Lua file with record_c_code=true (both JIT backends disabled so the
// step is cheap) and return the recorded globals+decls+impls portion.  The
// returned string is also printed to stdout so that CI logs show the actual C
// output for every infer test case.
static std::string InferGetCCode(const std::string &lua_file) {
    const auto s = FakeluaNewState();
    EXPECT_NE(s, nullptr);
    CompileConfig cfg;
    cfg.debug_mode = false;
    cfg.record_c_code = true;
    cfg.disable_jit[JIT_TCC] = true;
    cfg.disable_jit[JIT_GCC] = true;
    CompileFile(s, lua_file, cfg);
    const auto code = GetLastRecordedCCode(s);
    FakeluaDeleteState(s);
    // Print so CI logs can be inspected even without a debugger.
    std::cout << "\n=== C code for " << lua_file << " ===\n" << code << "=== end ===\n";
    return code;
}

// Full inference success: all ints, constant for-loop bounds give T_INT loop var.
// sum(1..10) = 55; i and sum must stay T_INT throughout.
TEST(infer, test_infer_typed_int_for) {
    const auto code = InferGetCCode("./infer/test_infer_typed_int_for.lua");
    // Both the accumulator and the loop variable must be declared as int64_t.
    ASSERT_NE(code.find("int64_t sum = 0;"), std::string::npos);
    ASSERT_NE(code.find("int64_t i = flua_for_ctrl_"), std::string::npos);
    // Native integer addition — no OpAdd macro.
    ASSERT_NE(code.find("sum = ((sum) + (i));"), std::string::npos);
    // No dynamic (CVar) accumulator declaration.
    ASSERT_EQ(code.find("CVar sum"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_typed_int_for.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 55);
    });
}

// Full inference success: float literal propagates as T_FLOAT through addition.
TEST(infer, test_infer_typed_float_local) {
    const auto code = InferGetCCode("./infer/test_infer_typed_float_local.lua");
    // Both locals must be typed as double.
    ASSERT_NE(code.find("double x = 1;"), std::string::npos);
    ASSERT_NE(code.find("double y = ((x) + (0.5));"), std::string::npos);
    // No CVar declarations for x or y.
    ASSERT_EQ(code.find("CVar x"), std::string::npos);
    ASSERT_EQ(code.find("CVar y"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_typed_float_local.lua", {.debug_mode = debug_mode});
        double ret = 0;
        Call(s, type, "test", ret);
        ASSERT_NEAR(ret, 1.5, 0.001);
    });
}

// Full inference success: INT + FLOAT promotes the result to T_FLOAT.
TEST(infer, test_infer_int_float_promotion) {
    const auto code = InferGetCCode("./infer/test_infer_int_float_promotion.lua");
    // a is T_INT, b is T_FLOAT (promoted from int + float literal).
    ASSERT_NE(code.find("int64_t a = 3;"), std::string::npos);
    ASSERT_NE(code.find("double b = ((a) + (1.5));"), std::string::npos);
    ASSERT_EQ(code.find("CVar a"), std::string::npos);
    ASSERT_EQ(code.find("CVar b"), std::string::npos);

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
    const auto code = InferGetCCode("./infer/test_infer_degrade_func_call.lua");
    // x must be CVar (helper() is T_DYNAMIC).
    ASSERT_NE(code.find("CVar x = "), std::string::npos);
    // Dynamic arithmetic macro must be used.
    ASSERT_NE(code.find("OpAdd("), std::string::npos);

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
    const auto code = InferGetCCode("./infer/test_infer_degrade_param.lua");
    // Both sum and the loop variable i must be CVar.
    ASSERT_NE(code.find("CVar sum = "), std::string::npos);
    ASSERT_NE(code.find("CVar i = "), std::string::npos);
    // Dynamic for-loop path must be used (CVar control variables).
    ASSERT_NE(code.find("CVar flua_for_ctrl_"), std::string::npos);

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
    const auto code = InferGetCCode("./infer/test_infer_reassign_stable.lua");
    // x must stay int64_t throughout.
    ASSERT_NE(code.find("int64_t x = 1;"), std::string::npos);
    ASSERT_NE(code.find("x = ((x) + (2));"), std::string::npos);
    ASSERT_EQ(code.find("CVar x"), std::string::npos);

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
    const auto code = InferGetCCode("./infer/test_infer_for_step_int.lua");
    // Typed int64_t for-loop path.
    ASSERT_NE(code.find("int64_t sum = 0;"), std::string::npos);
    ASSERT_NE(code.find("int64_t i = flua_for_ctrl_"), std::string::npos);
    ASSERT_NE(code.find("int64_t flua_for_step_"), std::string::npos);
    ASSERT_EQ(code.find("CVar sum"), std::string::npos);
    ASSERT_EQ(code.find("CVar i"), std::string::npos);

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
    const auto code = InferGetCCode("./infer/test_infer_for_step_dynamic.lua");
    // CVar for-loop path (dynamic step).
    ASSERT_NE(code.find("CVar sum = "), std::string::npos);
    ASSERT_NE(code.find("CVar i = "), std::string::npos);
    ASSERT_NE(code.find("CVar flua_for_ctrl_"), std::string::npos);
    // No native int64_t for the accumulator or loop var.
    ASSERT_EQ(code.find("int64_t sum"), std::string::npos);
    ASSERT_EQ(code.find("int64_t i"), std::string::npos);

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
    const auto code = InferGetCCode("./infer/test_infer_reassign_int_to_float.lua");
    // x must be CVar because int→float crosses a type boundary.
    ASSERT_NE(code.find("CVar x = "), std::string::npos);
    ASSERT_EQ(code.find("int64_t x"), std::string::npos);
    ASSERT_EQ(code.find("double x"), std::string::npos);

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
    const auto code = InferGetCCode("./infer/test_infer_if_scope_degrade.lua");
    // x degraded to CVar because x = n (T_DYNAMIC param) inside the if body.
    ASSERT_NE(code.find("CVar x = "), std::string::npos);
    ASSERT_EQ(code.find("int64_t x"), std::string::npos);

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
    const auto code = InferGetCCode("./infer/test_infer_while_scope_degrade.lua");
    // x degraded to CVar because x = n (T_DYNAMIC param) inside the while body.
    ASSERT_NE(code.find("CVar x = "), std::string::npos);
    ASSERT_EQ(code.find("int64_t x"), std::string::npos);

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
    const auto code = InferGetCCode("./infer/test_infer_type_pollution.lua");
    // a is mutated to a string later, so it must be CVar.
    ASSERT_NE(code.find("CVar a = "), std::string::npos);
    // b = a + 5 is computed while a is still numeric; b stays T_INT.
    ASSERT_NE(code.find("int64_t b = "), std::string::npos);
    ASSERT_EQ(code.find("CVar b"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_type_pollution.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 15);
    });
}

// Bottom-up / mixed-type: pure numeric expression using *, - and // are all
// typed now: (10+20)*3.14-(50//2) → T_FLOAT.  dynamic_res uses unknown_func()
// which is T_DYNAMIC.  dynamic_res = 100 + unknown_func() * 2 = 110.
TEST(infer, test_infer_bottom_up) {
    const auto code = InferGetCCode("./infer/test_infer_bottom_up.lua");
    // math_res is now T_FLOAT (all arithmetic ops are typed).
    ASSERT_NE(code.find("double math_res = "), std::string::npos);
    ASSERT_EQ(code.find("CVar math_res"), std::string::npos);
    // dynamic_res must still be CVar (function call forces T_DYNAMIC).
    ASSERT_NE(code.find("CVar dynamic_res = "), std::string::npos);
    // Dynamic arithmetic macros must be used for dynamic_res.
    ASSERT_NE(code.find("OpAdd("), std::string::npos);

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
    const auto code = InferGetCCode("./infer/test_infer_shadowing.lua");
    // Outer val and res must be int64_t (int literal, never mutated).
    ASSERT_NE(code.find("int64_t val = 100;"), std::string::npos);
    ASSERT_NE(code.find("int64_t res = "), std::string::npos);
    // Inner val (inside do...end block) is a string → CVar.
    ASSERT_NE(code.find("CVar val = "), std::string::npos);

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
    const auto code = InferGetCCode("./infer/test_infer_mutation.lua");
    // state must be CVar (mutated to a string inside the for loop).
    ASSERT_NE(code.find("CVar state = "), std::string::npos);
    // The for loop bounds are all int, so the loop variable stays int64_t.
    ASSERT_NE(code.find("int64_t i = flua_for_ctrl_"), std::string::npos);
    ASSERT_EQ(code.find("int64_t state"), std::string::npos);

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
    const auto code = InferGetCCode("./infer/test_infer_assign_degraded_var.lua");
    // sum must be CVar (degraded by the "done" assignment).
    ASSERT_NE(code.find("CVar sum = "), std::string::npos);
    // The assignment in the then-branch must use OpAdd (CVar arithmetic), NOT
    // the native  sum = ((sum) + (i))  form that would be a C type error.
    ASSERT_NE(code.find("OpAdd("), std::string::npos);
    ASSERT_EQ(code.find("int64_t sum"), std::string::npos);

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
    const auto code = InferGetCCode("./infer/test_infer_return_stale_type.lua");
    // x must be CVar.
    ASSERT_NE(code.find("CVar x = "), std::string::npos);
    // The return statement must pass the CVar directly ("return x;"), NOT box it
    // with a VAR_INT wrapper which would silently corrupt a string payload.
    ASSERT_NE(code.find("return x;"), std::string::npos);
    // No return statement that boxes with VAR_INT (checks for the pattern that
    // would be emitted if the stale-EvalType bug were present).
    ASSERT_EQ(code.find("return (CVar){.type_ = VAR_INT, .data_.i = (int64_t)(x)"), std::string::npos);

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
    const auto code = InferGetCCode("./infer/test_infer_paren_exp.lua");
    // Both a and b must be int64_t.
    ASSERT_NE(code.find("int64_t a = 3;"), std::string::npos);
    ASSERT_NE(code.find("int64_t b = ((a) + (2));"), std::string::npos);
    ASSERT_EQ(code.find("CVar a"), std::string::npos);
    ASSERT_EQ(code.find("CVar b"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_paren_exp.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 5);
    });
}

// For-loop cursor/outer local same name (case1): outer a degrades to CVar after
// loop while cursor a remains native int in loop body.
TEST(infer, test_infer_for_shadow_case1) {
    const auto code = InferGetCCode("./infer/test_infer_for_shadow_case1.lua");
    ASSERT_NE(code.find("CVar a = "), std::string::npos); // outer a
    ASSERT_NE(code.find("int64_t sum = 0;"), std::string::npos);
    ASSERT_NE(code.find("int64_t a = flua_for_ctrl_"), std::string::npos); // loop cursor

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_for_shadow_case1.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 55);
    });
}

// For-loop cursor assigned string in body (case2): cursor must be CVar even
// when bounds are integer-specialized, and outer a remains independently mutable.
TEST(infer, test_infer_for_shadow_case2) {
    const auto code = InferGetCCode("./infer/test_infer_for_shadow_case2.lua");
    ASSERT_NE(code.find("CVar a = "), std::string::npos);
    ASSERT_EQ(code.find("int64_t a = flua_for_ctrl_"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_for_shadow_case2.lua", {.debug_mode = debug_mode});
        std::string ret;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, "test");
    });
}

// For-loop cursor/outer local same name (case3): outer numeric a must stay
// typed after loop and support native numeric assignment.
TEST(infer, test_infer_for_shadow_case3) {
    const auto code = InferGetCCode("./infer/test_infer_for_shadow_case3.lua");
    ASSERT_NE(code.find("int64_t a = 2;"), std::string::npos); // outer a
    ASSERT_NE(code.find("int64_t a = flua_for_ctrl_"), std::string::npos); // cursor a
    ASSERT_NE(code.find("a = ((a) + (1));"), std::string::npos); // outer post-loop assign

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_for_shadow_case3.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 3);
    });
}

// For-loop cursor assigned string in body (case4): inner cursor a is CVar while
// outer a remains int64_t and is still used natively after loop.
TEST(infer, test_infer_for_shadow_case4) {
    const auto code = InferGetCCode("./infer/test_infer_for_shadow_case4.lua");
    ASSERT_NE(code.find("int64_t a = 2;"), std::string::npos); // outer a
    ASSERT_NE(code.find("CVar a = (CVar){.type_ = VAR_INT, .data_.i = (int64_t)(flua_for_ctrl_"), std::string::npos); // cursor a
    ASSERT_NE(code.find("a = ((a) + (1));"), std::string::npos); // outer post-loop assign

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_for_shadow_case4.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 3);
    });
}

// Extra uncovered scenario: inner typed local `a` must not leak and pollute the
// outer dynamic `a` declaration/assignments after leaving do...end scope.
TEST(infer, test_infer_do_shadow_typed_over_dynamic) {
    const auto code = InferGetCCode("./infer/test_infer_do_shadow_typed_over_dynamic.lua");
    ASSERT_NE(code.find("CVar a = "), std::string::npos); // outer a
    ASSERT_NE(code.find("int64_t a = 1;"), std::string::npos); // inner a

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_do_shadow_typed_over_dynamic.lua", {.debug_mode = debug_mode});
        std::string ret;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, "after");
    });
}

// INT - INT = INT specialization: x=10, x=x-3 stays T_INT.
TEST(infer, test_infer_typed_int_minus) {
    const auto code = InferGetCCode("./infer/test_infer_typed_int_minus.lua");
    ASSERT_NE(code.find("int64_t x = 10;"), std::string::npos);
    ASSERT_NE(code.find("x = ((x) - (3));"), std::string::npos);
    ASSERT_EQ(code.find("CVar x"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_typed_int_minus.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 7);
    });
}

// INT * INT = INT specialization: x=3, y=x*4 stays T_INT.
TEST(infer, test_infer_typed_int_star) {
    const auto code = InferGetCCode("./infer/test_infer_typed_int_star.lua");
    ASSERT_NE(code.find("int64_t x = 3;"), std::string::npos);
    ASSERT_NE(code.find("int64_t y = ((x) * (4));"), std::string::npos);
    ASSERT_EQ(code.find("CVar x"), std::string::npos);
    ASSERT_EQ(code.find("CVar y"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_typed_int_star.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 12);
    });
}

// INT - FLOAT promotes to T_FLOAT: x=3 (T_INT), y=x-1.5 → T_FLOAT.
TEST(infer, test_infer_typed_float_minus) {
    const auto code = InferGetCCode("./infer/test_infer_typed_float_minus.lua");
    ASSERT_NE(code.find("int64_t x = 3;"), std::string::npos);
    ASSERT_NE(code.find("double y = ((x) - (1.5));"), std::string::npos);
    ASSERT_EQ(code.find("CVar x"), std::string::npos);
    ASSERT_EQ(code.find("CVar y"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_typed_float_minus.lua", {.debug_mode = debug_mode});
        double ret = 0;
        Call(s, type, "test", ret);
        ASSERT_NEAR(ret, 1.5, 0.001);
    });
}

// FLOAT * INT promotes to T_FLOAT: x=2.5 (T_FLOAT), y=x*2 → T_FLOAT.
TEST(infer, test_infer_typed_float_star) {
    const auto code = InferGetCCode("./infer/test_infer_typed_float_star.lua");
    ASSERT_NE(code.find("double x = 2.5;"), std::string::npos);
    ASSERT_NE(code.find("double y = ((x) * (2));"), std::string::npos);
    ASSERT_EQ(code.find("CVar x"), std::string::npos);
    ASSERT_EQ(code.find("CVar y"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_typed_float_star.lua", {.debug_mode = debug_mode});
        double ret = 0;
        Call(s, type, "test", ret);
        ASSERT_NEAR(ret, 5.0, 0.001);
    });
}

// For-loop with STAR: sum = sum + i*2 uses PLUS and STAR, both T_INT.
// sum(2*(1..5)) = 30.
TEST(infer, test_infer_typed_int_for_star) {
    const auto code = InferGetCCode("./infer/test_infer_typed_int_for_star.lua");
    ASSERT_NE(code.find("int64_t sum = 0;"), std::string::npos);
    ASSERT_NE(code.find("int64_t i = flua_for_ctrl_"), std::string::npos);
    ASSERT_NE(code.find("sum = ((sum) + (((i) * (2))));"), std::string::npos);
    ASSERT_EQ(code.find("CVar sum"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_typed_int_for_star.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 30);
    });
}

// SLASH always produces T_FLOAT: 7/2 = 3.5.
TEST(infer, test_infer_typed_float_slash) {
    const auto code = InferGetCCode("./infer/test_infer_typed_float_slash.lua");
    ASSERT_NE(code.find("double x = "), std::string::npos);
    ASSERT_EQ(code.find("CVar x"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_typed_float_slash.lua", {.debug_mode = debug_mode});
        double ret = 0;
        Call(s, type, "test", ret);
        ASSERT_NEAR(ret, 3.5, 0.001);
    });
}

// POW always produces T_FLOAT: 2^10 = 1024.0.
TEST(infer, test_infer_typed_float_pow) {
    const auto code = InferGetCCode("./infer/test_infer_typed_float_pow.lua");
    ASSERT_NE(code.find("double x = "), std::string::npos);
    ASSERT_EQ(code.find("CVar x"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_typed_float_pow.lua", {.debug_mode = debug_mode});
        double ret = 0;
        Call(s, type, "test", ret);
        ASSERT_NEAR(ret, 1024.0, 0.001);
    });
}

// INT // INT = T_INT: 7//2 = 3 (floor division).
TEST(infer, test_infer_typed_int_double_slash) {
    const auto code = InferGetCCode("./infer/test_infer_typed_int_double_slash.lua");
    ASSERT_NE(code.find("int64_t x = "), std::string::npos);
    ASSERT_EQ(code.find("CVar x"), std::string::npos);
    // Must use the FlFloorDivInt helper.
    ASSERT_NE(code.find("FlFloorDivInt("), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_typed_int_double_slash.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 3);
    });
}

// INT % INT = T_INT: 7%3 = 1 (modulo).
TEST(infer, test_infer_typed_int_mod) {
    const auto code = InferGetCCode("./infer/test_infer_typed_int_mod.lua");
    ASSERT_NE(code.find("int64_t x = "), std::string::npos);
    ASSERT_EQ(code.find("CVar x"), std::string::npos);
    // Must use the FlModInt helper.
    ASSERT_NE(code.find("FlModInt("), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_typed_int_mod.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 1);
    });
}

// FLOAT // INT = T_FLOAT: 7.0//2 = 3.0 (floor division with float).
TEST(infer, test_infer_typed_float_double_slash) {
    const auto code = InferGetCCode("./infer/test_infer_typed_float_double_slash.lua");
    ASSERT_NE(code.find("double x = "), std::string::npos);
    ASSERT_EQ(code.find("CVar x"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_typed_float_double_slash.lua", {.debug_mode = debug_mode});
        double ret = 0;
        Call(s, type, "test", ret);
        ASSERT_NEAR(ret, 3.0, 0.001);
    });
}

// FLOAT % INT = T_FLOAT: 7.5%2 = 1.5 (modulo with float).
TEST(infer, test_infer_typed_float_mod) {
    const auto code = InferGetCCode("./infer/test_infer_typed_float_mod.lua");
    ASSERT_NE(code.find("double x = "), std::string::npos);
    ASSERT_EQ(code.find("CVar x"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_typed_float_mod.lua", {.debug_mode = debug_mode});
        double ret = 0;
        Call(s, type, "test", ret);
        ASSERT_NEAR(ret, 1.5, 0.001);
    });
}

// Negative floor division with Lua semantics: -7//2 = -4 (NOT -3).
TEST(infer, test_infer_typed_int_negative_floor_div) {
    const auto code = InferGetCCode("./infer/test_infer_typed_int_negative_floor_div.lua");
    ASSERT_NE(code.find("int64_t x = "), std::string::npos);
    ASSERT_EQ(code.find("CVar x"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_typed_int_negative_floor_div.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, -4);
    });
}

// Negative modulo with Lua semantics: -7%2 = 1 (NOT -1).
TEST(infer, test_infer_typed_int_negative_mod) {
    const auto code = InferGetCCode("./infer/test_infer_typed_int_negative_mod.lua");
    ASSERT_NE(code.find("int64_t x = "), std::string::npos);
    ASSERT_EQ(code.find("CVar x"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_typed_int_negative_mod.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 1);
    });
}

// ===== Numeric Specialization Tests =====

// fib(n) has one math param (n).  The compiler must generate fib_0 (int) and
// fib_1 (double) specializations, and the recursive calls inside fib_0 must
// be direct calls to fib_0 (not back through the entry dispatcher).
TEST(infer, test_spec_fib) {
    const auto code = InferGetCCode("./infer/test_spec_fib.lua");
    // Entry dispatcher must exist (original CVar signature).
    ASSERT_NE(code.find("CVar fib(CVar n)"), std::string::npos);
    // Two specialization declarations.
    ASSERT_NE(code.find("CVar fib_0(int64_t n)"), std::string::npos);
    ASSERT_NE(code.find("CVar fib_1(double n)"), std::string::npos);
    // Dispatcher must check type and call the right specialization.
    ASSERT_NE(code.find("fib_0(n.data_.i)"), std::string::npos);
    ASSERT_NE(code.find("fib_1(n.data_.f)"), std::string::npos);
    // Inside fib_0, recursive calls must go directly to fib_0 (not fib).
    // The pattern looks like: fib_0(((n) - (1)))
    ASSERT_NE(code.find("fib_0(((n) - (1)))"), std::string::npos);
    ASSERT_NE(code.find("fib_0(((n) - (2)))"), std::string::npos);
    // Inside fib_1, recursive calls must go directly to fib_1.
    ASSERT_NE(code.find("fib_1(((n) - (1)))"), std::string::npos);
    ASSERT_NE(code.find("fib_1(((n) - (2)))"), std::string::npos);
    // The entry dispatcher must NOT be called recursively from fib_0/fib_1.
    // Check: no FakeluaCallByName("fib") in the generated code.
    ASSERT_EQ(code.find("FakeluaCallByName(_S, FAKELUA_JIT_TYPE, \"fib\""), std::string::npos);

    // Functional verification: fib(10) == 55.
    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_spec_fib.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "fib", ret, 10);
        ASSERT_EQ(ret, 55);
    });
}

// test(a,b,c,d,e) has two math params: b (idx 1) and e (idx 4).
// The compiler must generate 4 specializations (b/e each int or float).
TEST(infer, test_spec_multi_param) {
    const auto code = InferGetCCode("./infer/test_spec_multi_param.lua");
    // All 4 specializations should be declared.
    ASSERT_NE(code.find("test_0_0"), std::string::npos);
    ASSERT_NE(code.find("test_1_0"), std::string::npos);
    ASSERT_NE(code.find("test_0_1"), std::string::npos);
    ASSERT_NE(code.find("test_1_1"), std::string::npos);
    // Entry function with CVar params.
    ASSERT_NE(code.find("CVar test(CVar a, CVar b, CVar c, CVar d, CVar e)"), std::string::npos);
    // int/int case: both b and e are int64_t in test_0_0.
    ASSERT_NE(code.find("test_0_0(CVar a, int64_t b"), std::string::npos);
    ASSERT_NE(code.find("test_1_1(CVar a, double b"), std::string::npos);

    // Functional verification: test(0, 3, 0, 0, 4) == 7.
    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_spec_multi_param.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 0, 3, 0, 0, 4);
        ASSERT_EQ(ret, 7);
    });
}

// test(n): n is accessed through a local alias x = n, and x * x is the
// arithmetic expression.  The analyzer must trace n → x and mark n as a
// math param.
TEST(infer, test_spec_wrapper_var) {
    const auto code = InferGetCCode("./infer/test_spec_wrapper_var.lua");
    // Specializations must be generated.
    ASSERT_NE(code.find("test_0(int64_t n)"), std::string::npos);
    ASSERT_NE(code.find("test_1(double n)"), std::string::npos);
    // Entry dispatcher must check type.
    ASSERT_NE(code.find("CVar test(CVar n)"), std::string::npos);

    // Functional verification: test(5) == 25.
    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_spec_wrapper_var.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 5);
        ASSERT_EQ(ret, 25);
    });
}
