#include "compile/compiler.h"
#include "fakelua.h"
#include "var/var_string.h"
#include "var/var_table.h"
#include "gtest/gtest.h"

using namespace fakelua;

static std::vector<JITType> GetSupportedJitTypes() {
    return {JIT_TCC, JIT_GCC};
}

static void InferRunHelper(const std::function<void(State *, JITType, bool)> &f) {
    const auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    for (const auto type: GetSupportedJitTypes()) {
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
    // With snapshot-based specialization n becomes a math param (int64_t / double).
    // int specialization: sum and i are fully typed as int64_t.
    ASSERT_NE(code.find("int64_t sum = 0"), std::string::npos);
    ASSERT_NE(code.find("int64_t i = "), std::string::npos);
    // float specialization: loop control vars become double, sum degrades to CVar.
    ASSERT_NE(code.find("CVar sum = "), std::string::npos);
    ASSERT_NE(code.find("double i = "), std::string::npos);
    ASSERT_NE(code.find("double flua_for_ctrl_"), std::string::npos);

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

// ForLoop with a math param step: snapshot-based specialization generates two
// specializations.  int specialization: all loop vars are int64_t, sum is int64_t.
// float specialization: loop ctrl vars become double, sum degrades to CVar because
// MergeType(T_INT, T_FLOAT) = T_DYNAMIC.
TEST(infer, test_infer_for_step_dynamic) {
    const auto code = InferGetCCode("./infer/test_infer_for_step_dynamic.lua");
    // int specialization: all typed.
    ASSERT_NE(code.find("int64_t sum = 0"), std::string::npos);
    ASSERT_NE(code.find("int64_t i = "), std::string::npos);
    // float specialization: loop ctrl vars become double, sum degrades to CVar.
    ASSERT_NE(code.find("CVar sum = "), std::string::npos);
    ASSERT_NE(code.find("double i = "), std::string::npos);
    ASSERT_NE(code.find("double flua_for_ctrl_"), std::string::npos);

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
// calls env_.Update, which walks outer scopes and may degrade x depending on the
// specialization bitmask.
// After comparison-based math-param detection, n > 0 causes n to be identified as a
// math param.  With numeric specialization:
// - test_0(int64_t n): x = n keeps x as T_INT (MergeType(T_INT,T_INT)=T_INT) → int64_t x
// - test_1(double n): x = n makes x T_DYNAMIC (MergeType(T_INT,T_FLOAT)=T_DYNAMIC) → CVar x
// The if condition n > 0 uses TryCompileNativeBoolExpr → native C comparison in both specs.
TEST(infer, test_infer_if_scope_degrade) {
    const auto code = InferGetCCode("./infer/test_infer_if_scope_degrade.lua");
    // Both specializations must exist.
    ASSERT_NE(code.find("test_0(int64_t n)"), std::string::npos);
    ASSERT_NE(code.find("test_1(double n)"), std::string::npos);
    ASSERT_NE(code.find("CVar test(CVar n)"), std::string::npos);
    // In the int specialization (test_0), x = n with n=T_INT keeps x as T_INT.
    ASSERT_NE(code.find("int64_t x"), std::string::npos);
    // In the float specialization (test_1), MergeType(T_INT, T_FLOAT) = T_DYNAMIC
    // so x degrades to CVar (is_degraded_literal path).
    ASSERT_NE(code.find("CVar x = "), std::string::npos);
    // Both specs must use native C comparison, not IsTrue.
    ASSERT_NE(code.find("(n) > (0)"), std::string::npos);

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
// loop body degrades x across the scope boundary.
// With numeric specialization, n is now a typed math param:
// - test_0(int64_t n): x = n keeps x as T_INT → int64_t x (no degradation in int path)
// - test_1(double n): x = n makes x T_FLOAT, merged with T_INT initial → T_DYNAMIC → CVar x
TEST(infer, test_infer_while_scope_degrade) {
    const auto code = InferGetCCode("./infer/test_infer_while_scope_degrade.lua");
    // In the float specialization (test_1), x degrades to CVar because x = n (T_FLOAT param).
    ASSERT_NE(code.find("CVar x = "), std::string::npos);
    // In the int specialization (test_0), x is correctly int64_t because x = n is T_INT throughout.
    ASSERT_NE(code.find("int64_t x"), std::string::npos);

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
    // Two specialization declarations — now return native types directly.
    ASSERT_NE(code.find("int64_t fib_0(int64_t n)"), std::string::npos);
    ASSERT_NE(code.find("double fib_1(double n)"), std::string::npos);
    // Dispatcher boxes the native result back into CVar.
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
    // No .data_.i extraction inside the spec bodies (no boxing/unboxing there).
    // The dispatcher itself still has .data_.i/.data_.f for the entry call.
    // Verify spec bodies use native add directly (no CVar wrapping of recursive results).
    ASSERT_NE(code.find("return ((flua_native_"), std::string::npos);

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

// ────────────────────────────────────────────────────────────────────────────
// Optimization 1: bitwise operators propagate T_INT when both operands are T_INT
// ────────────────────────────────────────────────────────────────────────────

// INT & INT = T_INT: 10 & 3 = 2.
TEST(infer, test_infer_typed_int_bitand) {
    const auto code = InferGetCCode("./infer/test_infer_typed_int_bitand.lua");
    ASSERT_NE(code.find("int64_t x = 10;"), std::string::npos);
    ASSERT_NE(code.find("int64_t y = ((int64_t)(x) & (int64_t)(3));"), std::string::npos);
    ASSERT_EQ(code.find("CVar x"), std::string::npos);
    ASSERT_EQ(code.find("CVar y"), std::string::npos);
    // Must NOT fall back to the dynamic OpBitAnd macro.
    ASSERT_EQ(code.find("OpBitAnd("), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_typed_int_bitand.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 2); // 10 & 3 = 2
    });
}

// INT | INT = T_INT: 12 | 3 = 15.
TEST(infer, test_infer_typed_int_bitor) {
    const auto code = InferGetCCode("./infer/test_infer_typed_int_bitor.lua");
    ASSERT_NE(code.find("int64_t x = 12;"), std::string::npos);
    ASSERT_NE(code.find("int64_t y = ((int64_t)(x) | (int64_t)(3));"), std::string::npos);
    ASSERT_EQ(code.find("CVar x"), std::string::npos);
    ASSERT_EQ(code.find("CVar y"), std::string::npos);
    ASSERT_EQ(code.find("OpBitOr("), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_typed_int_bitor.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 15); // 12 | 3 = 15
    });
}

// INT ~ INT = T_INT (XOR): 5 ~ 3 = 6.
TEST(infer, test_infer_typed_int_bitxor) {
    const auto code = InferGetCCode("./infer/test_infer_typed_int_bitxor.lua");
    ASSERT_NE(code.find("int64_t x = 5;"), std::string::npos);
    ASSERT_NE(code.find("int64_t y = ((int64_t)(x) ^ (int64_t)(3));"), std::string::npos);
    ASSERT_EQ(code.find("CVar x"), std::string::npos);
    ASSERT_EQ(code.find("CVar y"), std::string::npos);
    ASSERT_EQ(code.find("OpBitXor("), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_typed_int_bitxor.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 6); // 5 ^ 3 = 6
    });
}

// INT << INT = T_INT: 1 << 4 = 16.
TEST(infer, test_infer_typed_int_leftshift) {
    const auto code = InferGetCCode("./infer/test_infer_typed_int_leftshift.lua");
    ASSERT_NE(code.find("int64_t x = 1;"), std::string::npos);
    // CompileNumericExp now uses FlLShiftInt for Lua-correct clamping semantics.
    ASSERT_NE(code.find("FlLShiftInt((x), (4),"), std::string::npos);
    ASSERT_NE(code.find("int64_t y ="), std::string::npos);
    ASSERT_EQ(code.find("CVar x"), std::string::npos);
    ASSERT_EQ(code.find("CVar y"), std::string::npos);
    ASSERT_EQ(code.find("OpLeftShift("), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_typed_int_leftshift.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 16); // 1 << 4 = 16
    });
}

// INT >> INT = T_INT: 256 >> 3 = 32.
TEST(infer, test_infer_typed_int_rightshift) {
    const auto code = InferGetCCode("./infer/test_infer_typed_int_rightshift.lua");
    ASSERT_NE(code.find("int64_t x = 256;"), std::string::npos);
    // CompileNumericExp now uses FlRShiftInt for Lua-correct clamping semantics.
    ASSERT_NE(code.find("FlRShiftInt((x), (3),"), std::string::npos);
    ASSERT_NE(code.find("int64_t y ="), std::string::npos);
    ASSERT_EQ(code.find("CVar x"), std::string::npos);
    ASSERT_EQ(code.find("CVar y"), std::string::npos);
    ASSERT_EQ(code.find("OpRightShift("), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_typed_int_rightshift.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 32); // 256 >> 3 = 32
    });
}

// ────────────────────────────────────────────────────────────────────────────
// Optimization 2: unary operators propagate T_INT / T_FLOAT
// ────────────────────────────────────────────────────────────────────────────

// Unary minus on an integer variable yields T_INT → int64_t.
TEST(infer, test_infer_typed_int_unary_minus) {
    const auto code = InferGetCCode("./infer/test_infer_typed_int_unary_minus.lua");
    ASSERT_NE(code.find("int64_t n = 5;"), std::string::npos);
    ASSERT_NE(code.find("int64_t x = "), std::string::npos);
    // Must use the native negation form -(n), not the dynamic OpUnaryMinus macro.
    ASSERT_NE(code.find("(-(n))"), std::string::npos);
    ASSERT_EQ(code.find("CVar x"), std::string::npos);
    ASSERT_EQ(code.find("OpUnaryMinus("), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_typed_int_unary_minus.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, -5);
    });
}

// Unary minus on a float variable yields T_FLOAT → double.
TEST(infer, test_infer_typed_float_unary_minus) {
    const auto code = InferGetCCode("./infer/test_infer_typed_float_unary_minus.lua");
    ASSERT_NE(code.find("double x = "), std::string::npos);
    ASSERT_NE(code.find("(-(n))"), std::string::npos);
    ASSERT_EQ(code.find("CVar x"), std::string::npos);
    ASSERT_EQ(code.find("OpUnaryMinus("), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_typed_float_unary_minus.lua", {.debug_mode = debug_mode});
        double ret = 0;
        Call(s, type, "test", ret);
        ASSERT_NEAR(ret, -3.14, 0.001);
    });
}

// Bitwise NOT on an integer literal yields T_INT → int64_t. ~7 = -8.
TEST(infer, test_infer_typed_int_bitnot) {
    const auto code = InferGetCCode("./infer/test_infer_typed_int_bitnot.lua");
    ASSERT_NE(code.find("int64_t x = "), std::string::npos);
    ASSERT_NE(code.find("(~((int64_t)(7)))"), std::string::npos);
    ASSERT_EQ(code.find("CVar x"), std::string::npos);
    ASSERT_EQ(code.find("OpBitNot("), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_typed_int_bitnot.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, -8); // ~7 == -8 in two's complement
    });
}

// Unary minus on integer variable for-loop bounds enables the T_INT fast path.
// bound = 5; for i = -bound, bound: sum = -5+...+5 = 0.
TEST(infer, test_infer_unary_minus_for_bound) {
    const auto code = InferGetCCode("./infer/test_infer_unary_minus_for_bound.lua");
    ASSERT_NE(code.find("int64_t bound = 5;"), std::string::npos);
    ASSERT_NE(code.find("int64_t sum = 0;"), std::string::npos);
    ASSERT_NE(code.find("int64_t flua_for_ctrl_"), std::string::npos);
    ASSERT_NE(code.find("int64_t i = flua_for_ctrl_"), std::string::npos);
    // The lower bound must use native negation form via unop inference.
    ASSERT_NE(code.find("(-(bound))"), std::string::npos);
    ASSERT_EQ(code.find("CVar sum"), std::string::npos);
    ASSERT_EQ(code.find("CVar i"), std::string::npos);
    ASSERT_EQ(code.find("CVar flua_for_ctrl_"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_unary_minus_for_bound.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 0); // -5+...+5 = 0
    });
}

