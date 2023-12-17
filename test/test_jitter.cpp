#include "compile/compiler.h"
#include "fakelua.h"
#include "gtest/gtest.h"

using namespace fakelua;

#ifndef WIN32

static int jitter_gccjit_called_with[3];

extern "C" __attribute__((used)) void jitter_gccjit_called_function(int i, int j, int k) {
    jitter_gccjit_called_with[0] = i;
    jitter_gccjit_called_with[1] = j;
    jitter_gccjit_called_with[2] = k;
}

TEST(jitter, gccjit) {
    gcc_jit_context *ctxt = gcc_jit_context_acquire();
    /* Let's try to inject the equivalent of:
     extern void called_function (int i, int j, int k);

    void
    test_caller (int a)
    {
        called_function (a * 3, a * 4, a * 5);
    }
    */
    int i;
    gcc_jit_type *void_type = gcc_jit_context_get_type(ctxt, GCC_JIT_TYPE_VOID);
    gcc_jit_type *int_type = gcc_jit_context_get_type(ctxt, GCC_JIT_TYPE_INT);

    /* Declare the imported function.  */
    gcc_jit_param *params[3];
    params[0] = gcc_jit_context_new_param(ctxt, NULL, int_type, "i");
    params[1] = gcc_jit_context_new_param(ctxt, NULL, int_type, "j");
    params[2] = gcc_jit_context_new_param(ctxt, NULL, int_type, "k");
    gcc_jit_function *called_fn =
            gcc_jit_context_new_function(ctxt, NULL, GCC_JIT_FUNCTION_IMPORTED, void_type, "jitter_gccjit_called_function", 3, params, 0);

    /* Build the test_fn.  */
    gcc_jit_param *param_a = gcc_jit_context_new_param(ctxt, NULL, int_type, "a");
    gcc_jit_function *test_fn =
            gcc_jit_context_new_function(ctxt, NULL, GCC_JIT_FUNCTION_EXPORTED, void_type, "test_caller", 1, &param_a, 0);
    /* "a * 3, a * 4, a * 5"  */
    gcc_jit_rvalue *args[3];
    for (i = 0; i < 3; i++)
        args[i] = gcc_jit_context_new_binary_op(ctxt, NULL, GCC_JIT_BINARY_OP_MULT, int_type, gcc_jit_param_as_rvalue(param_a),
                                                gcc_jit_context_new_rvalue_from_int(ctxt, int_type, (i + 3)));
    gcc_jit_block *block = gcc_jit_function_new_block(test_fn, NULL);
    gcc_jit_block_add_eval(block, NULL, gcc_jit_context_new_call(ctxt, NULL, called_fn, 3, args));
    gcc_jit_block_end_with_void_return(block, NULL);

    /* Compile the function.  */
    gcc_jit_result *result = gcc_jit_context_compile(ctxt);
    ASSERT_NE(result, nullptr);

    gcc_jit_context_release(ctxt);

    typedef void (*fn_type)(int);

    fn_type test_caller = (fn_type) gcc_jit_result_get_code(result, "test_caller");
    ASSERT_NE(test_caller, nullptr);

    jitter_gccjit_called_with[0] = 0;
    jitter_gccjit_called_with[1] = 0;
    jitter_gccjit_called_with[2] = 0;

    /* Call the JIT-generated function.  */
    test_caller(5);

    /* Verify that it correctly called "called_function".  */
    ASSERT_EQ(jitter_gccjit_called_with[0], 15);
    ASSERT_EQ(jitter_gccjit_called_with[1], 20);
    ASSERT_EQ(jitter_gccjit_called_with[2], 25);

    gcc_jit_result_release(result);
}

#endif

TEST(jitter, empty_file) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    L->compile_file("./jit/test_empty_file.lua", {});
    L->compile_file("./jit/test_empty_file.lua", {debug_mode: false});
}

TEST(jitter, empty_func) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    L->compile_file("./jit/test_empty_func.lua", {});
    var *ret = 0;
    L->call("test", std::tie(ret));
    ASSERT_NE(ret, nullptr);
    ASSERT_EQ(ret->type(), var_type::VAR_NIL);

    L->compile_file("./jit/test_empty_func.lua", {debug_mode: false});
    L->call("test", std::tie(ret));
    ASSERT_NE(ret, nullptr);
    ASSERT_EQ(ret->type(), var_type::VAR_NIL);
}

TEST(jitter, multi_return) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    L->compile_file("./jit/test_multi_return.lua", {});
    int i = 0;
    float f = 0;
    bool b1 = false;
    bool b2 = false;
    std::string s;
    L->call("test", std::tie(i, f, b1, b2, s));
    ASSERT_EQ(i, 1);
    ASSERT_EQ(std::abs(f - 2.3) < 0.001, true);
    ASSERT_EQ(b1, false);
    ASSERT_EQ(b2, true);
    ASSERT_EQ(s, "test");

    L->compile_file("./jit/test_multi_return.lua", {debug_mode: false});
    i = 0;
    f = 0;
    b1 = false;
    b2 = false;
    s.clear();
    L->call("test", std::tie(i, f, b1, b2, s));
    ASSERT_EQ(i, 1);
    ASSERT_EQ(std::abs(f - 2.3) < 0.001, true);
    ASSERT_EQ(b1, false);
    ASSERT_EQ(b2, true);
    ASSERT_EQ(s, "test");
}

TEST(jitter, multi_name) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    L->compile_file("./jit/test_multi_name_func.lua", {});
    var *ret = 0;
    L->call("_G_my_test", std::tie(ret));
    ASSERT_NE(ret, nullptr);
    ASSERT_EQ(ret->type(), var_type::VAR_INT);
    ASSERT_EQ(ret->get_int(), 1);

    L->compile_file("./jit/test_multi_name_func.lua", {debug_mode: false});
    L->call("_G_my_test", std::tie(ret));
    ASSERT_NE(ret, nullptr);
    ASSERT_EQ(ret->type(), var_type::VAR_INT);
    ASSERT_EQ(ret->get_int(), 1);
}

TEST(jitter, multi_col_name) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    L->compile_file("./jit/test_multi_col_name_func.lua", {});
    var *ret = 0;
    L->call("_G_my_test", std::tie(ret));
    ASSERT_NE(ret, nullptr);
    ASSERT_EQ(ret->type(), var_type::VAR_INT);
    ASSERT_EQ(ret->get_int(), 1);

    L->compile_file("./jit/test_multi_col_name_func.lua", {debug_mode: false});
    L->call("_G_my_test", std::tie(ret));
    ASSERT_NE(ret, nullptr);
    ASSERT_EQ(ret->type(), var_type::VAR_INT);
    ASSERT_EQ(ret->get_int(), 1);
}

TEST(jitter, const_define) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int i = 0;
    L->compile_file("./jit/test_const_define.lua", {});
    L->call("test", std::tie(i));
    ASSERT_EQ(i, 1);
    L->compile_file("./jit/test_const_define.lua", {debug_mode: false});
    L->call("test", std::tie(i));
    ASSERT_EQ(i, 1);
}

TEST(jitter, multi_const_define) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    var *ret0 = (var *) 0xFF;
    int ret1 = 0;
    bool ret2 = false;
    bool ret3 = false;
    std::string ret4;
    double ret5;
    L->compile_file("./jit/test_multi_const_define.lua", {});
    L->call("test", std::tie(ret0, ret1, ret2, ret3, ret4, ret5));
    ASSERT_EQ(ret0->type(), var_type::VAR_NIL);
    ASSERT_EQ(ret1, 1);
    ASSERT_EQ(ret2, false);
    ASSERT_EQ(ret3, true);
    ASSERT_EQ(ret4, "test");
    ASSERT_EQ(std::abs(ret5 - 2.3) < 0.001, true);
    L->compile_file("./jit/test_multi_const_define.lua", {debug_mode: false});
    ret0 = (var *) 0xFF;
    ret1 = 0;
    ret2 = false;
    ret3 = false;
    ret4.clear();
    ret5 = 0;
    L->call("test", std::tie(ret0, ret1, ret2, ret3, ret4, ret5));
    ASSERT_EQ(ret0->type(), var_type::VAR_NIL);
    ASSERT_EQ(ret1, 1);
    ASSERT_EQ(ret2, false);
    ASSERT_EQ(ret3, true);
    ASSERT_EQ(ret4, "test");
    ASSERT_EQ(std::abs(ret5 - 2.3) < 0.001, true);
}