// ────────────────────────────────────────────────────────────────────────────
// Specialization: BITNOT (~) of a math param in arithmetic uses native fast path
// ────────────────────────────────────────────────────────────────────────────

// InferArgTypeForSpec handles BITNOT: when the operand is T_INT the result is
// T_INT, so (~n) + 1 uses the native C arithmetic path in the int specialization.
// For n=5: ~5 = -6 (int64_t bitwise NOT), -6 + 1 = -5.
TEST(infer, test_spec_bitnot_param) {
    const auto code = InferGetCCode("./infer/test_spec_bitnot_param.lua");
    // Both int and float specializations must be declared.
    ASSERT_NE(code.find("test_0(int64_t n)"), std::string::npos);
    ASSERT_NE(code.find("test_1(double n)"), std::string::npos);
    // The int specialization must use the native BITNOT form (not OpBitNot).
    // CompileNumericExp for BITNOT generates (~((int64_t)(n))).
    ASSERT_NE(code.find("(~((int64_t)(n)))"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_spec_bitnot_param.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 5);
        ASSERT_EQ(ret, -5); // ~5 = -6, -6 + 1 = -5
    });
}

// ────────────────────────────────────────────────────────────────────────────
// Optimization 3: T_FLOAT for-loop fast path (double control variables)
// ────────────────────────────────────────────────────────────────────────────

// All-float bounds: for i = 1.0, 3.0 → double fast path.
// sum = 1.0 + 2.0 + 3.0 = 6.0.
TEST(infer, test_infer_typed_float_for) {
    const auto code = InferGetCCode("./infer/test_infer_typed_float_for.lua");
    ASSERT_NE(code.find("double sum = "), std::string::npos);
    ASSERT_NE(code.find("double flua_for_ctrl_"), std::string::npos);
    ASSERT_NE(code.find("double i = flua_for_ctrl_"), std::string::npos);
    ASSERT_EQ(code.find("CVar sum"), std::string::npos);
    ASSERT_EQ(code.find("CVar i"), std::string::npos);
    // Must NOT use the CVar for-loop path (step_pos_var / cond_var).
    ASSERT_EQ(code.find("CVar flua_for_ctrl_"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_typed_float_for.lua", {.debug_mode = debug_mode});
        double ret = 0;
        Call(s, type, "test", ret);
        ASSERT_NEAR(ret, 6.0, 0.001); // 1.0+2.0+3.0
    });
}

// Float bounds with explicit float step: for i = 0.0, 1.0, 0.5.
// sum = 0.0 + 0.5 + 1.0 = 1.5.
TEST(infer, test_infer_typed_float_for_step) {
    const auto code = InferGetCCode("./infer/test_infer_typed_float_for_step.lua");
    ASSERT_NE(code.find("double sum = "), std::string::npos);
    ASSERT_NE(code.find("double flua_for_ctrl_"), std::string::npos);
    ASSERT_NE(code.find("double i = flua_for_ctrl_"), std::string::npos);
    ASSERT_EQ(code.find("CVar sum"), std::string::npos);
    ASSERT_EQ(code.find("CVar i"), std::string::npos);
    ASSERT_EQ(code.find("CVar flua_for_ctrl_"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_typed_float_for_step.lua", {.debug_mode = debug_mode});
        double ret = 0;
        Call(s, type, "test", ret);
        ASSERT_NEAR(ret, 1.5, 0.001); // 0.0+0.5+1.0
    });
}

// Mixed int/float bounds (begin=int, end=float): for i = 1, 5.0 → double fast path.
// sum = 1.0+2.0+3.0+4.0+5.0 = 15.0.
TEST(infer, test_infer_typed_float_for_mixed) {
    const auto code = InferGetCCode("./infer/test_infer_typed_float_for_mixed.lua");
    ASSERT_NE(code.find("double sum = "), std::string::npos);
    ASSERT_NE(code.find("double flua_for_ctrl_"), std::string::npos);
    ASSERT_NE(code.find("double i = flua_for_ctrl_"), std::string::npos);
    ASSERT_EQ(code.find("CVar sum"), std::string::npos);
    ASSERT_EQ(code.find("CVar i"), std::string::npos);
    ASSERT_EQ(code.find("CVar flua_for_ctrl_"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_typed_float_for_mixed.lua", {.debug_mode = debug_mode});
        double ret = 0;
        Call(s, type, "test", ret);
        ASSERT_NEAR(ret, 15.0, 0.001); // 1+2+3+4+5
    });
}

// CollectReassignedVars must descend into elseif blocks (type_inferencer.cpp lines 553-555).
// n is a math param; the function body has an if/elseif structure.
TEST(infer, test_spec_elseif_param) {
    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_spec_elseif_param.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 8);
        ASSERT_EQ(ret, 11); // n=8: r = 8+1=9, elseif branch: r = 9+2=11
    });
}

// CollectReassignedVars must descend into ForIn blocks (type_inferencer.cpp lines 574-578).
// n is a math param; the function body has a for-in loop that reassigns sum.
TEST(infer, test_spec_forin_param) {
    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_spec_forin_param.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 10);
        ASSERT_EQ(ret, 16); // sum = 10+0 + 1+2+3 = 16
    });
}

// ────────────────────────────────────────────────────────────────────────────
// Optimization: reassigned math params are still specialized
// ────────────────────────────────────────────────────────────────────────────

// GCD-style: both a and b are reassigned inside the while loop body
// (a = tmp; b = a % b). The compiler must still mark them as math params
// and generate int/float specializations.
TEST(infer, test_spec_reassign_gcd) {
    const auto code = InferGetCCode("./infer/test_spec_reassign_gcd.lua");
    // With two math params the suffixes are _0_0, _1_0, _0_1, _1_1.
    ASSERT_NE(code.find("test_0_0(int64_t a, int64_t b)"), std::string::npos);
    ASSERT_NE(code.find("test_1_1(double a, double b)"), std::string::npos);
    // Entry dispatcher must check type and route.
    ASSERT_NE(code.find("CVar test(CVar a, CVar b)"), std::string::npos);
    // In the all-int specialization, the reassignment of b must use native int64_t modulo.
    ASSERT_NE(code.find("FlModInt("), std::string::npos);
    // In the all-int specialization, while condition b != 0 must be a direct C comparison.
    ASSERT_NE(code.find("(b) != (0)"), std::string::npos);
    // No dynamic IsTrue for the while condition in the int specialization.
    ASSERT_EQ(code.find("IsTrue"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_spec_reassign_gcd.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 48, 18);
        ASSERT_EQ(ret, 6);
        Call(s, type, "test", ret, 100, 75);
        ASSERT_EQ(ret, 25);
        Call(s, type, "test", ret, 7, 13);
        ASSERT_EQ(ret, 1);
    });
}

// PowMod-style: base and exp are both reassigned inside the while loop body.
// The compiler must still specialize them as math params.
TEST(infer, test_spec_reassign_powmod) {
    const auto code = InferGetCCode("./infer/test_spec_reassign_powmod.lua");
    // With three math params the all-int suffix is _0_0_0.
    ASSERT_NE(code.find("test_0_0_0(int64_t base, int64_t exp, int64_t mod)"), std::string::npos);
    // Entry dispatcher must exist with CVar params.
    ASSERT_NE(code.find("CVar test(CVar base, CVar exp, CVar mod)"), std::string::npos);
    // In the int specialization, loop body arithmetic must use native int64_t.
    ASSERT_NE(code.find("FlModInt("), std::string::npos);
    ASSERT_NE(code.find("FlFloorDivInt("), std::string::npos);
    // The while condition in the int specialization must be a direct C comparison.
    ASSERT_NE(code.find("(exp) > (0)"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_spec_reassign_powmod.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 2, 10, 1000);
        ASSERT_EQ(ret, 24); // 2^10 mod 1000 = 1024 mod 1000 = 24
        Call(s, type, "test", ret, 3, 5, 100);
        ASSERT_EQ(ret, 43); // 3^5 mod 100 = 243 mod 100 = 43
        Call(s, type, "test", ret, 7, 1, 1000000007);
        ASSERT_EQ(ret, 7);
    });
}

// ────────────────────────────────────────────────────────────────────────────
// Optimization: native C comparisons in if/while/repeat/elseif conditions
// ────────────────────────────────────────────────────────────────────────────

// if condition with T_INT operands: TryCompileNativeBoolExpr must emit
// if ((x) > (5)) directly, without IsTrue or a bool temp variable.
TEST(infer, test_native_bool_if) {
    const auto code = InferGetCCode("./infer/test_native_bool_if.lua");
    // int specialization: condition must be a direct C comparison.
    ASSERT_NE(code.find("((x) > (5))"), std::string::npos);
    // No dynamic IsTrue call for the if condition.
    ASSERT_EQ(code.find("IsTrue"), std::string::npos);
    // No flua_ibt_ temp bool variable.
    ASSERT_EQ(code.find("flua_ibt_"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_native_bool_if.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 10);
        ASSERT_EQ(ret, 1);
        Call(s, type, "test", ret, 3);
        ASSERT_EQ(ret, 0);
    });
}

// while condition with T_INT operands: TryCompileNativeBoolExpr must emit
// while ((x) > (0)) directly, without the fallback while(1)+IsTrue pattern.
TEST(infer, test_native_bool_while) {
    const auto code = InferGetCCode("./infer/test_native_bool_while.lua");
    // int specialization: direct C comparison in while.
    ASSERT_NE(code.find("while ((x) > (0))"), std::string::npos);
    // No IsTrue or flua_wbt_ temp bool variable for the while condition.
    ASSERT_EQ(code.find("flua_wbt_"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_native_bool_while.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 5);
        ASSERT_EQ(ret, 15); // 5+4+3+2+1
        Call(s, type, "test", ret, 0);
        ASSERT_EQ(ret, 0);
        Call(s, type, "test", ret, 3);
        ASSERT_EQ(ret, 6); // 3+2+1
    });
}

// repeat..until condition with T_INT operands: TryCompileNativeBoolExpr must emit
// if ((i) > (...)) break; directly, without IsTrue or a bool temp variable.
TEST(infer, test_native_bool_repeat) {
    const auto code = InferGetCCode("./infer/test_native_bool_repeat.lua");
    // int specialization: direct C comparison as the repeat..until exit check.
    ASSERT_NE(code.find("(i) > ("), std::string::npos);
    // No IsTrue or flua_rbt_ temp bool variable for the repeat condition.
    ASSERT_EQ(code.find("flua_rbt_"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_native_bool_repeat.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 5);
        ASSERT_EQ(ret, 15); // 1+2+3+4+5
        Call(s, type, "test", ret, 1);
        ASSERT_EQ(ret, 1);
        Call(s, type, "test", ret, 10);
        ASSERT_EQ(ret, 55);
    });
}

// elseif condition with T_INT operands: both the if and elseif conditions must
// use direct C comparisons (no IsTrue, no flua_ibt_ temp bools).
TEST(infer, test_native_bool_elseif) {
    const auto code = InferGetCCode("./infer/test_native_bool_elseif.lua");
    // Both the if and the elseif must produce native comparisons.
    ASSERT_NE(code.find("((x) < (0))"), std::string::npos);
    ASSERT_NE(code.find("((x) == (0))"), std::string::npos);
    // No IsTrue or temp bool variables for the conditions.
    ASSERT_EQ(code.find("IsTrue"), std::string::npos);
    ASSERT_EQ(code.find("flua_ibt_"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_native_bool_elseif.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, -3);
        ASSERT_EQ(ret, -1);
        Call(s, type, "test", ret, 0);
        ASSERT_EQ(ret, 0);
        Call(s, type, "test", ret, 7);
        ASSERT_EQ(ret, 1);
    });
}

// Float comparison in if: x is T_FLOAT (after n + 0.0) and the comparison
// x >= 0.0 must also produce a direct C comparison (not IsTrue).
TEST(infer, test_native_bool_float) {
    const auto code = InferGetCCode("./infer/test_native_bool_float.lua");
    // float specialization: direct C comparison with >= and float literal.
    ASSERT_NE(code.find("((x) >= (0)"), std::string::npos);
    // No IsTrue or flua_ibt_ for the if condition.
    ASSERT_EQ(code.find("flua_ibt_"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_native_bool_float.lua", {.debug_mode = debug_mode});
        int ret = 0;
        double x = 1.5;
        Call(s, type, "test", ret, x);
        ASSERT_EQ(ret, 1);
        x = -0.5;
        Call(s, type, "test", ret, x);
        ASSERT_EQ(ret, 0);
    });
}

// AND of two native comparisons in if: (x == 1) and (y > 0) must produce
// a direct C &&-expression without IsTrue or temp bool variables.
TEST(infer, test_native_bool_and) {
    const auto code = InferGetCCode("./infer/test_native_bool_and.lua");
    // Both sub-comparisons and the && combiner must appear in the generated code.
    ASSERT_NE(code.find("((x) == (1))"), std::string::npos);
    ASSERT_NE(code.find("((y) > (0))"), std::string::npos);
    ASSERT_NE(code.find("&&"), std::string::npos);
    // No IsTrue or temp bool variables.
    ASSERT_EQ(code.find("IsTrue"), std::string::npos);
    ASSERT_EQ(code.find("flua_ibt_"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_native_bool_and.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 1, 5);
        ASSERT_EQ(ret, 1);
        Call(s, type, "test", ret, 1, 0);
        ASSERT_EQ(ret, 0);
        Call(s, type, "test", ret, 2, 5);
        ASSERT_EQ(ret, 0);
    });
}

// OR of two native comparisons in if: (x < 0) or (x > 10) must produce
// a direct C ||-expression without IsTrue or temp bool variables.
TEST(infer, test_native_bool_or) {
    const auto code = InferGetCCode("./infer/test_native_bool_or.lua");
    // Both sub-comparisons and the || combiner must appear in the generated code.
    ASSERT_NE(code.find("((x) < (0))"), std::string::npos);
    ASSERT_NE(code.find("((x) > (10))"), std::string::npos);
    ASSERT_NE(code.find("||"), std::string::npos);
    // No IsTrue or temp bool variables.
    ASSERT_EQ(code.find("IsTrue"), std::string::npos);
    ASSERT_EQ(code.find("flua_ibt_"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_native_bool_or.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, -1);
        ASSERT_EQ(ret, 1);
        Call(s, type, "test", ret, 11);
        ASSERT_EQ(ret, 1);
        Call(s, type, "test", ret, 5);
        ASSERT_EQ(ret, 0);
    });
}

// NOT of a native comparison in if: not (x > 5) must produce !(...)
// without IsTrue or temp bool variables.
TEST(infer, test_native_bool_not) {
    const auto code = InferGetCCode("./infer/test_native_bool_not.lua");
    // The negated comparison must appear in the generated code.
    ASSERT_NE(code.find("!(("), std::string::npos);
    ASSERT_NE(code.find("((x) > (5))"), std::string::npos);
    // No IsTrue or temp bool variables.
    ASSERT_EQ(code.find("IsTrue"), std::string::npos);
    ASSERT_EQ(code.find("flua_ibt_"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_native_bool_not.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 3);
        ASSERT_EQ(ret, 1);
        Call(s, type, "test", ret, 10);
        ASSERT_EQ(ret, 0);
    });
}

// Deeply nested AND chain: (x > 0) and (y > 0) and (z > 0) must produce
// a direct C &&-chain without IsTrue or temp bool variables.
TEST(infer, test_native_bool_nested) {
    const auto code = InferGetCCode("./infer/test_native_bool_nested.lua");
    // All three sub-comparisons and the && combiners must appear.
    ASSERT_NE(code.find("((x) > (0))"), std::string::npos);
    ASSERT_NE(code.find("((y) > (0))"), std::string::npos);
    ASSERT_NE(code.find("((z) > (0))"), std::string::npos);
    ASSERT_NE(code.find("&&"), std::string::npos);
    // No IsTrue or temp bool variables.
    ASSERT_EQ(code.find("IsTrue"), std::string::npos);
    ASSERT_EQ(code.find("flua_ibt_"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_native_bool_nested.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 1, 2, 3);
        ASSERT_EQ(ret, 1);
        Call(s, type, "test", ret, 1, 0, 3);
        ASSERT_EQ(ret, 0);
        Call(s, type, "test", ret, 0, 2, 3);
        ASSERT_EQ(ret, 0);
        Call(s, type, "test", ret, 1, 2, 0);
        ASSERT_EQ(ret, 0);
    });
}

// ────────────────────────────────────────────────────────────────────────────
// Missing grammar cases: LEFT_SHIFT/RIGHT_SHIFT and NUMBER_SIGN (#) in
// math param specialization context
// ────────────────────────────────────────────────────────────────────────────

// Math param n: n << 2 should use the native arithmetic fast path via
// kNativeArithOps + FlLShiftInt.  InferArgTypeForSpec already returns T_INT
// for LEFT_SHIFT; kNativeArithOps now includes LEFT_SHIFT.
// For n=5: 5 << 2 = 20.
TEST(infer, test_spec_leftshift_param) {
    const auto code = InferGetCCode("./infer/test_spec_leftshift_param.lua");
    // Both int and float specializations must be declared.
    ASSERT_NE(code.find("test_0(int64_t n)"), std::string::npos);
    ASSERT_NE(code.find("test_1(double n)"), std::string::npos);
    // The int specialization must use FlLShiftInt (not OpLeftShift).
    ASSERT_NE(code.find("FlLShiftInt("), std::string::npos);
    ASSERT_EQ(code.find("OpLeftShift("), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_spec_leftshift_param.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 5);
        ASSERT_EQ(ret, 20); // 5 << 2 = 20
    });
}

// Math param n: n >> 1 should use the native arithmetic fast path via
// kNativeArithOps + FlRShiftInt.  InferArgTypeForSpec already returns T_INT
// for RIGHT_SHIFT; kNativeArithOps now includes RIGHT_SHIFT.
// For n=16: 16 >> 1 = 8.
TEST(infer, test_spec_rightshift_param) {
    const auto code = InferGetCCode("./infer/test_spec_rightshift_param.lua");
    // Both int and float specializations must be declared.
    ASSERT_NE(code.find("test_0(int64_t n)"), std::string::npos);
    ASSERT_NE(code.find("test_1(double n)"), std::string::npos);
    // The int specialization must use FlRShiftInt (not OpRightShift).
    ASSERT_NE(code.find("FlRShiftInt("), std::string::npos);
    ASSERT_EQ(code.find("OpRightShift("), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_spec_rightshift_param.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 16);
        ASSERT_EQ(ret, 8); // 16 >> 1 = 8
    });
}

// Math param n: n + #s should use the native arithmetic fast path because
// InferArgTypeForSpec now returns T_INT for unop NUMBER_SIGN (# always
// yields an integer), enabling CompileNumericExp to emit FlLenInt.
// For n=10 and s="hello" (length 5): test(10) == 15.
TEST(infer, test_spec_len_param) {
    const auto code = InferGetCCode("./infer/test_spec_len_param.lua");
    // Both int and float specializations must be declared.
    ASSERT_NE(code.find("test_0(int64_t n)"), std::string::npos);
    ASSERT_NE(code.find("test_1(double n)"), std::string::npos);
    // The int specialization must call FlLenInt to get the string length.
    ASSERT_NE(code.find("FlLenInt("), std::string::npos);
    // No dynamic OpLen call in the generated code.
    ASSERT_EQ(code.find("OpLen("), std::string::npos);
    // Fix: NUMBER_SIGN in EvalReturnExpType now returns T_INT, so n+#s yields
    // a T_INT return and the int specialization returns int64_t natively.
    ASSERT_NE(code.find("int64_t test_0(int64_t n)"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_spec_len_param.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 10);
        ASSERT_EQ(ret, 15); // n=10, #"hello"=5, 10+5=15
    });
}

// ────────────────────────────────────────────────────────────────────────────
// Missing grammar cases: for-loop with math param as bound, and comparison
// with a specialized function call result.
// ────────────────────────────────────────────────────────────────────────────

// Math param n used as the upper bound of a numeric for-loop.
// TypeInferencer identifies n as a math param via the inner sum + i arithmetic
// (n=T_INT → bound T_INT → loop var i T_INT → T_INT + T_INT = T_INT; vs
// n=T_DYNAMIC → end T_DYNAMIC → i T_DYNAMIC → T_DYNAMIC).
// In the int specialization, CompileStmtForLoop must detect all bounds as T_INT
// (from the snapshot) and emit int64_t control variables.
// test(10) == 55, test(5) == 15.
TEST(infer, test_spec_for_bound_param) {
    const auto code = InferGetCCode("./infer/test_spec_for_bound_param.lua");
    // Both int and float specializations must be declared.
    ASSERT_NE(code.find("test_0(int64_t n)"), std::string::npos);
    ASSERT_NE(code.find("test_1(double n)"), std::string::npos);
    // Entry dispatcher must exist.
    ASSERT_NE(code.find("CVar test(CVar n)"), std::string::npos);
    // In the int specialization: native int64_t for-loop control variables.
    ASSERT_NE(code.find("int64_t flua_for_ctrl_"), std::string::npos);
    ASSERT_NE(code.find("int64_t i = flua_for_ctrl_"), std::string::npos);
    // The upper bound end variable is used (typed int path).
    ASSERT_NE(code.find("int64_t flua_for_end_"), std::string::npos);
    // No dynamic CVar control variables in the int specialization.
    ASSERT_EQ(code.find("CVar flua_for_ctrl_"), std::string::npos);
    ASSERT_EQ(code.find("CVar i"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_spec_for_bound_param.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 10);
        ASSERT_EQ(ret, 55); // 1+2+...+10
        Call(s, type, "test", ret, 5);
        ASSERT_EQ(ret, 15); // 1+2+3+4+5
        Call(s, type, "test", ret, 0);
        ASSERT_EQ(ret, 0); // empty range
    });
}

// Comparison where the left operand is the result of a call to a specialized
// function f.  Without the TryCompileNativeBoolExpr fix, EvalType() would
// return T_DYNAMIC for f(n) (function calls are always T_DYNAMIC) and the
// condition would fall through to the IsTrue dynamic path.  With the fix,
// InferArgTypeForSpec(f(n)) returns T_INT in the int specialization, so the
// condition is emitted as a native C comparison.
// For n > 0: f(n) = 2n > n is true, return x (= n); for n = 0: return 0.
TEST(infer, test_spec_compare_func_result) {
    const auto code = InferGetCCode("./infer/test_spec_compare_func_result.lua");
    // Both f and test must be specialized and now return native int64_t directly.
    ASSERT_NE(code.find("int64_t f_0(int64_t n)"), std::string::npos);
    ASSERT_NE(code.find("int64_t test_0(int64_t n)"), std::string::npos);
    // The if condition must use a direct C comparison: f_0(n) is called natively.
    ASSERT_NE(code.find("f_0(n)"), std::string::npos);
    // With native returns, there is no .data_.i extraction for the spec call result.
    ASSERT_EQ(code.find("f_0(n).data_.i"), std::string::npos);
    // No dynamic IsTrue or flua_ibt_ temp bool variables.
    ASSERT_EQ(code.find("IsTrue"), std::string::npos);
    ASSERT_EQ(code.find("flua_ibt_"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_spec_compare_func_result.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 5);
        ASSERT_EQ(ret, 5); // f(5)=10 > 5 → true → return x=5
        Call(s, type, "test", ret, 0);
        ASSERT_EQ(ret, 0); // f(0)=0 > 0 → false → return 0
        Call(s, type, "test", ret, 3);
        ASSERT_EQ(ret, 3); // f(3)=6 > 3 → true → return x=3
    });
}

// Equality comparison (==) cannot drive math-param specialization because == works
// on any Lua type (strings, booleans, nil, tables). Specialising based on == alone
// would force callers to pass numeric values, breaking e.g. string equality calls.
// Here n is still specialised via arithmetic (n+1, n-1); m remains a CVar in
// specialised bodies, and the == condition uses OpEq + IsTrue.
TEST(infer, test_spec_compare_equal) {
    const auto code = InferGetCCode("./infer/test_spec_compare_equal.lua");
    ASSERT_NE(code.find("int64_t test_0(int64_t n, CVar m)"), std::string::npos);
    ASSERT_NE(code.find("double test_1(double n, CVar m)"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_spec_compare_equal.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 5, 5);
        ASSERT_EQ(ret, 6);
        Call(s, type, "test", ret, 5, 3);
        ASSERT_EQ(ret, 4);
        double dret = 0.0;
        Call(s, type, "test", dret, 2.5, 2.5);
        ASSERT_DOUBLE_EQ(dret, 3.5);
        Call(s, type, "test", dret, 2.5, 1.5);
        ASSERT_DOUBLE_EQ(dret, 1.5);
    });
}

// Pure equality comparison with no arithmetic: n and m are only compared via ==
// and returned as-is.  Because neither parameter participates in any arithmetic
// expression, HasArithmeticImprovement returns false and no numeric
// specialisation should be generated for test.  Only the generic CVar entry
// point must exist.  The function must also work correctly at runtime for
// integers, floats, and strings.
TEST(infer, test_spec_no_arith_compare_only) {
    const auto code = InferGetCCode("./infer/test_spec_no_arith_compare_only.lua");
    // No specialised variants must be emitted.
    ASSERT_EQ(code.find("test_0"), std::string::npos);
    ASSERT_EQ(code.find("test_1"), std::string::npos);
    // The generic CVar entry point must exist.
    ASSERT_NE(code.find("CVar test(CVar n, CVar m)"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_spec_no_arith_compare_only.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 5, 5);
        ASSERT_EQ(ret, 5); // n == m → return n
        Call(s, type, "test", ret, 5, 3);
        ASSERT_EQ(ret, 3); // n != m → return m
        double dret = 0.0;
        Call(s, type, "test", dret, 2.5, 2.5);
        ASSERT_DOUBLE_EQ(dret, 2.5); // n == m → return n
        Call(s, type, "test", dret, 2.5, 1.5);
        ASSERT_DOUBLE_EQ(dret, 1.5); // n != m → return m
        std::string sret;
        Call(s, type, "test", sret, std::string("hello"), std::string("hello"));
        ASSERT_EQ(sret, "hello"); // string equality: return n
        Call(s, type, "test", sret, std::string("hello"), std::string("world"));
        ASSERT_EQ(sret, "world"); // string inequality: return m
    });
}

// ────────────────────────────────────────────────────────────────────────────
// Missing grammar cases: pure unary ops as math params, and do...end blocks.
// ────────────────────────────────────────────────────────────────────────────

// Pure unary negation: function f(n) return -n end.
// Previously IsArithmeticBinop would skip this because there is no binary
// operator; n was not detected as a math param.
// With IsArithmeticExpr now handling unop MINUS, both specialisations are
// generated and the int spec returns int64_t natively.
// test(5) == -5, test(-3) == 3.
TEST(infer, test_spec_unary_minus_param) {
    const auto code = InferGetCCode("./infer/test_spec_unary_minus_param.lua");
    // Both int and float specializations must be declared.
    ASSERT_NE(code.find("test_0(int64_t n)"), std::string::npos);
    ASSERT_NE(code.find("test_1(double n)"), std::string::npos);
    // Specializations must use native negation (no OpUnaryMinus macro).
    ASSERT_NE(code.find("(-(n))"), std::string::npos);
    ASSERT_EQ(code.find("OpUnaryMinus("), std::string::npos);
    // Both specializations must return natively (int64_t / double).
    ASSERT_NE(code.find("int64_t test_0(int64_t n)"), std::string::npos);
    ASSERT_NE(code.find("double test_1(double n)"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_spec_unary_minus_param.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 5);
        ASSERT_EQ(ret, -5);
        Call(s, type, "test", ret, -3);
        ASSERT_EQ(ret, 3);
        double dret = 0.0;
        Call(s, type, "test", dret, 2.5);
        ASSERT_DOUBLE_EQ(dret, -2.5);
    });
}

// Pure bitwise NOT: function f(n) return ~n end.
// Like unary minus, previously no binary operator means n was not detected as
// a math param.  With the IsArithmeticExpr fix, both specialisations are
// generated.  Int spec uses native ~((int64_t)(n)) and returns int64_t.
// Float spec: ~T_FLOAT = T_DYNAMIC, so float spec returns CVar.
// test(5) == -6 (~5 = -6 in two's complement).
TEST(infer, test_spec_bitnot_standalone_param) {
    const auto code = InferGetCCode("./infer/test_spec_bitnot_standalone_param.lua");
    // Both int and float specializations must be declared.
    ASSERT_NE(code.find("test_0(int64_t n)"), std::string::npos);
    ASSERT_NE(code.find("test_1(double n)"), std::string::npos);
    // Int specialization: native BITNOT (~((int64_t)(n))).
    ASSERT_NE(code.find("(~((int64_t)(n)))"), std::string::npos);
    // Int specialization must return int64_t natively.
    ASSERT_NE(code.find("int64_t test_0(int64_t n)"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_spec_bitnot_standalone_param.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 5);
        ASSERT_EQ(ret, -6); // ~5 = -6
        Call(s, type, "test", ret, 0);
        ASSERT_EQ(ret, -1); // ~0 = -1
    });
}

// Return inside do...end block.
// Previously AllPathsReturn and CollectReturnExps did not recurse into
// SyntaxTreeType::Block (do...end) statements, so the return type was inferred
// as T_DYNAMIC (CVar) even when n+1 is clearly T_INT.
// With the fix, the int specialization returns int64_t natively.
// test(10) == 11.
TEST(infer, test_spec_do_return) {
    const auto code = InferGetCCode("./infer/test_spec_do_return.lua");
    // Both int and float specializations must be declared.
    ASSERT_NE(code.find("test_0(int64_t n)"), std::string::npos);
    ASSERT_NE(code.find("test_1(double n)"), std::string::npos);
    // Fix: return inside do...end is now detected, so int spec returns int64_t.
    ASSERT_NE(code.find("int64_t test_0(int64_t n)"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_spec_do_return.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 10);
        ASSERT_EQ(ret, 11);
        Call(s, type, "test", ret, 0);
        ASSERT_EQ(ret, 1);
        double dret = 0.0;
        Call(s, type, "test", dret, 2.5);
        ASSERT_DOUBLE_EQ(dret, 3.5);
    });
}
// f uses n arithmetically (n+1) so it has a math param and gets specialised.
// But f always returns the string "hello", not a numeric result.
// InferArgTypeForSpec for f(n) must return T_DYNAMIC so the caller treats the
// CVar as opaque (no .data_.i access), and the returned string is preserved.
// This exercises the fixed-point return-type inference that was missing before.
TEST(infer, test_spec_non_numeric_return) {
    const auto code = InferGetCCode("./infer/test_spec_non_numeric_return.lua");
    // f must still be specialised (n is a math param due to n+1 arithmetic).
    ASSERT_NE(code.find("f_0(int64_t n)"), std::string::npos);
    // But since f returns a string, f_0 result must NOT be accessed via .data_.i
    // in test's body.  The call must go through the normal CVar path.
    ASSERT_EQ(code.find("f_0(n).data_.i"), std::string::npos);
    ASSERT_EQ(code.find("f_0(n).data_.f"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_spec_non_numeric_return.lua", {.debug_mode = debug_mode});
        std::string ret;
        Call(s, type, "test", ret, 5);
        ASSERT_EQ(ret, "hello");
        Call(s, type, "test", ret, 0);
        ASSERT_EQ(ret, "hello");
    });
}

// Three-level nested call chain: func2 has direct arithmetic (n+1), func1
// only calls func2(n), and test only calls func1(n).  After the nested-call
// fix, HasMathCallImprovement propagates the math-param recognition upward:
// func1 and test are both specialised even though they contain no arithmetic
// operators themselves.  All three specialisations must return native int64_t
// so the whole call chain is free of CVar boxing/unboxing.
// test(10) == 11, test(2.5) == 3.5.
TEST(infer, test_spec_nested_call) {
    const auto code = InferGetCCode("./infer/test_spec_nested_call.lua");
    // All three functions must have an int64_t specialisation.
    ASSERT_NE(code.find("func2_0(int64_t n)"), std::string::npos);
    ASSERT_NE(code.find("func1_0(int64_t n)"), std::string::npos);
    ASSERT_NE(code.find("test_0(int64_t n)"), std::string::npos);
    // All three specialisations must return native int64_t (not CVar).
    ASSERT_NE(code.find("int64_t func2_0(int64_t n)"), std::string::npos);
    ASSERT_NE(code.find("int64_t func1_0(int64_t n)"), std::string::npos);
    ASSERT_NE(code.find("int64_t test_0(int64_t n)"), std::string::npos);
    // func1_0 must call func2_0 directly (native spec call, no boxing).
    ASSERT_NE(code.find("func2_0(n)"), std::string::npos);
    // test_0 must call func1_0 directly.
    ASSERT_NE(code.find("func1_0(n)"), std::string::npos);
    // CVar entry dispatchers must still exist for runtime polymorphism.
    ASSERT_NE(code.find("CVar func2(CVar n)"), std::string::npos);
    ASSERT_NE(code.find("CVar func1(CVar n)"), std::string::npos);
    ASSERT_NE(code.find("CVar test(CVar n)"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_spec_nested_call.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 10);
        ASSERT_EQ(ret, 11);
        Call(s, type, "test", ret, 0);
        ASSERT_EQ(ret, 1);
        double dret = 0.0;
        Call(s, type, "test", dret, 2.5);
        ASSERT_DOUBLE_EQ(dret, 3.5);
    });
}

// args ::= (explist) | tableconstructor | LiteralString
// Ensure math-param specialisation analysis walks all function-call args forms
// without missing the normal explist-based specialisation path.
TEST(infer, test_spec_args_syntax_mix) {
    const auto code = InferGetCCode("./infer/test_spec_args_syntax_mix.lua");
    ASSERT_NE(code.find("int64_t callee_0(int64_t n)"), std::string::npos);
    ASSERT_NE(code.find("double callee_1(double n)"), std::string::npos);
    ASSERT_NE(code.find("int64_t test_0(int64_t n)"), std::string::npos);
    ASSERT_NE(code.find("double test_1(double n)"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_spec_args_syntax_mix.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 10);
        ASSERT_EQ(ret, 11);
        double dret = 0.0;
        Call(s, type, "test", dret, 2.5);
        ASSERT_DOUBLE_EQ(dret, 3.5);
    });
}

TEST(infer, test_spec_local_from_func_call) {
    const auto code = InferGetCCode("./infer/test_spec_local_from_func_call.lua");
    // func must be specialised (has direct arithmetic n+1).
    ASSERT_NE(code.find("int64_t func_0(int64_t n)"), std::string::npos);
    // wrapper must also be specialised because it passes n to func's math param.
    ASSERT_NE(code.find("int64_t wrapper_0(int64_t n)"), std::string::npos);
    // local x should be declared as int64_t (not CVar) inside wrapper_0.
    ASSERT_NE(code.find("int64_t x ="), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_spec_local_from_func_call.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "wrapper", ret, 10);
        ASSERT_EQ(ret, 11);
        double dret = 0.0;
        Call(s, type, "wrapper", dret, 2.5);
        ASSERT_DOUBLE_EQ(dret, 3.5);
    });
}

TEST(infer, test_spec_local_chain_from_func_call) {
    const auto code = InferGetCCode("./infer/test_spec_local_chain_from_func_call.lua");
    // func must be specialised (direct arithmetic n*2).
    ASSERT_NE(code.find("int64_t func_0(int64_t n)"), std::string::npos);
    // chain must also be specialised.
    ASSERT_NE(code.find("int64_t chain_0(int64_t n)"), std::string::npos);
    // Both x and y should be int64_t inside chain_0.
    ASSERT_NE(code.find("int64_t x ="), std::string::npos);
    ASSERT_NE(code.find("int64_t y ="), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_spec_local_chain_from_func_call.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "chain", ret, 5);
        ASSERT_EQ(ret, 11);
        Call(s, type, "chain", ret, 3);
        ASSERT_EQ(ret, 7);
        double dret = 0.0;
        Call(s, type, "chain", dret, 2.5);
        ASSERT_DOUBLE_EQ(dret, 6.0);
    });
}

// ────────────────────────────────────────────────────────────────────────────
// Missing grammar cases: local function, for-loop with arithmetic bounds/step,
// break inside specialized loop, arithmetic local var chain, repeat...until.
// ────────────────────────────────────────────────────────────────────────────

// local function 形式的数学参数特化。
// square 通过 local function 定义，n 是数学参数（n*n 有算术改善）。
// DiscoverMathParams 对 LocalFunction 和 Function 均适用。
// test 通过嵌套调用推断也被特化（square(n)+1 触发改善）。
TEST(infer, test_spec_local_func) {
    const auto code = InferGetCCode("./infer/test_spec_local_func.lua");
    // local function square must be specialised.
    ASSERT_NE(code.find("int64_t square_0(int64_t n)"), std::string::npos);
    ASSERT_NE(code.find("double square_1(double n)"), std::string::npos);
    // Entry dispatcher must exist for square.
    ASSERT_NE(code.find("CVar square(CVar n)"), std::string::npos);
    // test must also be specialised via the nested-call improvement.
    ASSERT_NE(code.find("int64_t test_0(int64_t n)"), std::string::npos);
    ASSERT_NE(code.find("CVar test(CVar n)"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_spec_local_func.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 5);
        ASSERT_EQ(ret, 26); // 5*5+1
        Call(s, type, "test", ret, 3);
        ASSERT_EQ(ret, 10); // 3*3+1
        double dret = 0.0;
        Call(s, type, "test", dret, 2.0);
        ASSERT_DOUBLE_EQ(dret, 5.0); // 2.0*2.0+1
    });
}

// 数学参数 n 作为 for 循环上界的算术表达式（n * 2）。
// 整数特化中，ExpEnd = n*2 被快照标注为 T_INT，
// 使 CompileStmtForLoop 走 typed_int_for 路径，上界使用原生 int64_t 表达式。
TEST(infer, test_spec_for_arith_end) {
    const auto code = InferGetCCode("./infer/test_spec_for_arith_end.lua");
    // Both int and float specializations must be declared.
    ASSERT_NE(code.find("test_0(int64_t n)"), std::string::npos);
    ASSERT_NE(code.find("test_1(double n)"), std::string::npos);
    // In the int specialization: native int64_t for-loop control variables.
    ASSERT_NE(code.find("int64_t flua_for_ctrl_"), std::string::npos);
    ASSERT_NE(code.find("int64_t flua_for_end_"), std::string::npos);
    // No dynamic CVar control variables in the int specialization.
    ASSERT_EQ(code.find("CVar flua_for_ctrl_"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_spec_for_arith_end.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 5);
        ASSERT_EQ(ret, 55); // 1+2+...+10
        Call(s, type, "test", ret, 3);
        ASSERT_EQ(ret, 21); // 1+2+...+6
        Call(s, type, "test", ret, 0);
        ASSERT_EQ(ret, 0); // empty range
    });
}

// 数学参数 n 作为 for 循环步长的算术表达式（n + 1）。
// 整数特化中，ExpStep = n+1 被快照标注为 T_INT，
// 使 CompileStmtForLoop 走 typed_int_for 路径（begin/end/step 全为 T_INT）。
TEST(infer, test_spec_for_arith_step) {
    const auto code = InferGetCCode("./infer/test_spec_for_arith_step.lua");
    // Both int and float specializations must be declared.
    ASSERT_NE(code.find("test_0(int64_t n)"), std::string::npos);
    ASSERT_NE(code.find("test_1(double n)"), std::string::npos);
    // In the int specialization: native int64_t for-loop control variables.
    ASSERT_NE(code.find("int64_t flua_for_ctrl_"), std::string::npos);
    ASSERT_NE(code.find("int64_t flua_for_step_"), std::string::npos);
    ASSERT_NE(code.find("int64_t flua_for_end_"), std::string::npos);
    // No dynamic CVar control variables in the int specialization.
    ASSERT_EQ(code.find("CVar flua_for_ctrl_"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_spec_for_arith_step.lua", {.debug_mode = debug_mode});
        int ret = 0;
        // step = n+1 = 3: i = 1, 4, 7, 10  -> sum = 22
        Call(s, type, "test", ret, 2);
        ASSERT_EQ(ret, 22);
        // step = n+1 = 4: i = 1, 5, 9  -> sum = 15
        Call(s, type, "test", ret, 3);
        ASSERT_EQ(ret, 15);
    });
}

// for 循环中 break 在数学参数特化函数里的正确性。
// n 为数学参数（sum + i 是整数算术），循环在 i > 3 时提前退出。
TEST(infer, test_spec_for_break) {
    const auto code = InferGetCCode("./infer/test_spec_for_break.lua");
    // Both int and float specializations must be declared.
    ASSERT_NE(code.find("test_0(int64_t n)"), std::string::npos);
    ASSERT_NE(code.find("test_1(double n)"), std::string::npos);
    // In the int specialization: native int64_t for-loop control variables.
    ASSERT_NE(code.find("int64_t flua_for_ctrl_"), std::string::npos);
    // break must be emitted.
    ASSERT_NE(code.find("break;"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_spec_for_break.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 10);
        ASSERT_EQ(ret, 6); // 1+2+3 (stops at i=4)
        Call(s, type, "test", ret, 2);
        ASSERT_EQ(ret, 3); // 1+2 (n < 4, no break needed)
        Call(s, type, "test", ret, 0);
        ASSERT_EQ(ret, 0); // empty range
    });
}

// 纯算术局部变量链：local x = n + 1; local y = x * 2; return y。
// 整数特化中，快照将 n+1 和 x*2 均标注为 T_INT，
// x 和 y 均声明为 int64_t，return 直接返回 int64_t。
TEST(infer, test_spec_arith_chain) {
    const auto code = InferGetCCode("./infer/test_spec_arith_chain.lua");
    // Both int and float specializations must be declared.
    ASSERT_NE(code.find("int64_t test_0(int64_t n)"), std::string::npos);
    ASSERT_NE(code.find("double test_1(double n)"), std::string::npos);
    // x and y must be declared as native types (int64_t) in the int spec.
    ASSERT_NE(code.find("int64_t x ="), std::string::npos);
    ASSERT_NE(code.find("int64_t y ="), std::string::npos);
    // No CVar for x or y in the int specialization.
    ASSERT_EQ(code.find("CVar x"), std::string::npos);
    ASSERT_EQ(code.find("CVar y"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_spec_arith_chain.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 5);
        ASSERT_EQ(ret, 12); // (5+1)*2
        Call(s, type, "test", ret, 3);
        ASSERT_EQ(ret, 8); // (3+1)*2
        double dret = 0.0;
        Call(s, type, "test", dret, 1.5);
        ASSERT_DOUBLE_EQ(dret, 5.0); // (1.5+1)*2
    });
}

// repeat...until 循环内数学参数参与循环体算术运算。
// n 被用于 sum = sum + n（整数算术），使 n 成为数学参数。
// until 条件 i > 5 使用 TryCompileNativeBoolExpr 编译为原生布尔表达式。
TEST(infer, test_spec_repeat_arith) {
    const auto code = InferGetCCode("./infer/test_spec_repeat_arith.lua");
    // Both int and float specializations must be declared.
    ASSERT_NE(code.find("test_0(int64_t n)"), std::string::npos);
    ASSERT_NE(code.find("test_1(double n)"), std::string::npos);
    // Entry dispatcher must exist.
    ASSERT_NE(code.find("CVar test(CVar n)"), std::string::npos);
    // The do...while loop must be emitted.
    ASSERT_NE(code.find("do {"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_spec_repeat_arith.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 3);
        ASSERT_EQ(ret, 15); // 3*5
        Call(s, type, "test", ret, 2);
        ASSERT_EQ(ret, 10); // 2*5
        double dret = 0.0;
        Call(s, type, "test", dret, 1.5);
        ASSERT_DOUBLE_EQ(dret, 7.5); // 1.5*5
    });
}

// ────────────────────────────────────────────────────────────────────────────
// Missing grammar cases: comparison-only math params (min/max/clamp patterns).
// Previously, functions that used parameters ONLY in comparison expressions
// (no arithmetic) were not detected as having math params.  After adding
// IsNativeComparisonExpr detection in HasArithmeticImprovement /
// ParamAffectsArithmetic, these functions are specialised and comparisons
// use TryCompileNativeBoolExpr to emit native C comparisons.
// ────────────────────────────────────────────────────────────────────────────

// min(a, b): comparison-only math params.
// Both a and b are detected as math params via the a < b comparison node.
// In the all-int specialization, the if condition must use native C comparison.
// test(3, 5) == 3, test(7, 2) == 2, test(4, 4) == 4.
TEST(infer, test_spec_min_param) {
    const auto code = InferGetCCode("./infer/test_spec_min_param.lua");
    // Both min and test must be specialised (two math params each).
    ASSERT_NE(code.find("min_0_0(int64_t a, int64_t b)"), std::string::npos);
    ASSERT_NE(code.find("test_0_0(int64_t a, int64_t b)"), std::string::npos);
    // Entry dispatchers must exist.
    ASSERT_NE(code.find("CVar min(CVar a, CVar b)"), std::string::npos);
    ASSERT_NE(code.find("CVar test(CVar a, CVar b)"), std::string::npos);
    // The int specialization must use a native C comparison, not IsTrue.
    ASSERT_NE(code.find("(a) < (b)"), std::string::npos);
    ASSERT_EQ(code.find("IsTrue"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_spec_min_param.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 3, 5);
        ASSERT_EQ(ret, 3);
        Call(s, type, "test", ret, 7, 2);
        ASSERT_EQ(ret, 2);
        Call(s, type, "test", ret, 4, 4);
        ASSERT_EQ(ret, 4);
        double dret = 0.0;
        Call(s, type, "test", dret, 1.5, 2.5);
        ASSERT_DOUBLE_EQ(dret, 1.5);
        Call(s, type, "test", dret, 3.7, 1.2);
        ASSERT_DOUBLE_EQ(dret, 1.2);
    });
}

// max(a, b): comparison-only math params.
// Both a and b are detected as math params via the a > b comparison node.
// In the all-int specialization, the if condition must use native C comparison.
// test(3, 5) == 5, test(7, 2) == 7, test(4, 4) == 4.
TEST(infer, test_spec_max_param) {
    const auto code = InferGetCCode("./infer/test_spec_max_param.lua");
    // Both max and test must be specialised (two math params each).
    ASSERT_NE(code.find("max_0_0(int64_t a, int64_t b)"), std::string::npos);
    ASSERT_NE(code.find("test_0_0(int64_t a, int64_t b)"), std::string::npos);
    // Entry dispatchers must exist.
    ASSERT_NE(code.find("CVar max(CVar a, CVar b)"), std::string::npos);
    ASSERT_NE(code.find("CVar test(CVar a, CVar b)"), std::string::npos);
    // The int specialization must use a native C comparison, not IsTrue.
    ASSERT_NE(code.find("(a) > (b)"), std::string::npos);
    ASSERT_EQ(code.find("IsTrue"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_spec_max_param.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 3, 5);
        ASSERT_EQ(ret, 5);
        Call(s, type, "test", ret, 7, 2);
        ASSERT_EQ(ret, 7);
        Call(s, type, "test", ret, 4, 4);
        ASSERT_EQ(ret, 4);
        double dret = 0.0;
        Call(s, type, "test", dret, 1.5, 2.5);
        ASSERT_DOUBLE_EQ(dret, 2.5);
        Call(s, type, "test", dret, 3.7, 1.2);
        ASSERT_DOUBLE_EQ(dret, 3.7);
    });
}

// clamp(x, lo, hi): three comparison-only math params.
// x, lo, hi are all detected as math params via x < lo and x > hi comparisons.
// 3 math params → 8 specializations (2^3).
// In the all-int specialization, both if conditions use native C comparisons.
// test(5,1,10)==5, test(0,1,10)==1, test(15,1,10)==10.
TEST(infer, test_spec_clamp_param) {
    const auto code = InferGetCCode("./infer/test_spec_clamp_param.lua");
    // All-int specialization (bitmask 0_0_0) must exist for clamp and test.
    ASSERT_NE(code.find("clamp_0_0_0(int64_t x, int64_t lo, int64_t hi)"), std::string::npos);
    ASSERT_NE(code.find("test_0_0_0(int64_t x, int64_t lo, int64_t hi)"), std::string::npos);
    // Entry dispatchers must exist.
    ASSERT_NE(code.find("CVar clamp(CVar x, CVar lo, CVar hi)"), std::string::npos);
    ASSERT_NE(code.find("CVar test(CVar x, CVar lo, CVar hi)"), std::string::npos);
    // Both if conditions must be emitted as native C comparisons.
    ASSERT_NE(code.find("(x) < (lo)"), std::string::npos);
    ASSERT_NE(code.find("(x) > (hi)"), std::string::npos);
    ASSERT_EQ(code.find("IsTrue"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_spec_clamp_param.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 5, 1, 10);
        ASSERT_EQ(ret, 5); // in range
        Call(s, type, "test", ret, 0, 1, 10);
        ASSERT_EQ(ret, 1); // below lo
        Call(s, type, "test", ret, 15, 1, 10);
        ASSERT_EQ(ret, 10); // above hi
        double dret = 0.0;
        Call(s, type, "test", dret, 5.5, 1.0, 10.0);
        ASSERT_DOUBLE_EQ(dret, 5.5);
        Call(s, type, "test", dret, 0.5, 1.0, 10.0);
        ASSERT_DOUBLE_EQ(dret, 1.0);
        Call(s, type, "test", dret, 15.5, 1.0, 10.0);
        ASSERT_DOUBLE_EQ(dret, 10.0);
    });
}

// 缺失语法补全：and/or 运算符 + for 循环 begin 表达式
// ────────────────────────────────────────────────────────────────────────────

// n or 2 のような OR 式を含む特化：乗算 * 3 が数学パラメータ検出のトリガー。
// Lua では整数（0 を含む）は常に真値なので n or 2 = n。
// 整数特化：local y = n or 2 は int64_t y = n にコンパイルされる（CVar 装箱なし）。
// 乗算 y * 3 は原生 int64_t 演算になる。
// test(5) == 15, test(0) == 0（0 は Lua 真値）, test(3.5) == 10.5。
TEST(infer, test_spec_or_param) {
    const auto code = InferGetCCode("./infer/test_spec_or_param.lua");
    // n must be specialised as math param (triggered by y * 3).
    ASSERT_NE(code.find("test_0(int64_t n)"), std::string::npos);
    ASSERT_NE(code.find("test_1(double n)"), std::string::npos);
    // In the int spec, y should be declared as int64_t (OR returns left when left is truthy).
    ASSERT_NE(code.find("int64_t y ="), std::string::npos);
    // No CVar y in int specialization.
    ASSERT_EQ(code.find("CVar y"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_spec_or_param.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 5);
        ASSERT_EQ(ret, 15); // 5 or 2 = 5 (5 is truthy), 5 * 3 = 15
        Call(s, type, "test", ret, 0); // 0 is truthy in Lua: 0 or 2 = 0, 0 * 3 = 0
        ASSERT_EQ(ret, 0);
        double dret = 0.0;
        Call(s, type, "test", dret, 3.5);
        ASSERT_DOUBLE_EQ(dret, 10.5); // 3.5 or 2 = 3.5, 3.5 * 3 = 10.5
    });
}

// n and (n+1): Lua では整数は常に真値なので結果は常に (n+1)。
// n+1 という加算が数学パラメータ検出のトリガーとなる。
// 整数特化：AND は n を評価（ダミー）して n+1 の原生値を返す。
// test(5) == 6, test(0) == 1（0 は Lua 真値）, test(3.5) == 4.5。
TEST(infer, test_spec_and_param) {
    const auto code = InferGetCCode("./infer/test_spec_and_param.lua");
    // n must be specialised as math param (triggered by n + 1 inside AND).
    ASSERT_NE(code.find("test_0(int64_t n)"), std::string::npos);
    ASSERT_NE(code.find("test_1(double n)"), std::string::npos);
    // Entry dispatcher must exist.
    ASSERT_NE(code.find("CVar test(CVar n)"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_spec_and_param.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 5);
        ASSERT_EQ(ret, 6); // 5 and 6 = 6
        Call(s, type, "test", ret, 0); // 0 is truthy in Lua: 0 and 1 = 1
        ASSERT_EQ(ret, 1);
        double dret = 0.0;
        Call(s, type, "test", dret, 3.5);
        ASSERT_DOUBLE_EQ(dret, 4.5); // 3.5 and 4.5 = 4.5
    });
}

// 数学参数 n 用作 for 循环的起始值（begin），而非上界。
// 整数特化中，ExpBegin = n 被快照标注为 T_INT，
// 使 CompileStmtForLoop 走 typed_int_for 路径，控制变量使用原生 int64_t。
// test(3) == 3+4+...+10 = 52, test(8) == 8+9+10 = 27, test(11) == 0（空区间）。
TEST(infer, test_spec_for_arith_begin) {
    const auto code = InferGetCCode("./infer/test_spec_for_arith_begin.lua");
    // Both int and float specializations must be declared.
    ASSERT_NE(code.find("test_0(int64_t n)"), std::string::npos);
    ASSERT_NE(code.find("test_1(double n)"), std::string::npos);
    // In the int specialization: native int64_t for-loop control variables.
    ASSERT_NE(code.find("int64_t flua_for_ctrl_"), std::string::npos);
    ASSERT_NE(code.find("int64_t flua_for_end_"), std::string::npos);
    // No dynamic CVar control variables in the int specialization.
    ASSERT_EQ(code.find("CVar flua_for_ctrl_"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_spec_for_arith_begin.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 3);
        ASSERT_EQ(ret, 52); // 3+4+...+10
        Call(s, type, "test", ret, 8);
        ASSERT_EQ(ret, 27); // 8+9+10
        Call(s, type, "test", ret, 11);
        ASSERT_EQ(ret, 0); // empty range
    });
}

// 数学参数 n 作为 for 循环起始算术表达式（n + 1）。
// 整数特化中，ExpBegin = n+1 被快照标注为 T_INT，
// 使 CompileStmtForLoop 走 typed_int_for 路径（begin/end 全为 T_INT）。
// test(10) == 11+12+...+20 = 155, test(0) == 1+2+...+20 = 210, test(19) == 20。
TEST(infer, test_spec_for_arith_begin_expr) {
    const auto code = InferGetCCode("./infer/test_spec_for_arith_begin_expr.lua");
    // Both int and float specializations must be declared.
    ASSERT_NE(code.find("test_0(int64_t n)"), std::string::npos);
    ASSERT_NE(code.find("test_1(double n)"), std::string::npos);
    // In the int specialization: native int64_t for-loop control variables.
    ASSERT_NE(code.find("int64_t flua_for_ctrl_"), std::string::npos);
    ASSERT_NE(code.find("int64_t flua_for_end_"), std::string::npos);
    // No dynamic CVar control variables in the int specialization.
    ASSERT_EQ(code.find("CVar flua_for_ctrl_"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_spec_for_arith_begin_expr.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 10);
        ASSERT_EQ(ret, 155); // 11+12+...+20
        Call(s, type, "test", ret, 0);
        ASSERT_EQ(ret, 210); // 1+2+...+20
        Call(s, type, "test", ret, 19);
        ASSERT_EQ(ret, 20); // just 20
    });
}

// Native BITAND in CompileBinop via return expression: return x & 10 where x is T_INT.
// Exercises the BITAND branch of the native arithmetic fast path (c_gen.cpp line 2396).
// 12 & 10 = 8.
TEST(infer, test_infer_native_binop_bitand) {
    const auto code = InferGetCCode("./infer/test_infer_native_binop_bitand.lua");
    ASSERT_NE(code.find("(int64_t)(x)"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_native_binop_bitand.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 8);
    });
}

// Native BITOR in CompileBinop via return expression: return x | 3 where x is T_INT.
// Exercises the BITOR branch of the native arithmetic fast path (c_gen.cpp line 2398).
// 12 | 3 = 15.
TEST(infer, test_infer_native_binop_bitor) {
    const auto code = InferGetCCode("./infer/test_infer_native_binop_bitor.lua");
    ASSERT_NE(code.find("(int64_t)(x)"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_native_binop_bitor.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 15);
    });
}

// Native XOR in CompileBinop via return expression: return x ~ 3 where x is T_INT.
// Exercises the XOR branch of the native arithmetic fast path (c_gen.cpp line 2400).
// 5 ~ 3 = 6.
TEST(infer, test_infer_native_binop_bitxor) {
    const auto code = InferGetCCode("./infer/test_infer_native_binop_bitxor.lua");
    ASSERT_NE(code.find("(int64_t)(x)"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_native_binop_bitxor.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 6);
    });
}

// Native LEFT_SHIFT in CompileBinop via return expression: return x << 4 where x is T_INT.
// Exercises the LEFT_SHIFT branch of the native arithmetic fast path using FlLShiftInt.
// 1 << 4 = 16.
TEST(infer, test_infer_native_binop_lshift) {
    const auto code = InferGetCCode("./infer/test_infer_native_binop_lshift.lua");
    ASSERT_NE(code.find("FlLShiftInt("), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_native_binop_lshift.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 16);
    });
}

// Native RIGHT_SHIFT in CompileBinop via return expression: return x >> 3 where x is T_INT.
// Exercises the RIGHT_SHIFT branch of the native arithmetic fast path using FlRShiftInt.
// 256 >> 3 = 32.
TEST(infer, test_infer_native_binop_rshift) {
    const auto code = InferGetCCode("./infer/test_infer_native_binop_rshift.lua");
    ASSERT_NE(code.find("FlRShiftInt("), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_native_binop_rshift.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 32);
    });
}

// Unary MINUS in InferArgTypeForSpec: exercises c_gen.cpp line 1392.
// In "-x + y", InferArgTypeForSpec is called on the -x unop operand, which
// recursively calls InferArgTypeForSpec on x (T_INT) -> returns T_INT.
// Both operands T_INT -> native fast path for PLUS. -3 + 7 = 4.
TEST(infer, test_infer_native_unop_minus_in_binop) {
    const auto code = InferGetCCode("./infer/test_infer_native_unop_minus_in_binop.lua");
    ASSERT_NE(code.find("-(x)"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_native_unop_minus_in_binop.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 4);
    });
}

// NUMBER_SIGN unop in InferArgTypeForSpec: exercises c_gen.cpp line 1401.
// In "#s + 0", InferArgTypeForSpec called on #s (NUMBER_SIGN) returns T_INT.
// Both operands T_INT -> native fast path for PLUS. #"hello" + 0 = 5.
TEST(infer, test_infer_native_unop_numsign_in_binop) {
    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_native_unop_numsign_in_binop.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 5);
    });
}

// Typed float native var assigned from a T_DYNAMIC function call: exercises
// c_gen.cpp lines 1735-1741 (CVar extraction path for typed native vars).
// local x = 1.0 -> double; x = helper() -> T_DYNAMIC; TryCompileNativeExpr
// fails, so rhs is compiled as CVar and .data_.f is extracted into x.
TEST(infer, test_infer_typed_float_var_cvar_assign) {
    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_typed_float_var_cvar_assign.lua", {.debug_mode = debug_mode});
        double ret = 0.0;
        Call(s, type, "test", ret);
        ASSERT_NEAR(ret, 3.0, 0.001);
    });
}

// Native STAR in CompileBinop: return expression with both operands typed T_INT.
// local x = 3 (T_INT), 4 = T_INT literal => native (x * 4) via CompileBinop fast path.
TEST(infer, test_infer_native_binop_star) {
    const auto code = InferGetCCode("./infer/test_infer_native_binop_star.lua");
    // Native multiplication must appear in the generated C code.
    ASSERT_NE(code.find("((x) * (4))"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_native_binop_star.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 12);
    });
}

// Native SLASH in CompileBinop: return expression with T_INT operand.
// local x = 3 (T_INT), x / 2 promotes both operands to double.
TEST(infer, test_infer_native_binop_slash) {
    const auto code = InferGetCCode("./infer/test_infer_native_binop_slash.lua");
    // Division must cast operands to double in the generated C code.
    ASSERT_NE(code.find("(double)(x)"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_native_binop_slash.lua", {.debug_mode = debug_mode});
        double ret = 0.0;
        Call(s, type, "test", ret);
        ASSERT_NEAR(ret, 1.5, 0.001);
    });
}

// Native POW in CompileBinop: return expression with T_INT operand.
// local x = 2 (T_INT), x ^ 10 uses pow() with double casts.
TEST(infer, test_infer_native_binop_pow) {
    const auto code = InferGetCCode("./infer/test_infer_native_binop_pow.lua");
    // pow() call must appear in the generated C code.
    ASSERT_NE(code.find("pow("), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_native_binop_pow.lua", {.debug_mode = debug_mode});
        double ret = 0.0;
        Call(s, type, "test", ret);
        ASSERT_NEAR(ret, 1024.0, 0.001);
    });
}

// Native DOUBLE_SLASH int in CompileBinop: return expression with T_INT operands.
// local x = 7 (T_INT), x // 2 uses FlFloorDivInt.
TEST(infer, test_infer_native_binop_floor_div_int) {
    const auto code = InferGetCCode("./infer/test_infer_native_binop_floor_div_int.lua");
    // FlFloorDivInt must be emitted for integer floor division.
    ASSERT_NE(code.find("FlFloorDivInt("), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_native_binop_floor_div_int.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 3);
    });
}

// Native DOUBLE_SLASH float in CompileBinop: return expression with T_FLOAT operand.
// local x = 7.0 (T_FLOAT), x // 2 uses floor(double/double).
TEST(infer, test_infer_native_binop_floor_div_float) {
    const auto code = InferGetCCode("./infer/test_infer_native_binop_floor_div_float.lua");
    // floor() call must appear in the generated C code.
    ASSERT_NE(code.find("floor("), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_native_binop_floor_div_float.lua", {.debug_mode = debug_mode});
        double ret = 0.0;
        Call(s, type, "test", ret);
        ASSERT_NEAR(ret, 3.0, 0.001);
    });
}

// Native MOD int in CompileBinop: return expression with T_INT operands.
// local x = 7 (T_INT), x % 3 uses FlModInt.
TEST(infer, test_infer_native_binop_mod_int) {
    const auto code = InferGetCCode("./infer/test_infer_native_binop_mod_int.lua");
    // FlModInt must be emitted for integer modulo.
    ASSERT_NE(code.find("FlModInt("), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_native_binop_mod_int.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 1);
    });
}

// Native MOD float in CompileBinop: return expression with T_FLOAT operand.
// local x = 7.5 (T_FLOAT), x % 3 uses FlModFloat.
TEST(infer, test_infer_native_binop_mod_float) {
    const auto code = InferGetCCode("./infer/test_infer_native_binop_mod_float.lua");
    // FlModFloat must be emitted for float modulo.
    ASSERT_NE(code.find("FlModFloat("), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_infer_native_binop_mod_float.lua", {.debug_mode = debug_mode});
        double ret = 0.0;
        Call(s, type, "test", ret);
        ASSERT_NEAR(ret, 1.5, 0.001);
    });
}

// Specializable function with for-in loop inside the body.
// Exercises type_inferencer.cpp CollectReturnExps for ForIn (lines 868-869).
// n is a math param (used in sum + n); the for-in traverses {1,2,3}.
// test(10) = 1+2+3 + 10 = 16.
TEST(infer, test_spec_for_in_body) {
    const auto code = InferGetCCode("./infer/test_spec_for_in_body.lua");
    ASSERT_NE(code.find("CVar test(CVar n)"), std::string::npos);
    ASSERT_NE(code.find("CVar sum = "), std::string::npos);
    ASSERT_NE(code.find("OpAdd((sum), (n),"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_spec_for_in_body.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 10);
        ASSERT_EQ(ret, 16);
        Call(s, type, "test", ret, 0);
        ASSERT_EQ(ret, 6);
    });
}

// Specializable function with elseif where not all paths return via AllPathsReturn.
// Exercises type_inferencer.cpp lines 804-805 (elseif AllPathsReturn returns false
// when an elseif block itself does not return, preventing over-eager specialization).
// test(15) = 30, test(8) = 9, test(3) = 3.
TEST(infer, test_spec_elseif_no_all_return) {
    const auto code = InferGetCCode("./infer/test_spec_elseif_no_all_return.lua");
    ASSERT_NE(code.find("test_0(int64_t n)"), std::string::npos);
    ASSERT_NE(code.find("test_1(double n)"), std::string::npos);
    ASSERT_NE(code.find("CVar test(CVar n)"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_spec_elseif_no_all_return.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 15);
        ASSERT_EQ(ret, 30);
        Call(s, type, "test", ret, 8);
        ASSERT_EQ(ret, 9);
        Call(s, type, "test", ret, 3);
        ASSERT_EQ(ret, 3);
    });
}

// Specializable function with a bare 'return' (no expression).
// Exercises type_inferencer.cpp line 847: CollectReturnExps pushes nullptr
// when the return statement has no expression list.
// test(5) = 10, test(-1) = nil (from bare return).
TEST(infer, test_spec_bare_return) {
    const auto code = InferGetCCode("./infer/test_spec_bare_return.lua");
    ASSERT_NE(code.find("test_0(int64_t n)"), std::string::npos);
    ASSERT_NE(code.find("test_1(double n)"), std::string::npos);
    ASSERT_NE(code.find("CVar test(CVar n)"), std::string::npos);
    ASSERT_NE(code.find("((n) * (2))"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_spec_bare_return.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 5);
        ASSERT_EQ(ret, 10);
    });
}

// For-loop with math param used only as loop bound or step; loop body does NOT use
// the loop variable in arithmetic expressions.
TEST(infer, test_count_loop) {
    const auto code = InferGetCCode("./infer/test_count_loop.lua");
    ASSERT_NE(code.find("count_to_0(int64_t n)"), std::string::npos);
    ASSERT_NE(code.find("count_to_1(double n)"), std::string::npos);
    ASSERT_NE(code.find("count_from_0(int64_t n)"), std::string::npos);
    ASSERT_NE(code.find("count_from_1(double n)"), std::string::npos);
    ASSERT_NE(code.find("count_step_0(int64_t n)"), std::string::npos);
    ASSERT_NE(code.find("count_step_1(double n)"), std::string::npos);
    ASSERT_NE(code.find("int64_t i = flua_for_ctrl_"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_count_loop.lua", {.debug_mode = debug_mode});
        int r = 0;

        Call(s, type, "test_count_to", r, 5);
        ASSERT_EQ(r, 5);
        Call(s, type, "test_count_to", r, 0);
        ASSERT_EQ(r, 0);
        Call(s, type, "test_count_to", r, 100);
        ASSERT_EQ(r, 100);

        Call(s, type, "test_count_from", r, 5);
        ASSERT_EQ(r, 5);
        Call(s, type, "test_count_from", r, 1);
        ASSERT_EQ(r, 1);
        Call(s, type, "test_count_from", r, 10);
        ASSERT_EQ(r, 10);

        Call(s, type, "test_count_step", r, 5);
        ASSERT_EQ(r, 21);
        Call(s, type, "test_count_step", r, 10);
        ASSERT_EQ(r, 11);
        Call(s, type, "test_count_step", r, 1);
        ASSERT_EQ(r, 101);
    });
}

// ────────────────────────────────────────────────────────────────────────────
// 遗漏语法补全测试
// ────────────────────────────────────────────────────────────────────────────

// NOT_EQUAL (~=) 在 if 条件中生成原生 C 不等比较。
// n 通过 n + 1 算术运算被识别为数学参数；if 条件 n ~= 0 应发出 (n) != (0)。
// test(0) == 0, test(1) == 2, test(5) == 6, test(-1) == 0（-1+1=0）。
TEST(infer, test_native_bool_not_equal_if) {
    const auto code = InferGetCCode("./infer/test_native_bool_not_equal_if.lua");
    // n is a math param (triggered by n + 1).
    ASSERT_NE(code.find("test_0(int64_t n)"), std::string::npos);
    // The if condition must use a native != comparison, not IsTrue.
    ASSERT_NE(code.find("((n) != (0))"), std::string::npos);
    ASSERT_EQ(code.find("IsTrue"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_native_bool_not_equal_if.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 0);
        ASSERT_EQ(ret, 0);
        Call(s, type, "test", ret, 1);
        ASSERT_EQ(ret, 2);
        Call(s, type, "test", ret, 5);
        ASSERT_EQ(ret, 6);
        Call(s, type, "test", ret, -1);
        ASSERT_EQ(ret, 0);
    });
}

// LESS_EQUAL (<=) 和 MORE_EQUAL (>=) 作为唯一比较运算触发数学参数特化。
// clamp_le 仅含 <= 和 >= 比较，不含算术运算，验证这两种运算符同样能被
// IsNativeComparisonExpr 识别并触发 HasComparisonOperandTypeChange。
// test(5, 1, 10) == 5, test(0, 1, 10) == 1, test(15, 1, 10) == 10.
TEST(infer, test_spec_clamp_le) {
    const auto code = InferGetCCode("./infer/test_spec_clamp_le.lua");
    // clamp_le and test must both be specialised (three math params each).
    ASSERT_NE(code.find("clamp_le_0_0_0(int64_t x, int64_t lo, int64_t hi)"), std::string::npos);
    ASSERT_NE(code.find("test_0_0_0(int64_t x, int64_t lo, int64_t hi)"), std::string::npos);
    // Entry dispatchers must exist.
    ASSERT_NE(code.find("CVar clamp_le(CVar x, CVar lo, CVar hi)"), std::string::npos);
    ASSERT_NE(code.find("CVar test(CVar x, CVar lo, CVar hi)"), std::string::npos);
    // Both if conditions must use native C comparisons.
    ASSERT_NE(code.find("(x) <= (lo)"), std::string::npos);
    ASSERT_NE(code.find("(x) >= (hi)"), std::string::npos);
    ASSERT_EQ(code.find("IsTrue"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_spec_clamp_le.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 5, 1, 10);
        ASSERT_EQ(ret, 5); // in range
        Call(s, type, "test", ret, 0, 1, 10);
        ASSERT_EQ(ret, 1); // below lo (0 <= 1)
        Call(s, type, "test", ret, 15, 1, 10);
        ASSERT_EQ(ret, 10); // above hi (15 >= 10)
        Call(s, type, "test", ret, 1, 1, 10);
        ASSERT_EQ(ret, 1); // x == lo → x <= lo, returns lo
        Call(s, type, "test", ret, 10, 1, 10);
        ASSERT_EQ(ret, 10); // x == hi → x >= hi, returns hi
        double dret = 0.0;
        Call(s, type, "test", dret, 5.5, 1.0, 10.0);
        ASSERT_DOUBLE_EQ(dret, 5.5);
        Call(s, type, "test", dret, 0.5, 1.0, 10.0);
        ASSERT_DOUBLE_EQ(dret, 1.0);
        Call(s, type, "test", dret, 15.5, 1.0, 10.0);
        ASSERT_DOUBLE_EQ(dret, 10.0);
    });
}

// "local f = function(n) ... end" 在顶层与 "local function f(n) ... end" 等价，
// 经预处理转换后应参与数学参数特化流程。
// square(n) = n*n：n 是数学参数，int 特化返回 int64_t。
// test(n) 调用 square(n)：通过 HasMathCallImprovement 传递特化。
// test(3) == 9, test(4) == 16, test(2.0) == 4.0.
TEST(infer, test_spec_funcdef_assignment) {
    const auto code = InferGetCCode("./infer/test_spec_funcdef_assignment.lua");
    // square must be specialised via the functiondef assignment conversion.
    ASSERT_NE(code.find("square_0(int64_t n)"), std::string::npos);
    // test must be specialised too (math call improvement via square).
    ASSERT_NE(code.find("test_0(int64_t n)"), std::string::npos);
    // Entry dispatchers must exist.
    ASSERT_NE(code.find("CVar square(CVar n)"), std::string::npos);
    ASSERT_NE(code.find("CVar test(CVar n)"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_spec_funcdef_assignment.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 3);
        ASSERT_EQ(ret, 9);
        Call(s, type, "test", ret, 4);
        ASSERT_EQ(ret, 16);
        Call(s, type, "test", ret, 0);
        ASSERT_EQ(ret, 0);
        double dret = 0.0;
        Call(s, type, "test", dret, 2.0);
        ASSERT_DOUBLE_EQ(dret, 4.0);
        Call(s, type, "test", dret, 1.5);
        ASSERT_DOUBLE_EQ(dret, 2.25);
    });
}

// ────────────────────────────────────────────────────────────────────────────
// 遗漏语法补全测试（二）
// ────────────────────────────────────────────────────────────────────────────

// not (comparison) 作为 if 条件，直接使用数学参数（无局部变量中间体）。
// n 通过 n < 0（IsNativeComparisonExpr: LESS）和算术 n + 1 被识别为数学参数。
// if 条件 not (n < 0) 应生成原生 C 布尔 !((n) < (0))，不使用 IsTrue。
// test(5) == 6, test(0) == 1, test(-1) == 0.
TEST(infer, test_spec_not_if_cond) {
    const auto code = InferGetCCode("./infer/test_spec_not_if_cond.lua");
    // n is a math param (triggered by both n < 0 comparison and n + 1 arithmetic).
    ASSERT_NE(code.find("test_0(int64_t n)"), std::string::npos);
    ASSERT_NE(code.find("test_1(double n)"), std::string::npos);
    // The if condition must use a native negated comparison, not IsTrue.
    ASSERT_NE(code.find("!((n) < (0))"), std::string::npos);
    ASSERT_EQ(code.find("IsTrue"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_spec_not_if_cond.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 5);
        ASSERT_EQ(ret, 6);
        Call(s, type, "test", ret, 0);
        ASSERT_EQ(ret, 1);
        Call(s, type, "test", ret, -1);
        ASSERT_EQ(ret, 0);
        double dret = 0.0;
        Call(s, type, "test", dret, 5.5);
        ASSERT_DOUBLE_EQ(dret, 6.5);
        Call(s, type, "test", dret, -0.5);
        ASSERT_DOUBLE_EQ(dret, 0.0);
    });
}

// 复合 and 条件：if a > 0 and b > 0 then。
// a 和 b 均通过比较（IsNativeComparisonExpr: MORE）和算术 a + b 被识别为数学参数。
// if 条件应生成原生 C 布尔 ((a) > (0)) && ((b) > (0))，不使用 IsTrue。
// test(3, 4) == 7, test(-1, 4) == 0, test(3, -1) == 0, test(0, 0) == 0.
TEST(infer, test_spec_and_cond) {
    const auto code = InferGetCCode("./infer/test_spec_and_cond.lua");
    // Both a and b are math params.
    ASSERT_NE(code.find("test_0_0(int64_t a, int64_t b)"), std::string::npos);
    ASSERT_NE(code.find("test_1_1(double a, double b)"), std::string::npos);
    // Entry dispatcher must exist.
    ASSERT_NE(code.find("CVar test(CVar a, CVar b)"), std::string::npos);
    // The if condition must use native && comparison, not IsTrue.
    ASSERT_NE(code.find("((a) > (0)) && ((b) > (0))"), std::string::npos);
    ASSERT_EQ(code.find("IsTrue"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_spec_and_cond.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 3, 4);
        ASSERT_EQ(ret, 7);
        Call(s, type, "test", ret, -1, 4);
        ASSERT_EQ(ret, 0);
        Call(s, type, "test", ret, 3, -1);
        ASSERT_EQ(ret, 0);
        Call(s, type, "test", ret, 0, 0);
        ASSERT_EQ(ret, 0);
        double dret = 0.0;
        Call(s, type, "test", dret, 1.5, 2.5);
        ASSERT_DOUBLE_EQ(dret, 4.0);
        Call(s, type, "test", dret, -1.0, 2.5);
        ASSERT_DOUBLE_EQ(dret, 0.0);
    });
}

// 复合 or 条件：if n < 0 or n > 10 then。
// n 通过比较（IsNativeComparisonExpr: LESS, MORE）和算术 n + 1 被识别为数学参数。
// if 条件应生成原生 C 布尔 ((n) < (0)) || ((n) > (10))，不使用 IsTrue。
// test(5) == 6, test(-1) == 0, test(11) == 0, test(0) == 1, test(10) == 11.
TEST(infer, test_spec_or_cond) {
    const auto code = InferGetCCode("./infer/test_spec_or_cond.lua");
    // n is a math param.
    ASSERT_NE(code.find("test_0(int64_t n)"), std::string::npos);
    ASSERT_NE(code.find("test_1(double n)"), std::string::npos);
    // Entry dispatcher must exist.
    ASSERT_NE(code.find("CVar test(CVar n)"), std::string::npos);
    // The if condition must use native || comparison, not IsTrue.
    ASSERT_NE(code.find("((n) < (0)) || ((n) > (10))"), std::string::npos);
    ASSERT_EQ(code.find("IsTrue"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_spec_or_cond.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 5);
        ASSERT_EQ(ret, 6);
        Call(s, type, "test", ret, -1);
        ASSERT_EQ(ret, 0);
        Call(s, type, "test", ret, 11);
        ASSERT_EQ(ret, 0);
        Call(s, type, "test", ret, 0);
        ASSERT_EQ(ret, 1);
        Call(s, type, "test", ret, 10);
        ASSERT_EQ(ret, 11);
        double dret = 0.0;
        Call(s, type, "test", dret, 5.5);
        ASSERT_DOUBLE_EQ(dret, 6.5);
        Call(s, type, "test", dret, -0.5);
        ASSERT_DOUBLE_EQ(dret, 0.0);
        Call(s, type, "test", dret, 10.5);
        ASSERT_DOUBLE_EQ(dret, 0.0);
    });
}

// while 循环条件 not (comparison)，直接使用数学参数（无局部变量中间体）。
// n 通过比较（LESS）和算术（s + n, n - 1）被识别为数学参数。
// while 条件 not (n < 0) 应生成原生 C 布尔 !((n) < (0))，不使用 IsTrue。
// test(0) == 0, test(1) == 1, test(5) == 15 (= 5+4+3+2+1+0).
TEST(infer, test_spec_while_not_cond) {
    const auto code = InferGetCCode("./infer/test_spec_while_not_cond.lua");
    // n is a math param (triggered by arithmetic and comparison).
    ASSERT_NE(code.find("test_0(int64_t n)"), std::string::npos);
    ASSERT_NE(code.find("test_1(double n)"), std::string::npos);
    // The while condition must use a native negated comparison, not IsTrue.
    ASSERT_NE(code.find("!((n) < (0))"), std::string::npos);
    ASSERT_EQ(code.find("IsTrue"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_spec_while_not_cond.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 0);
        ASSERT_EQ(ret, 0);
        Call(s, type, "test", ret, 1);
        ASSERT_EQ(ret, 1);
        Call(s, type, "test", ret, 5);
        ASSERT_EQ(ret, 15);
        double dret = 0.0;
        Call(s, type, "test", dret, 3.0);
        ASSERT_DOUBLE_EQ(dret, 6.0); // 3.0 + 2.0 + 1.0 + 0.0
    });
}

// .. 拼接运算符与算术运算共存时，不应阻止 n 的数学参数特化。
// n 通过算术 n + 1 被识别为数学参数；拼接结果为 T_DYNAMIC，
// 不在 IsArithmeticExpr 中，因此不干扰特化发现流程。
// test(5) == 6, test(0) == 1, test(-1) == 0, test(3) == 4.
TEST(infer, test_spec_concat_no_interfere) {
    const auto code = InferGetCCode("./infer/test_spec_concat_no_interfere.lua");
    // n IS a math param (triggered by n + 1).
    ASSERT_NE(code.find("test_0(int64_t n)"), std::string::npos);
    ASSERT_NE(code.find("test_1(double n)"), std::string::npos);
    // Entry dispatcher must exist.
    ASSERT_NE(code.find("CVar test(CVar n)"), std::string::npos);
    // The arithmetic n + 1 must use the native int fast path.
    ASSERT_NE(code.find("((n) + (1))"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_spec_concat_no_interfere.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret, 5);
        ASSERT_EQ(ret, 6);
        Call(s, type, "test", ret, 0);
        ASSERT_EQ(ret, 1);
        Call(s, type, "test", ret, -1);
        ASSERT_EQ(ret, 0);
        Call(s, type, "test", ret, 3);
        ASSERT_EQ(ret, 4);
        double dret = 0.0;
        Call(s, type, "test", dret, 2.5);
        ASSERT_DOUBLE_EQ(dret, 3.5);
    });
}

// 仅含 and/or 运算符（无算术、无有序比较）的函数不应被特化。
// a 和 b 仅用于 and 运算，可接受任意 Lua 类型（nil、字符串、数值等），
// ParamAffectsArithmetic 对两者均返回 false，不会生成特化版本。
TEST(infer, test_spec_and_or_only_no_spec) {
    const auto code = InferGetCCode("./infer/test_spec_and_or_only_no_spec.lua");
    // No specialization should be generated: no test_0 or test_1 variants.
    ASSERT_EQ(code.find("test_0("), std::string::npos);
    ASSERT_EQ(code.find("test_1("), std::string::npos);
    // Only the generic CVar entry function should exist.
    ASSERT_NE(code.find("CVar test(CVar a, CVar b)"), std::string::npos);

    InferRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./infer/test_spec_and_or_only_no_spec.lua", {.debug_mode = debug_mode});
        int ret = 0;
        // In Lua: 1 and 2 = 2, 0 and 2 = 2 (0 is truthy), 3 and 4 = 4.
        Call(s, type, "test", ret, 1, 2);
        ASSERT_EQ(ret, 2);
        Call(s, type, "test", ret, 0, 2);
        ASSERT_EQ(ret, 2);
        Call(s, type, "test", ret, 3, 4);
        ASSERT_EQ(ret, 4);
    });
}
