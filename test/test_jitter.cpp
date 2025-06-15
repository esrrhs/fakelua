#include "compile/compiler.h"
#include "fakelua.h"
#include "var/var_string.h"
#include "var/var_table.h"
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

static void jitter_run_helper(const std::function<void(bool)> &f) {
    f(true);
    f(false);
}

TEST(jitter, empty_file) {
    jitter_run_helper([](bool debug_mode) {
        const auto L = fakelua_newstate();
        ASSERT_NE(L.get(), nullptr);

        L->compile_file("./jit/test_empty_file.lua", {.debug_mode = debug_mode});
    });
}

TEST(jitter, empty_func) {
    jitter_run_helper([](bool debug_mode) {
        const auto L = fakelua_newstate();
        ASSERT_NE(L.get(), nullptr);

        L->compile_file("./jit/test_empty_func.lua", {.debug_mode = debug_mode});
        cvar ret;
        const auto v = static_cast<var &>(ret);
        L->call("test", std::tie(ret));
        ASSERT_EQ(v.type(), var_type::VAR_NIL);
    });
}

TEST(jitter, empty_local_func) {
    jitter_run_helper([](bool debug_mode) {
        const auto L = fakelua_newstate();
        ASSERT_NE(L.get(), nullptr);

        cvar ret;
        const auto v = static_cast<var &>(ret);
        L->compile_file("./jit/test_empty_local_func.lua", {});
        L->call("test", std::tie(ret));
        ASSERT_EQ(v.type(), var_type::VAR_NIL);
    });
}

TEST(jitter, multi_return) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int i = 0;
    float f = 0;
    bool b1 = false;
    bool b2 = false;
    std::string s;
    L->compile_file("./jit/test_multi_return.lua", {});
    L->call("test", std::tie(i, f, b1, b2, s));
    ASSERT_EQ(i, 1);
    ASSERT_NEAR(f, 2.3, 0.001);
    ASSERT_EQ(b1, false);
    ASSERT_EQ(b2, true);
    ASSERT_EQ(s, "test");

    i = 0;
    f = 0;
    b1 = false;
    b2 = false;
    s.clear();
    L->compile_file("./jit/test_multi_return.lua", {.debug_mode = false});
    L->call("test", std::tie(i, f, b1, b2, s));
    ASSERT_EQ(i, 1);
    ASSERT_NEAR(f, 2.3, 0.001);
    ASSERT_EQ(b1, false);
    ASSERT_EQ(b2, true);
    ASSERT_EQ(s, "test");
}

TEST(jitter, multi_return_call) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    std::string s;
    int i = 0;
    float f = 0;
    L->compile_file("./jit/test_multi_return_call.lua", {});
    L->call("test", std::tie(s, i, f));
    ASSERT_EQ(s, "test");
    ASSERT_EQ(i, 1);
    ASSERT_NEAR(f, 2.3, 0.001);

    s.clear();
    i = 0;
    f = 0;
    L->compile_file("./jit/test_multi_return_call.lua", {.debug_mode = false});
    L->call("test", std::tie(s, i, f));
    ASSERT_EQ(s, "test");
    ASSERT_EQ(i, 1);
    ASSERT_NEAR(f, 2.3, 0.001);
}

TEST(jitter, multi_return_call_ex) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    std::string s;
    int i = 0;
    float f = 0;
    L->compile_file("./jit/test_multi_return_call_ex.lua", {});
    L->call("test", std::tie(s, i, f));
    ASSERT_EQ(s, "test");
    ASSERT_EQ(i, 1);
    ASSERT_NEAR(f, 2.4, 0.001);

    s.clear();
    i = 0;
    f = 0;
    L->compile_file("./jit/test_multi_return_call_ex.lua", {.debug_mode = false});
    L->call("test", std::tie(s, i, f));
    ASSERT_EQ(s, "test");
    ASSERT_EQ(i, 1);
    ASSERT_NEAR(f, 2.4, 0.001);
}

TEST(jitter, multi_return_sub) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    std::string s;
    int i = 0;
    float f = 0;
    L->compile_file("./jit/test_multi_return_sub.lua", {});
    L->call("test", std::tie(s, i, f));
    ASSERT_EQ(s, "test");
    ASSERT_EQ(i, 1);
    ASSERT_NEAR(f, 2.3, 0.001);

    s.clear();
    i = 0;
    f = 0;
    L->compile_file("./jit/test_multi_return_sub.lua", {.debug_mode = false});
    L->call("test", std::tie(s, i, f));
    ASSERT_EQ(s, "test");
    ASSERT_EQ(i, 1);
    ASSERT_NEAR(f, 2.3, 0.001);
}

TEST(jitter, multi_return_multi) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int i1 = 0;
    int i2 = 0;
    int i3 = 0;
    int i4 = 0;
    int i5 = 0;
    L->compile_file("./jit/test_multi_return_multi.lua", {});
    L->call("test", std::tie(i1, i2, i3, i4, i5));
    ASSERT_EQ(i1, 1);
    ASSERT_EQ(i2, 2);
    ASSERT_EQ(i3, 3);
    ASSERT_EQ(i4, 4);
    ASSERT_EQ(i5, 5);

    i1 = 0;
    i2 = 0;
    i3 = 0;
    i4 = 0;
    i5 = 0;
    L->compile_file("./jit/test_multi_return_multi.lua", {.debug_mode = false});
    L->call("test", std::tie(i1, i2, i3, i4, i5));
    ASSERT_EQ(i1, 1);
    ASSERT_EQ(i2, 2);
    ASSERT_EQ(i3, 3);
    ASSERT_EQ(i4, 4);
    ASSERT_EQ(i5, 5);
}

TEST(jitter, multi_return_multi_ex) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int i1 = 0;
    int i2 = 0;
    int i3 = 0;
    int i4 = 0;
    L->compile_file("./jit/test_multi_return_multi_ex.lua", {});
    L->call("test", std::tie(i1, i2, i3, i4));
    ASSERT_EQ(i1, 1);
    ASSERT_EQ(i2, 2);
    ASSERT_EQ(i3, 3);
    ASSERT_EQ(i4, 6);

    i1 = 0;
    i2 = 0;
    i3 = 0;
    i4 = 0;
    L->compile_file("./jit/test_multi_return_multi_ex.lua", {.debug_mode = false});
    L->call("test", std::tie(i1, i2, i3, i4));
    ASSERT_EQ(i1, 1);
    ASSERT_EQ(i2, 2);
    ASSERT_EQ(i3, 3);
    ASSERT_EQ(i4, 6);
}

TEST(jitter, multi_name) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int ret = 0;
    L->compile_file("./jit/test_multi_name_func.lua", {});
    L->call("test", std::tie(ret), 1);
    ASSERT_EQ(ret, 2);

    ret = 0;
    L->compile_file("./jit/test_multi_name_func.lua", {.debug_mode = false});
    L->call("test", std::tie(ret), 1);
    ASSERT_EQ(ret, 2);
}

TEST(jitter, multi_col_name) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int ret = 0;
    L->compile_file("./jit/test_multi_col_name_func.lua", {});
    L->call("test", std::tie(ret), 1);
    ASSERT_EQ(ret, 2);

    ret = 0;
    L->compile_file("./jit/test_multi_col_name_func.lua", {.debug_mode = false});
    L->call("test", std::tie(ret), 1);
    ASSERT_EQ(ret, 2);
}

TEST(jitter, const_define) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int i = 0;
    L->compile_file("./jit/test_const_define.lua", {});
    L->call("test", std::tie(i));
    ASSERT_EQ(i, 1);

    i = 0;
    L->compile_file("./jit/test_const_define.lua", {.debug_mode = false});
    L->call("test", std::tie(i));
    ASSERT_EQ(i, 1);
}

TEST(jitter, multi_const_define) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    cvar ret0 = {};
    auto v = (var &) ret0;
    int ret1 = 0;
    bool ret2 = false;
    bool ret3 = false;
    std::string ret4;
    double ret5;
    L->compile_file("./jit/test_multi_const_define.lua", {});
    L->call("test", std::tie(ret0, ret1, ret2, ret3, ret4, ret5));
    ASSERT_EQ(v.type(), var_type::VAR_NIL);
    ASSERT_EQ(ret1, 1);
    ASSERT_EQ(ret2, false);
    ASSERT_EQ(ret3, true);
    ASSERT_EQ(ret4, "test");
    ASSERT_NEAR(ret5, 2.3, 0.001);
    L->compile_file("./jit/test_multi_const_define.lua", {.debug_mode = false});
    ret0 = {};
    ret1 = 0;
    ret2 = false;
    ret3 = false;
    ret4.clear();
    ret5 = 0;
    L->call("test", std::tie(ret0, ret1, ret2, ret3, ret4, ret5));
    ASSERT_EQ(v.type(), var_type::VAR_NIL);
    ASSERT_EQ(ret1, 1);
    ASSERT_EQ(ret2, false);
    ASSERT_EQ(ret3, true);
    ASSERT_EQ(ret4, "test");
    ASSERT_NEAR(ret5, 2.3, 0.001);
}

TEST(jitter, empty_func_with_params) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int ret1 = 0;
    bool ret2 = false;
    std::string ret3;
    double ret4 = 0;
    L->compile_file("./jit/test_empty_func_with_params.lua", {});
    L->call("test", std::tie(ret1, ret2, ret3, ret4), 2.3, "test", true, 1);
    ASSERT_EQ(ret1, 1);
    ASSERT_EQ(ret2, true);
    ASSERT_EQ(ret3, "test");
    ASSERT_NEAR(ret4, 2.3, 0.001);

    ret1 = 0;
    ret2 = false;
    ret3.clear();
    ret4 = 0;
    L->compile_file("./jit/test_empty_func_with_params.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2, ret3, ret4), 2.3, "test", true, 1);
    ASSERT_EQ(ret1, 1);
    ASSERT_EQ(ret2, true);
    ASSERT_EQ(ret3, "test");
    ASSERT_NEAR(ret4, 2.3, 0.001);
}

TEST(jitter, variadic_func) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int ret1 = 0;
    bool ret2 = false;
    std::string ret3;
    double ret4 = 0;
    L->compile_file("./jit/test_variadic_func.lua", {});
    L->call("test", std::tie(ret1, ret2, ret3, ret4), 1, true, "test", 2.3);
    ASSERT_EQ(ret1, 1);
    ASSERT_EQ(ret2, true);
    ASSERT_EQ(ret3, "test");
    ASSERT_NEAR(ret4, 2.3, 0.001);

    ret1 = 0;
    ret2 = false;
    ret3.clear();
    ret4 = 0;
    L->compile_file("./jit/test_variadic_func.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2, ret3, ret4), 1, true, "test", 2.3);
    ASSERT_EQ(ret1, 1);
    ASSERT_EQ(ret2, true);
    ASSERT_EQ(ret3, "test");
    ASSERT_NEAR(ret4, 2.3, 0.001);
}

TEST(jitter, variadic_func_with_params) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    std::string ret1;
    bool ret2 = false;
    int ret3 = 0;
    L->compile_file("./jit/test_variadic_func_with_params.lua", {});
    L->call("test", std::tie(ret1, ret2, ret3), 1, true, "test", 2.3);
    ASSERT_EQ(ret1, "test");
    ASSERT_EQ(ret2, true);
    ASSERT_EQ(ret3, 1);

    ret1.clear();
    ret2 = false;
    ret3 = 0;
    L->compile_file("./jit/test_variadic_func_with_params.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2, ret3), 1, true, "test", 2.3);
    ASSERT_EQ(ret1, "test");
    ASSERT_EQ(ret2, true);
    ASSERT_EQ(ret3, 1);
}

TEST(jitter, variadic_func_multi_type) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    char in1 = 1, out1 = 0;
    unsigned char in2 = 2, out2 = 0;
    short in3 = 3, out3 = 0;
    unsigned short in4 = 4, out4 = 0;
    unsigned int in5 = 5, out5 = 0;
    long in6 = 6, out6 = 0;
    unsigned long in7 = 7, out7 = 0;
    long long in8 = 8, out8 = 0;
    unsigned long long in9 = 9, out9 = 0;
    float in10 = 10, out10 = 0;
    char *in11 = (char *) "11", *out11 = nullptr;
    std::string in12 = "12", out12;
    std::string_view in13 = "13", out13;
    const char *in14 = "14", *out14 = nullptr;
    float in15 = 15.1, out15 = 0;
    double in16 = 16.2, out16 = 0;
    double in17 = 17, out17 = 0;
    L->compile_file("./jit/test_variadic_func.lua", {});
    L->call("test", std::tie(out1, out2, out3, out4, out5, out6, out7, out8, out9, out10, out11, out12, out13, out14, out15, out16, out17),
            in1, in2, in3, in4, in5, in6, in7, in8, in9, in10, in11, in12, in13, in14, in15, in16, in17);
    ASSERT_EQ(out1, in1);
    ASSERT_EQ(out2, in2);
    ASSERT_EQ(out3, in3);
    ASSERT_EQ(out4, in4);
    ASSERT_EQ(out5, in5);
    ASSERT_EQ(out6, in6);
    ASSERT_EQ(out7, in7);
    ASSERT_EQ(out8, in8);
    ASSERT_EQ(out9, in9);
    ASSERT_EQ(out10, in10);
    ASSERT_STREQ(out11, in11);
    ASSERT_EQ(out12, in12);
    ASSERT_EQ(out13, in13);
    ASSERT_STREQ(out14, in14);
    ASSERT_NEAR(out15, in15, 0.001);
    ASSERT_NEAR(out16, in16, 0.001);
    ASSERT_NEAR(out17, in17, 0.001);

    out1 = 0;
    out2 = 0;
    out3 = 0;
    out4 = 0;
    out5 = 0;
    out6 = 0;
    out7 = 0;
    out8 = 0;
    out9 = 0;
    out10 = 0;
    out11 = nullptr;
    out12.clear();
    out13 = "";
    L->compile_file("./jit/test_variadic_func.lua", {.debug_mode = false});
    L->call("test", std::tie(out1, out2, out3, out4, out5, out6, out7, out8, out9, out10, out11, out12, out13, out14, out15, out16, out17),
            in1, in2, in3, in4, in5, in6, in7, in8, in9, in10, in11, in12, in13, in14, in15, in16, in17);
    ASSERT_EQ(out1, in1);
    ASSERT_EQ(out2, in2);
    ASSERT_EQ(out3, in3);
    ASSERT_EQ(out4, in4);
    ASSERT_EQ(out5, in5);
    ASSERT_EQ(out6, in6);
    ASSERT_EQ(out7, in7);
    ASSERT_EQ(out8, in8);
    ASSERT_EQ(out9, in9);
    ASSERT_EQ(out10, in10);
    ASSERT_STREQ(out11, in11);
    ASSERT_EQ(out12, in12);
    ASSERT_EQ(out13, in13);
    ASSERT_STREQ(out14, in14);
    ASSERT_NEAR(out15, in15, 0.001);
    ASSERT_NEAR(out16, in16, 0.001);
    ASSERT_NEAR(out17, in17, 0.001);
}

TEST(jitter, variadic_func_vi) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    std::vector<var_interface *> tmp;
    auto newfunc = [&]() {
        auto ret = new simple_var_impl();
        tmp.push_back(ret);
        return ret;
    };
    L->set_var_interface_new_func(newfunc);

    // construct a table below by use simple_var_impl:
    //    {
    //        a = 1,
    //        b = "test",
    //        c = 2.3,
    //        d = true,
    //        e = {
    //            f = 1,
    //        }
    //    }
    simple_var_impl *var = newfunc();
    std::vector<std::pair<var_interface *, var_interface *>> kv;
    auto k = newfunc();
    k->vi_set_string("a");
    auto v = newfunc();
    v->vi_set_int(1);
    kv.emplace_back(k, v);
    k = newfunc();
    k->vi_set_string("b");
    v = newfunc();
    v->vi_set_string("test");
    kv.emplace_back(k, v);
    k = newfunc();
    k->vi_set_string("c");
    v = newfunc();
    v->vi_set_float(2.3);
    kv.emplace_back(k, v);
    k = newfunc();
    k->vi_set_string("d");
    v = newfunc();
    v->vi_set_bool(true);
    kv.emplace_back(k, v);
    k = newfunc();
    k->vi_set_string("e");
    v = newfunc();
    std::vector<std::pair<var_interface *, var_interface *>> sub_kv;
    auto sub_k = newfunc();
    sub_k->vi_set_string("f");
    auto sub_v = newfunc();
    sub_v->vi_set_int(1);
    sub_kv.emplace_back(sub_k, sub_v);
    v->vi_set_table(sub_kv);
    kv.emplace_back(k, v);
    var->vi_set_table(kv);

    auto dumpstr = var->vi_to_string(0);
    ASSERT_EQ(dumpstr, "table:\n"
                       "\t[\"a\"] = 1\n"
                       "\t[\"b\"] = \"test\"\n"
                       "\t[\"c\"] = 2.300000\n"
                       "\t[\"d\"] = true\n"
                       "\t[\"e\"] = table:\n"
                       "\t\t[\"f\"] = 1");

    var_interface *ret = nullptr;
    L->compile_file("./jit/test_variadic_func.lua", {});
    L->call("test", std::tie(ret), var);
    ASSERT_NE(ret, nullptr);
    ASSERT_EQ(ret->vi_get_type(), var_interface::type::TABLE);

    // need sort kv
    dynamic_cast<simple_var_impl *>(ret)->vi_sort_table();
    ASSERT_EQ(ret->vi_to_string(0), dumpstr);

    ret = nullptr;
    L->compile_file("./jit/test_variadic_func.lua", {.debug_mode = false});
    L->call("test", std::tie(ret), var);
    ASSERT_NE(ret, nullptr);
    ASSERT_EQ(ret->vi_get_type(), var_interface::type::TABLE);

    // need sort kv
    dynamic_cast<simple_var_impl *>(ret)->vi_sort_table();
    ASSERT_EQ(ret->vi_to_string(0), dumpstr);

    for (auto &i: tmp) {
        delete i;
    }
}

TEST(jitter, variadic_func_with_empty) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    L->compile_file("./jit/test_variadic_func.lua", {});
    L->call("test", std::tie());

    L->compile_file("./jit/test_variadic_func.lua", {.debug_mode = false});
    L->call("test", std::tie());
}

TEST(jitter, variadic_func_vi_array) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    std::vector<var_interface *> tmp;
    auto newfunc = [&]() {
        auto ret = new simple_var_impl();
        tmp.push_back(ret);
        return ret;
    };
    L->set_var_interface_new_func(newfunc);

    // construct a table below by use simple_var_impl:
    //    {
    //        1, nil, 2
    //    }
    simple_var_impl *var = newfunc();
    std::vector<std::pair<var_interface *, var_interface *>> kv;
    auto k = newfunc();
    k->vi_set_int(1);
    auto v = newfunc();
    v->vi_set_int(1);
    kv.emplace_back(k, v);
    k = newfunc();
    k->vi_set_int(2);
    v = newfunc();
    v->vi_set_int(2);
    kv.emplace_back(k, v);
    k = newfunc();
    k->vi_set_int(3);
    v = newfunc();
    v->vi_set_int(3);
    kv.emplace_back(k, v);
    var->vi_set_table(kv);

    auto dumpstr = var->vi_to_string(0);
    ASSERT_EQ(dumpstr, "table:\n"
                       "\t[1] = 1\n"
                       "\t[2] = 2\n"
                       "\t[3] = 3");


    var_interface *ret = nullptr;
    L->compile_file("./jit/test_variadic_func.lua", {});
    L->call("test", std::tie(ret), var);
    ASSERT_NE(ret, nullptr);
    ASSERT_EQ(ret->vi_get_type(), var_interface::type::TABLE);

    // need sort kv
    dynamic_cast<simple_var_impl *>(ret)->vi_sort_table();
    ASSERT_EQ(ret->vi_to_string(0), dumpstr);

    ret = nullptr;
    L->compile_file("./jit/test_variadic_func.lua", {.debug_mode = false});
    L->call("test", std::tie(ret), var);
    ASSERT_NE(ret, nullptr);
    ASSERT_EQ(ret->vi_get_type(), var_interface::type::TABLE);

    // need sort kv
    dynamic_cast<simple_var_impl *>(ret)->vi_sort_table();
    ASSERT_EQ(ret->vi_to_string(0), dumpstr);

    for (auto &i: tmp) {
        delete i;
    }
}

TEST(jitter, variadic_func_vi_nil) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    std::vector<var_interface *> tmp;
    auto newfunc = [&]() {
        auto ret = new simple_var_impl();
        tmp.push_back(ret);
        return ret;
    };
    L->set_var_interface_new_func(newfunc);

    simple_var_impl *var = newfunc();
    var->vi_set_nil();

    auto dumpstr = var->vi_to_string(0);
    ASSERT_EQ(dumpstr, "nil");

    var_interface *ret = nullptr;
    L->compile_file("./jit/test_variadic_func.lua", {});
    L->call("test", std::tie(ret), var);
    ASSERT_NE(ret, nullptr);
    ASSERT_EQ(ret->vi_get_type(), var_interface::type::NIL);

    ret = nullptr;
    L->compile_file("./jit/test_variadic_func.lua", {.debug_mode = false});
    L->call("test", std::tie(ret), var);
    ASSERT_NE(ret, nullptr);
    ASSERT_EQ(ret->vi_get_type(), var_interface::type::NIL);

    for (auto &i: tmp) {
        delete i;
    }
}

TEST(jitter, string) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int ret = 0;
    L->compile_string("function test() return 1 end", {});
    L->call("test", std::tie(ret));
    ASSERT_EQ(ret, 1);

    L->compile_string("function test() return 1 end", {.debug_mode = false});
    L->call("test", std::tie(ret));
    ASSERT_EQ(ret, 1);
}

TEST(jitter, local_define) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    cvar a;
    auto va = (var &) a;
    cvar b;
    auto vb = (var &) b;
    L->compile_file("./jit/test_local_define.lua", {});
    L->call("test", std::tie(a, b));
    ASSERT_EQ(va.type(), var_type::VAR_NIL);

    ASSERT_EQ(vb.type(), var_type::VAR_NIL);

    a = {};
    b = {};
    L->compile_file("./jit/test_local_define.lua", {.debug_mode = false});
    L->call("test", std::tie(a, b));
    ASSERT_EQ(va.type(), var_type::VAR_NIL);

    ASSERT_EQ(vb.type(), var_type::VAR_NIL);
}

TEST(jitter, local_define_with_values) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    cvar a = {};
    auto va = (var &) a;
    cvar b = {};
    auto vb = (var &) b;
    cvar c = {};
    auto vc = (var &) c;
    cvar d = {};
    auto vd = (var &) d;
    cvar e = {};
    auto ve = (var &) e;
    L->compile_file("./jit/test_local_define_with_value.lua", {});
    L->call("test", std::tie(a, b, c, d, e), true, 2);
    ASSERT_EQ(va.type(), var_type::VAR_BOOL);
    ASSERT_EQ(va.get_bool(), true);
    ASSERT_EQ(vb.type(), var_type::VAR_INT);
    ASSERT_EQ(vb.get_int(), 2);
    ASSERT_EQ(vc.type(), var_type::VAR_INT);
    ASSERT_EQ(vc.get_int(), 1);
    ASSERT_EQ(vd.type(), var_type::VAR_STRING);
    ASSERT_EQ(vd.get_string()->str(), "test");
    ASSERT_EQ(ve.type(), var_type::VAR_NIL);

    a = {};
    b = {};
    c = {};
    d = {};
    e = {};
    L->compile_file("./jit/test_local_define_with_value.lua", {.debug_mode = false});
    L->call("test", std::tie(a, b, c, d, e), true, 2);
    ASSERT_EQ(va.type(), var_type::VAR_BOOL);
    ASSERT_EQ(va.get_bool(), true);
    ASSERT_EQ(vb.type(), var_type::VAR_INT);
    ASSERT_EQ(vb.get_int(), 2);
    ASSERT_EQ(vc.type(), var_type::VAR_INT);
    ASSERT_EQ(vc.get_int(), 1);
    ASSERT_EQ(vd.type(), var_type::VAR_STRING);
    ASSERT_EQ(vd.get_string()->str(), "test");
    ASSERT_EQ(ve.type(), var_type::VAR_NIL);
}

TEST(jitter, test_assign) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int a = 0;
    std::string b;
    L->compile_file("./jit/test_assign.lua", {});
    L->call("test", std::tie(a, b), true, 1.1);
    ASSERT_EQ(a, 1);
    ASSERT_EQ(b, "2");

    a = 0;
    b.clear();
    L->compile_file("./jit/test_assign.lua", {.debug_mode = false});
    L->call("test", std::tie(a, b), true, 1.1);
    ASSERT_EQ(a, 1);
    ASSERT_EQ(b, "2");
}

TEST(jitter, test_assign_not_match) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int a = 0;
    std::string b;
    int c = 0;
    int d = 0;
    L->compile_file("./jit/test_assign_not_match.lua", {});
    L->call("test", std::tie(a, b, c, d), true, "2", 1.1, 1);
    ASSERT_EQ(a, 1);
    ASSERT_EQ(b, "2");
    ASSERT_EQ(c, 3);
    ASSERT_EQ(d, 4);

    a = 0;
    b.clear();
    L->compile_file("./jit/test_assign_not_match.lua", {.debug_mode = false});
    L->call("test", std::tie(a, b, c, d), true, "2", 1.1, 1);
    ASSERT_EQ(a, 1);
    ASSERT_EQ(b, "2");
    ASSERT_EQ(c, 3);
    ASSERT_EQ(d, 4);
}

TEST(jitter, test_assign_variadic_match) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    std::string a;
    int b = 0;
    L->compile_file("./jit/test_assign_variadic.lua", {});
    L->call("test", std::tie(a, b), 1, "2");
    ASSERT_EQ(a, "2");
    ASSERT_EQ(b, 1);

    a.clear();
    b = 0;
    L->compile_file("./jit/test_assign_variadic.lua", {.debug_mode = false});
    L->call("test", std::tie(a, b), 1, "2");
    ASSERT_EQ(a, "2");
    ASSERT_EQ(b, 1);
}

TEST(jitter, test_assign_variadic_no_match) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int a = 0;
    int b = 0;
    L->compile_file("./jit/test_assign_variadic_no_match.lua", {});
    L->call("test", std::tie(a, b), 2, "2");
    ASSERT_EQ(a, 1);
    ASSERT_EQ(b, 2);

    a = 0;
    b = 0;
    L->compile_file("./jit/test_assign_variadic_no_match.lua", {.debug_mode = false});
    L->call("test", std::tie(a, b), 2, "2");
    ASSERT_EQ(a, 1);
    ASSERT_EQ(b, 2);
}

TEST(jitter, test_assign_variadic_empty) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int a = 0;
    cvar b = {};
    auto vb = (var &) b;
    L->compile_file("./jit/test_assign_variadic_no_match.lua", {});
    L->call("test", std::tie(a, b));
    ASSERT_EQ(a, 1);
    ASSERT_EQ(vb.type(), var_type::VAR_NIL);

    a = 0;
    b = {};
    L->compile_file("./jit/test_assign_variadic_no_match.lua", {.debug_mode = false});
    L->call("test", std::tie(a, b));
    ASSERT_EQ(a, 1);
    ASSERT_EQ(vb.type(), var_type::VAR_NIL);
}

TEST(jitter, test_const_table) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    std::vector<var_interface *> tmp;
    auto newfunc = [&]() {
        auto ret = new simple_var_impl();
        tmp.push_back(ret);
        return ret;
    };
    L->set_var_interface_new_func(newfunc);

    var_interface *t1 = nullptr;
    var_interface *t2 = nullptr;
    var_interface *t3 = nullptr;
    L->compile_file("./jit/test_const_table.lua", {});
    L->call("test", std::tie(t1, t2, t3));
    ASSERT_NE(t1, nullptr);
    ASSERT_EQ(t1->vi_get_type(), var_interface::type::TABLE);
    ASSERT_NE(t2, nullptr);
    ASSERT_EQ(t2->vi_get_type(), var_interface::type::TABLE);
    ASSERT_NE(t3, nullptr);
    ASSERT_EQ(t3->vi_get_type(), var_interface::type::TABLE);

    // need sort kv
    dynamic_cast<simple_var_impl *>(t1)->vi_sort_table();
    dynamic_cast<simple_var_impl *>(t2)->vi_sort_table();
    dynamic_cast<simple_var_impl *>(t3)->vi_sort_table();
    ASSERT_EQ(t1->vi_to_string(0),
              "table:\n\t[1] = 1\n\t[2] = 2\n\t[3] = 3\n\t[4] = 4\n\t[5] = 5\n\t[6] = 6\n\t[7] = 7\n\t[8] = 8\n\t[9] = 9\n\t[10] = 10");
    ASSERT_EQ(t2->vi_to_string(0), "table:\n\t[\"a\"] = 1\n\t[\"b\"] = 2\n\t[\"c\"] = 3");
    ASSERT_EQ(t3->vi_to_string(0), "table:\n\t[1] = 1\n\t[2] = 3\n\t[3] = 5\n\t[\"b\"] = 2\n\t[\"d\"] = 4");

    t1 = nullptr;
    t2 = nullptr;
    t3 = nullptr;
    L->compile_file("./jit/test_const_table.lua", {.debug_mode = false});
    L->call("test", std::tie(t1, t2, t3));
    ASSERT_NE(t1, nullptr);
    ASSERT_EQ(t1->vi_get_type(), var_interface::type::TABLE);
    ASSERT_NE(t2, nullptr);
    ASSERT_EQ(t2->vi_get_type(), var_interface::type::TABLE);
    ASSERT_NE(t3, nullptr);
    ASSERT_EQ(t3->vi_get_type(), var_interface::type::TABLE);

    // need sort kv
    dynamic_cast<simple_var_impl *>(t1)->vi_sort_table();
    dynamic_cast<simple_var_impl *>(t2)->vi_sort_table();
    dynamic_cast<simple_var_impl *>(t3)->vi_sort_table();
    ASSERT_EQ(t1->vi_to_string(0),
              "table:\n\t[1] = 1\n\t[2] = 2\n\t[3] = 3\n\t[4] = 4\n\t[5] = 5\n\t[6] = 6\n\t[7] = 7\n\t[8] = 8\n\t[9] = 9\n\t[10] = 10");
    ASSERT_EQ(t2->vi_to_string(0), "table:\n\t[\"a\"] = 1\n\t[\"b\"] = 2\n\t[\"c\"] = 3");
    ASSERT_EQ(t3->vi_to_string(0), "table:\n\t[1] = 1\n\t[2] = 3\n\t[3] = 5\n\t[\"b\"] = 2\n\t[\"d\"] = 4");

    for (auto &i: tmp) {
        delete i;
    }
}

TEST(jitter, test_const_nested_table) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    std::vector<var_interface *> tmp;
    auto newfunc = [&]() {
        auto ret = new simple_var_impl();
        tmp.push_back(ret);
        return ret;
    };
    L->set_var_interface_new_func(newfunc);

    var_interface *t = nullptr;
    L->compile_file("./jit/test_const_nested_table.lua", {});
    L->call("test", std::tie(t));
    ASSERT_NE(t, nullptr);
    ASSERT_EQ(t->vi_get_type(), var_interface::type::TABLE);

    // need sort kv
    dynamic_cast<simple_var_impl *>(t)->vi_sort_table();
    ASSERT_EQ(t->vi_to_string(0),
              "table:\n\t[\"array\"] = table:\n\t\t[1] = 1\n\t\t[2] = 2\n\t\t[3] = 3\n\t[\"map\"] = table:\n\t\t[\"a\"] "
              "= 1\n\t\t[\"b\"] = 2\n\t\t[\"c\"] = 3");

    t = nullptr;
    L->compile_file("./jit/test_const_nested_table.lua", {.debug_mode = false});
    L->call("test", std::tie(t));
    ASSERT_NE(t, nullptr);
    ASSERT_EQ(t->vi_get_type(), var_interface::type::TABLE);

    // need sort kv
    dynamic_cast<simple_var_impl *>(t)->vi_sort_table();
    ASSERT_EQ(t->vi_to_string(0),
              "table:\n\t[\"array\"] = table:\n\t\t[1] = 1\n\t\t[2] = 2\n\t\t[3] = 3\n\t[\"map\"] = table:\n\t\t[\"a\"] "
              "= 1\n\t\t[\"b\"] = 2\n\t\t[\"c\"] = 3");

    for (auto &i: tmp) {
        delete i;
    }
}

TEST(jitter, test_local_table) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    std::vector<var_interface *> tmp;
    auto newfunc = [&]() {
        auto ret = new simple_var_impl();
        tmp.push_back(ret);
        return ret;
    };
    L->set_var_interface_new_func(newfunc);

    var_interface *t1 = nullptr;
    var_interface *t2 = nullptr;
    var_interface *t3 = nullptr;
    L->compile_file("./jit/test_local_table.lua", {});
    L->call("test", std::tie(t1, t2, t3));
    ASSERT_NE(t1, nullptr);
    ASSERT_EQ(t1->vi_get_type(), var_interface::type::TABLE);
    ASSERT_NE(t2, nullptr);
    ASSERT_EQ(t2->vi_get_type(), var_interface::type::TABLE);
    ASSERT_NE(t3, nullptr);
    ASSERT_EQ(t3->vi_get_type(), var_interface::type::TABLE);

    // need sort kv
    dynamic_cast<simple_var_impl *>(t1)->vi_sort_table();
    dynamic_cast<simple_var_impl *>(t2)->vi_sort_table();
    dynamic_cast<simple_var_impl *>(t3)->vi_sort_table();
    ASSERT_EQ(t1->vi_to_string(0),
              "table:\n\t[1] = 1\n\t[2] = 2\n\t[3] = 3\n\t[4] = 4\n\t[5] = 5\n\t[6] = 6\n\t[7] = 7\n\t[8] = 8\n\t[9] = 9\n\t[10] = 10");
    ASSERT_EQ(t2->vi_to_string(0), "table:\n\t[\"a\"] = 1\n\t[\"b\"] = 2\n\t[\"c\"] = 3");
    ASSERT_EQ(t3->vi_to_string(0), "table:\n\t[1] = 1\n\t[2] = 3\n\t[3] = 5\n\t[\"b\"] = 2\n\t[\"d\"] = 4");

    t1 = nullptr;
    t2 = nullptr;
    t3 = nullptr;
    L->compile_file("./jit/test_local_table.lua", {.debug_mode = false});
    L->call("test", std::tie(t1, t2, t3));
    ASSERT_NE(t1, nullptr);
    ASSERT_EQ(t1->vi_get_type(), var_interface::type::TABLE);
    ASSERT_NE(t2, nullptr);
    ASSERT_EQ(t2->vi_get_type(), var_interface::type::TABLE);
    ASSERT_NE(t3, nullptr);
    ASSERT_EQ(t3->vi_get_type(), var_interface::type::TABLE);

    // need sort kv
    dynamic_cast<simple_var_impl *>(t1)->vi_sort_table();
    dynamic_cast<simple_var_impl *>(t2)->vi_sort_table();
    dynamic_cast<simple_var_impl *>(t3)->vi_sort_table();
    ASSERT_EQ(t1->vi_to_string(0),
              "table:\n\t[1] = 1\n\t[2] = 2\n\t[3] = 3\n\t[4] = 4\n\t[5] = 5\n\t[6] = 6\n\t[7] = 7\n\t[8] = 8\n\t[9] = 9\n\t[10] = 10");
    ASSERT_EQ(t2->vi_to_string(0), "table:\n\t[\"a\"] = 1\n\t[\"b\"] = 2\n\t[\"c\"] = 3");
    ASSERT_EQ(t3->vi_to_string(0), "table:\n\t[1] = 1\n\t[2] = 3\n\t[3] = 5\n\t[\"b\"] = 2\n\t[\"d\"] = 4");

    for (auto &i: tmp) {
        delete i;
    }
}

TEST(jitter, test_local_nested_table) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    std::vector<var_interface *> tmp;
    auto newfunc = [&]() {
        auto ret = new simple_var_impl();
        tmp.push_back(ret);
        return ret;
    };
    L->set_var_interface_new_func(newfunc);

    var_interface *t = nullptr;
    L->compile_file("./jit/test_local_nested_table.lua", {});
    L->call("test", std::tie(t));
    ASSERT_NE(t, nullptr);
    ASSERT_EQ(t->vi_get_type(), var_interface::type::TABLE);

    // need sort kv
    dynamic_cast<simple_var_impl *>(t)->vi_sort_table();
    ASSERT_EQ(t->vi_to_string(0),
              "table:\n\t[\"array\"] = table:\n\t\t[1] = 1\n\t\t[2] = 2\n\t\t[3] = 3\n\t[\"map\"] = table:\n\t\t[\"a\"] "
              "= 1\n\t\t[\"b\"] = 2\n\t\t[\"c\"] = 3");

    t = nullptr;
    L->compile_file("./jit/test_local_nested_table.lua", {.debug_mode = false});
    L->call("test", std::tie(t));
    ASSERT_NE(t, nullptr);
    ASSERT_EQ(t->vi_get_type(), var_interface::type::TABLE);

    // need sort kv
    dynamic_cast<simple_var_impl *>(t)->vi_sort_table();
    ASSERT_EQ(t->vi_to_string(0),
              "table:\n\t[\"array\"] = table:\n\t\t[1] = 1\n\t\t[2] = 2\n\t\t[3] = 3\n\t[\"map\"] = table:\n\t\t[\"a\"] "
              "= 1\n\t\t[\"b\"] = 2\n\t\t[\"c\"] = 3");

    for (auto &i: tmp) {
        delete i;
    }
}

TEST(jitter, test_local_table_with_variadic) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    std::vector<var_interface *> tmp;
    auto newfunc = [&]() {
        auto ret = new simple_var_impl();
        tmp.push_back(ret);
        return ret;
    };
    L->set_var_interface_new_func(newfunc);

    var_interface *t = nullptr;
    L->compile_file("./jit/test_local_table_with_variadic.lua", {});
    L->call("test", std::tie(t), "a", "b", "c");
    ASSERT_NE(t, nullptr);
    ASSERT_EQ(t->vi_get_type(), var_interface::type::TABLE);

    // need sort kv
    dynamic_cast<simple_var_impl *>(t)->vi_sort_table();
    ASSERT_EQ(t->vi_to_string(0), "table:\n\t[1] = \"a\"\n\t[2] = \"b\"\n\t[3] = \"c\"");

    t = nullptr;
    L->compile_file("./jit/test_local_table_with_variadic.lua", {.debug_mode = false});
    L->call("test", std::tie(t), "a", "b", "c");
    ASSERT_NE(t, nullptr);
    ASSERT_EQ(t->vi_get_type(), var_interface::type::TABLE);

    // need sort kv
    dynamic_cast<simple_var_impl *>(t)->vi_sort_table();
    ASSERT_EQ(t->vi_to_string(0), "table:\n\t[1] = \"a\"\n\t[2] = \"b\"\n\t[3] = \"c\"");

    for (auto &i: tmp) {
        delete i;
    }
}

TEST(jitter, test_local_table_with_variadic_no_end) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    std::vector<var_interface *> tmp;
    auto newfunc = [&]() {
        auto ret = new simple_var_impl();
        tmp.push_back(ret);
        return ret;
    };
    L->set_var_interface_new_func(newfunc);

    var_interface *t = nullptr;
    L->compile_file("./jit/test_local_table_with_variadic_no_end.lua", {});
    L->call("test", std::tie(t), "a", "b", "c", "d", "e");
    ASSERT_NE(t, nullptr);
    ASSERT_EQ(t->vi_get_type(), var_interface::type::TABLE);

    // need sort kv
    dynamic_cast<simple_var_impl *>(t)->vi_sort_table();
    ASSERT_EQ(t->vi_to_string(0), "table:\n\t[1] = \"c\"\n\t[2] = \"a\"\n\t[3] = \"b\"");

    t = nullptr;
    L->compile_file("./jit/test_local_table_with_variadic_no_end.lua", {.debug_mode = false});
    L->call("test", std::tie(t), "a", "b", "c", "d", "e");
    ASSERT_NE(t, nullptr);
    ASSERT_EQ(t->vi_get_type(), var_interface::type::TABLE);

    // need sort kv
    dynamic_cast<simple_var_impl *>(t)->vi_sort_table();
    ASSERT_EQ(t->vi_to_string(0), "table:\n\t[1] = \"c\"\n\t[2] = \"a\"\n\t[3] = \"b\"");

    for (auto &i: tmp) {
        delete i;
    }
}

TEST(jitter, test_local_table_with_variadic_no_end_replace) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    std::vector<var_interface *> tmp;
    auto newfunc = [&]() {
        auto ret = new simple_var_impl();
        tmp.push_back(ret);
        return ret;
    };
    L->set_var_interface_new_func(newfunc);

    var_interface *t = nullptr;
    L->compile_file("./jit/test_local_table_with_variadic_no_end_replace.lua", {});
    L->call("test", std::tie(t), "a", "b", "c", "d", "e");
    ASSERT_NE(t, nullptr);
    ASSERT_EQ(t->vi_get_type(), var_interface::type::TABLE);

    // need sort kv
    dynamic_cast<simple_var_impl *>(t)->vi_sort_table();
    ASSERT_EQ(t->vi_to_string(0), "table:\n\t[1] = \"c\"\n\t[2] = \"b\"");

    t = nullptr;
    L->compile_file("./jit/test_local_table_with_variadic_no_end_replace.lua", {.debug_mode = false});
    L->call("test", std::tie(t), "a", "b", "c", "d", "e");
    ASSERT_NE(t, nullptr);
    ASSERT_EQ(t->vi_get_type(), var_interface::type::TABLE);

    // need sort kv
    dynamic_cast<simple_var_impl *>(t)->vi_sort_table();
    ASSERT_EQ(t->vi_to_string(0), "table:\n\t[1] = \"c\"\n\t[2] = \"b\"");

    for (auto &i: tmp) {
        delete i;
    }
}

TEST(jitter, compile_empty_string) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    L->compile_string("", {});
}

TEST(jitter, test_assign_simple_var) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int a = 0;
    int b = 0;
    L->compile_file("./jit/test_assign_simple_var.lua", {});
    L->call("test", std::tie(a, b), "test", 1);
    ASSERT_EQ(a, 1);
    ASSERT_EQ(b, 1);

    a = 0;
    b = 0;
    L->compile_file("./jit/test_assign_simple_var.lua", {.debug_mode = false});
    L->call("test", std::tie(a, b), "test", 1);
    ASSERT_EQ(a, 1);
    ASSERT_EQ(b, 1);
}

TEST(jitter, test_const_define_simple_var) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int a = 0;
    int b = 0;
    int c = 0;
    L->compile_file("./jit/test_const_define_simple_var.lua", {});
    L->call("test", std::tie(a, b, c));
    ASSERT_EQ(a, 1);
    ASSERT_EQ(b, 1);
    ASSERT_EQ(c, 1);

    a = 0;
    b = 0;
    L->compile_file("./jit/test_const_define_simple_var.lua", {.debug_mode = false});
    L->call("test", std::tie(a, b, c));
    ASSERT_EQ(a, 1);
    ASSERT_EQ(b, 1);
    ASSERT_EQ(c, 1);
}

TEST(jitter, test_binop_plus) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int ret1 = 0;
    double ret2 = 0;
    L->compile_file("./jit/test_binop_plus.lua", {});
    L->call("test", std::tie(ret1, ret2), 1, 2, 1.1, 2.2);
    ASSERT_EQ(ret1, 3);
    ASSERT_NEAR(ret2, 3.3, 0.001);

    ret1 = 0;
    ret2 = 0;
    L->compile_file("./jit/test_binop_plus.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2), "1", 2, "1.1", 2.2);
    ASSERT_EQ(ret1, 3);
    ASSERT_NEAR(ret2, 3.3, 0.001);
}

TEST(jitter, test_const_binop_plus) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int ret1 = 0;
    double ret2 = 0;
    L->compile_file("./jit/test_const_binop_plus.lua", {});
    L->call("test", std::tie(ret1, ret2));
    ASSERT_EQ(ret1, 3);
    ASSERT_NEAR(ret2, 3.2, 0.001);

    ret1 = 0;
    ret2 = 0;
    L->compile_file("./jit/test_const_binop_plus.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2));
    ASSERT_EQ(ret1, 3);
    ASSERT_NEAR(ret2, 3.2, 0.001);
}

TEST(jitter, test_binop_minus) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int ret1 = 0;
    double ret2 = 0;
    L->compile_file("./jit/test_binop_minus.lua", {});
    L->call("test", std::tie(ret1, ret2), 1, 2, 2, 1.2);
    ASSERT_EQ(ret1, -1);
    ASSERT_NEAR(ret2, 0.8, 0.001);

    ret1 = 0;
    ret2 = 0;
    L->compile_file("./jit/test_binop_minus.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2), "1", "2", 2, 1.2);
    ASSERT_EQ(ret1, -1);
    ASSERT_NEAR(ret2, 0.8, 0.001);
}

TEST(jitter, test_const_binop_minus) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int ret1 = 0;
    double ret2 = 0;
    L->compile_file("./jit/test_const_binop_minus.lua", {});
    L->call("test", std::tie(ret1, ret2));
    ASSERT_EQ(ret1, -1);
    ASSERT_NEAR(ret2, 1.1, 0.001);

    ret1 = 0;
    ret2 = 0;
    L->compile_file("./jit/test_const_binop_minus.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2));
    ASSERT_EQ(ret1, -1);
    ASSERT_NEAR(ret2, 1.1, 0.001);
}

TEST(jitter, test_binop_star) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int ret1 = 0;
    double ret2 = 0;
    L->compile_file("./jit/test_binop_star.lua", {});
    L->call("test", std::tie(ret1, ret2), 1, 2, 2, 1.2);
    ASSERT_EQ(ret1, 4);
    ASSERT_NEAR(ret2, 1.4, 0.001);

    ret1 = 0;
    ret2 = 0;
    L->compile_file("./jit/test_binop_star.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2), "1", "2", 2, 1.2);
    ASSERT_EQ(ret1, 4);
    ASSERT_NEAR(ret2, 1.4, 0.001);
}

TEST(jitter, test_const_binop_star) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    double ret1 = 0;
    double ret2 = 0;
    L->compile_file("./jit/test_const_binop_star.lua", {});
    L->call("test", std::tie(ret1, ret2));
    ASSERT_NEAR(ret1, 3.2, 0.001);
    ASSERT_NEAR(ret2, 0.1, 0.001);

    ret1 = 0;
    ret2 = 0;
    L->compile_file("./jit/test_const_binop_star.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2));
    ASSERT_NEAR(ret1, 3.2, 0.001);
    ASSERT_NEAR(ret2, 0.1, 0.001);
}

TEST(jitter, test_empty_return) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    cvar ret = {};
    auto v = (var &) ret;
    L->compile_file("./jit/test_empty_return.lua", {});
    L->call("test", std::tie(ret));
    ASSERT_EQ(v.type(), var_type::VAR_NIL);

    ret = {};
    L->compile_file("./jit/test_empty_return.lua", {.debug_mode = false});
    L->call("test", std::tie(ret));
    ASSERT_EQ(v.type(), var_type::VAR_NIL);
}

TEST(jitter, test_empty_func_no_return) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    cvar ret = {};
    auto v = (var &) ret;
    L->compile_file("./jit/test_empty_func_no_return.lua", {});
    L->call("test", std::tie(ret));
    ASSERT_EQ(v.type(), var_type::VAR_NIL);

    ret = {};
    L->compile_file("./jit/test_empty_func_no_return.lua", {.debug_mode = false});
    L->call("test", std::tie(ret));
    ASSERT_EQ(v.type(), var_type::VAR_NIL);
}

TEST(jitter, test_binop_slash) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    double ret1 = 0;
    double ret2 = 0;
    L->compile_file("./jit/test_binop_slash.lua", {});
    L->call("test", std::tie(ret1, ret2), 1, 2, 2.4, 1.2);
    ASSERT_NEAR(ret1, 2.5, 0.001);
    ASSERT_NEAR(ret2, 1, 0.001);

    ret1 = 0;
    ret2 = 0;
    L->compile_file("./jit/test_binop_slash.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2), "1", "2", 2.4, 1.2);
    ASSERT_NEAR(ret1, 2.5, 0.001);
    ASSERT_NEAR(ret2, 1, 0.001);
}

TEST(jitter, test_const_binop_slash) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    double ret1 = 0;
    double ret2 = 0;
    L->compile_file("./jit/test_const_binop_slash.lua", {});
    L->call("test", std::tie(ret1, ret2));
    ASSERT_NEAR(ret1, 1.7, 0.001);
    ASSERT_NEAR(ret2, 0.1, 0.001);

    ret1 = 0;
    ret2 = 0;
    L->compile_file("./jit/test_const_binop_slash.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2));
    ASSERT_NEAR(ret1, 1.7, 0.001);
    ASSERT_NEAR(ret2, 0.1, 0.001);
}

TEST(jitter, test_binop_double_slash) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int ret1 = 0;
    int ret2 = 0;
    L->compile_file("./jit/test_binop_double_slash.lua", {});
    L->call("test", std::tie(ret1, ret2), 3, 2, 2.4, 1.2);
    ASSERT_EQ(ret1, 3);
    ASSERT_EQ(ret2, 1);

    ret1 = 0;
    ret2 = 0;
    L->compile_file("./jit/test_binop_double_slash.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2), 3, 2, 2.4, 1.2);
    ASSERT_EQ(ret1, 3);
    ASSERT_EQ(ret2, 1);
}

TEST(jitter, test_const_binop_double_slash) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    double ret1 = 0;
    double ret2 = 0;
    L->compile_file("./jit/test_const_binop_double_slash.lua", {});
    L->call("test", std::tie(ret1, ret2));
    ASSERT_NEAR(ret1, 2.2, 0.001);
    ASSERT_NEAR(ret2, -0.1, 0.001);

    ret1 = 0;
    ret2 = 0;
    L->compile_file("./jit/test_const_binop_double_slash.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2));
    ASSERT_NEAR(ret1, 2.2, 0.001);
    ASSERT_NEAR(ret2, -0.1, 0.001);
}

TEST(jitter, test_binop_pow) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int ret1 = 0;
    double ret2 = 0;
    L->compile_file("./jit/test_binop_pow.lua", {});
    L->call("test", std::tie(ret1, ret2), 3, 2, 2.4, 1.2);
    ASSERT_EQ(ret1, 11);
    ASSERT_NEAR(ret2, 1.859258955601, 0.001);

    ret1 = 0;
    ret2 = 0;
    L->compile_file("./jit/test_binop_pow.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2), 3, 2, 2.4, 1.2);
    ASSERT_EQ(ret1, 11);
    ASSERT_NEAR(ret2, 1.859258955601, 0.001);
}

TEST(jitter, test_const_binop_pow) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int ret1 = 0;
    double ret2 = 0;
    L->compile_file("./jit/test_const_binop_pow.lua", {});
    L->call("test", std::tie(ret1, ret2));
    ASSERT_EQ(ret1, 8);
    ASSERT_NEAR(ret2, 1.2332863005547, 0.001);

    ret1 = 0;
    ret2 = 0;
    L->compile_file("./jit/test_const_binop_pow.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2));
    ASSERT_EQ(ret1, 8);
    ASSERT_NEAR(ret2, 1.2332863005547, 0.001);
}

TEST(jitter, test_binop_mod) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int ret1 = 0;
    int ret2 = 0;
    L->compile_file("./jit/test_binop_mod.lua", {});
    L->call("test", std::tie(ret1, ret2), 3, 2, 2.4, 1.2);
    ASSERT_EQ(ret1, 3);
    ASSERT_EQ(ret2, -1);

    ret1 = 0;
    ret2 = 0;
    L->compile_file("./jit/test_binop_mod.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2), 3, 2, 2.4, 1.2);
    ASSERT_EQ(ret1, 3);
    ASSERT_EQ(ret2, -1);
}

TEST(jitter, test_const_binop_mod) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int ret1 = 0;
    int ret2 = 0;
    L->compile_file("./jit/test_const_binop_mod.lua", {});
    L->call("test", std::tie(ret1, ret2));
    ASSERT_EQ(ret1, 2);
    ASSERT_EQ(ret2, 0);

    ret1 = 0;
    ret2 = 0;
    L->compile_file("./jit/test_const_binop_mod.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2));
    ASSERT_EQ(ret1, 2);
    ASSERT_EQ(ret2, 0);
}

TEST(jitter, test_binop_bitand) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int ret1 = 0;
    int ret2 = 0;
    L->compile_file("./jit/test_binop_bitand.lua", {});
    L->call("test", std::tie(ret1, ret2), 3, 2, "4", 12);
    ASSERT_EQ(ret1, 0);
    ASSERT_EQ(ret2, 0);

    ret1 = 0;
    ret2 = 0;
    L->compile_file("./jit/test_binop_bitand.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2), 3, 2, "4", 12);
    ASSERT_EQ(ret1, 0);
    ASSERT_EQ(ret2, 0);
}

TEST(jitter, test_const_binop_bitand) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int ret1 = 0;
    int ret2 = 0;
    L->compile_file("./jit/test_const_binop_bitand.lua", {});
    L->call("test", std::tie(ret1, ret2));
    ASSERT_EQ(ret1, 2);
    ASSERT_EQ(ret2, -124);

    ret1 = 0;
    ret2 = 0;
    L->compile_file("./jit/test_const_binop_bitand.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2));
    ASSERT_EQ(ret1, 2);
    ASSERT_EQ(ret2, -124);
}

TEST(jitter, test_binop_xor) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int ret1 = 0;
    int ret2 = 0;
    L->compile_file("./jit/test_binop_xor.lua", {});
    L->call("test", std::tie(ret1, ret2), 3, 2, "4", 12);
    ASSERT_EQ(ret1, 7);
    ASSERT_EQ(ret2, 15);

    ret1 = 0;
    ret2 = 0;
    L->compile_file("./jit/test_binop_xor.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2), 3, 2, "4", 12);
    ASSERT_EQ(ret1, 7);
    ASSERT_EQ(ret2, 15);
}

TEST(jitter, test_const_binop_xor) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int ret1 = 0;
    int ret2 = 0;
    L->compile_file("./jit/test_const_binop_xor.lua", {});
    L->call("test", std::tie(ret1, ret2));
    ASSERT_EQ(ret1, 1);
    ASSERT_EQ(ret2, 18);

    ret1 = 0;
    ret2 = 0;
    L->compile_file("./jit/test_const_binop_xor.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2));
    ASSERT_EQ(ret1, 1);
    ASSERT_EQ(ret2, 18);
}

TEST(jitter, test_binop_bitor) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int ret1 = 0;
    int ret2 = 0;
    L->compile_file("./jit/test_binop_bitor.lua", {});
    L->call("test", std::tie(ret1, ret2), 3, 2, "4", 12);
    ASSERT_EQ(ret1, 7);
    ASSERT_EQ(ret2, 15);

    ret1 = 0;
    ret2 = 0;
    L->compile_file("./jit/test_binop_bitor.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2), 3, 2, "4", 12);
    ASSERT_EQ(ret1, 7);
    ASSERT_EQ(ret2, 15);
}

TEST(jitter, test_const_binop_bitor) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int ret1 = 0;
    int ret2 = 0;
    L->compile_file("./jit/test_const_binop_bitor.lua", {});
    L->call("test", std::tie(ret1, ret2));
    ASSERT_EQ(ret1, 3);
    ASSERT_EQ(ret2, -123);

    ret1 = 0;
    ret2 = 0;
    L->compile_file("./jit/test_const_binop_bitor.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2));
    ASSERT_EQ(ret1, 3);
    ASSERT_EQ(ret2, -123);
}

TEST(jitter, test_binop_right_shift) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int ret1 = 0;
    int ret2 = 0;
    L->compile_file("./jit/test_binop_right_shift.lua", {});
    L->call("test", std::tie(ret1, ret2), 3, 2, "4", 12);
    ASSERT_EQ(ret1, 1);
    ASSERT_EQ(ret2, 0);

    ret1 = 0;
    ret2 = 0;
    L->compile_file("./jit/test_binop_right_shift.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2), 3, 2, "4", 12);
    ASSERT_EQ(ret1, 1);
    ASSERT_EQ(ret2, 0);
}

TEST(jitter, test_const_binop_right_shift) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int ret1 = 0;
    int64_t ret2 = 0;
    L->compile_file("./jit/test_const_binop_right_shift.lua", {});
    L->call("test", std::tie(ret1, ret2));
    ASSERT_EQ(ret1, 1776);
    ASSERT_EQ(ret2, 4611686018427387873);

    ret1 = 0;
    ret2 = 0;
    L->compile_file("./jit/test_const_binop_right_shift.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2));
    ASSERT_EQ(ret1, 1776);
    ASSERT_EQ(ret2, 4611686018427387873);
}

TEST(jitter, test_binop_left_shift) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int ret1 = 0;
    int ret2 = 0;
    L->compile_file("./jit/test_binop_left_shift.lua", {});
    L->call("test", std::tie(ret1, ret2), 3, 2, "4", 12);
    ASSERT_EQ(ret1, 20);
    ASSERT_EQ(ret2, 8192);

    ret1 = 0;
    ret2 = 0;
    L->compile_file("./jit/test_binop_left_shift.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2), 3, 2, "4", 12);
    ASSERT_EQ(ret1, 20);
    ASSERT_EQ(ret2, 8192);
}

TEST(jitter, test_const_binop_left_shift) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int ret1 = 0;
    int ret2 = 0;
    L->compile_file("./jit/test_const_binop_left_shift.lua", {});
    L->call("test", std::tie(ret1, ret2));
    ASSERT_EQ(ret1, 27);
    ASSERT_EQ(ret2, -496);

    ret1 = 0;
    ret2 = 0;
    L->compile_file("./jit/test_const_binop_left_shift.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2));
    ASSERT_EQ(ret1, 27);
    ASSERT_EQ(ret2, -496);
}

TEST(jitter, test_binop_concat) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    std::string ret;
    L->compile_file("./jit/test_binop_concat.lua", {});
    L->call("test", std::tie(ret), 3, 1.2, true, "test");
    ASSERT_EQ(ret, "31.2truetest");

    ret.clear();
    L->compile_file("./jit/test_binop_concat.lua", {.debug_mode = false});
    L->call("test", std::tie(ret), 3, 1.2, true, "test");
    ASSERT_EQ(ret, "31.2truetest");
}

TEST(jitter, test_const_binop_concat) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    std::string ret;
    L->compile_file("./jit/test_const_binop_concat.lua", {});
    L->call("test", std::tie(ret));
    ASSERT_EQ(ret, "23.2trueabcnil");

    ret.clear();
    L->compile_file("./jit/test_const_binop_concat.lua", {.debug_mode = false});
    L->call("test", std::tie(ret));
    ASSERT_EQ(ret, "23.2trueabcnil");
}

TEST(jitter, test_binop_less) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    bool ret1 = false;
    bool ret2 = false;
    L->compile_file("./jit/test_binop_less.lua", {});
    L->call("test", std::tie(ret1, ret2), 3, 1.2, 1, "10");
    ASSERT_FALSE(ret1);
    ASSERT_TRUE(ret2);

    ret1 = false;
    ret2 = false;
    L->compile_file("./jit/test_binop_less.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2), 3, 1.2, 1, "10");
    ASSERT_FALSE(ret1);
    ASSERT_TRUE(ret2);
}

TEST(jitter, test_const_binop_less) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    bool ret1 = false;
    bool ret2 = false;
    L->compile_file("./jit/test_const_binop_less.lua", {});
    L->call("test", std::tie(ret1, ret2));
    ASSERT_FALSE(ret1);
    ASSERT_TRUE(ret2);

    ret1 = false;
    ret2 = false;
    L->compile_file("./jit/test_const_binop_less.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2));
    ASSERT_FALSE(ret1);
    ASSERT_TRUE(ret2);
}

TEST(jitter, test_binop_less_equal) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    bool ret1 = false;
    bool ret2 = false;
    L->compile_file("./jit/test_binop_less_equal.lua", {});
    L->call("test", std::tie(ret1, ret2), 3, 1.2, 10, "10");
    ASSERT_FALSE(ret1);
    ASSERT_TRUE(ret2);

    ret1 = false;
    ret2 = false;
    L->compile_file("./jit/test_binop_less_equal.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2), 3, 1.2, 10, "10");
    ASSERT_FALSE(ret1);
    ASSERT_TRUE(ret2);
}

TEST(jitter, test_const_binop_less_equal) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    bool ret1 = false;
    bool ret2 = false;
    L->compile_file("./jit/test_const_binop_less_equal.lua", {});
    L->call("test", std::tie(ret1, ret2));
    ASSERT_FALSE(ret1);
    ASSERT_TRUE(ret2);

    ret1 = false;
    ret2 = false;
    L->compile_file("./jit/test_const_binop_less_equal.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2));
    ASSERT_FALSE(ret1);
    ASSERT_TRUE(ret2);
}

TEST(jitter, test_binop_more) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    bool ret1 = false;
    bool ret2 = false;
    L->compile_file("./jit/test_binop_more.lua", {});
    L->call("test", std::tie(ret1, ret2), 3, 1.2, 1, "10");
    ASSERT_TRUE(ret1);
    ASSERT_FALSE(ret2);

    ret1 = false;
    ret2 = false;
    L->compile_file("./jit/test_binop_more.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2), 3, 1.2, 1, "10");
    ASSERT_TRUE(ret1);
    ASSERT_FALSE(ret2);
}

TEST(jitter, test_const_binop_more) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    bool ret1 = false;
    bool ret2 = false;
    L->compile_file("./jit/test_const_binop_more.lua", {});
    L->call("test", std::tie(ret1, ret2));
    ASSERT_TRUE(ret1);
    ASSERT_FALSE(ret2);

    ret1 = false;
    ret2 = false;
    L->compile_file("./jit/test_const_binop_more.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2));
    ASSERT_TRUE(ret1);
    ASSERT_FALSE(ret2);
}

TEST(jitter, test_binop_more_equal) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    bool ret1 = false;
    bool ret2 = false;
    L->compile_file("./jit/test_binop_more_equal.lua", {});
    L->call("test", std::tie(ret1, ret2), 1, 1.2, 10, "10");
    ASSERT_FALSE(ret1);
    ASSERT_TRUE(ret2);

    ret1 = false;
    ret2 = false;
    L->compile_file("./jit/test_binop_more_equal.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2), 1, 1.2, 10, "10");
    ASSERT_FALSE(ret1);
    ASSERT_TRUE(ret2);
}

TEST(jitter, test_const_binop_more_equal) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    bool ret1 = false;
    bool ret2 = false;
    L->compile_file("./jit/test_const_binop_more_equal.lua", {});
    L->call("test", std::tie(ret1, ret2));
    ASSERT_TRUE(ret1);
    ASSERT_TRUE(ret2);

    ret1 = false;
    ret2 = false;
    L->compile_file("./jit/test_const_binop_more_equal.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2));
    ASSERT_TRUE(ret1);
    ASSERT_TRUE(ret2);
}

TEST(jitter, test_binop_equal) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    bool ret1 = false;
    bool ret2 = false;
    L->compile_file("./jit/test_binop_equal.lua", {});
    L->call("test", std::tie(ret1, ret2), 1, 1.2, "10", "10");
    ASSERT_FALSE(ret1);
    ASSERT_TRUE(ret2);

    ret1 = false;
    ret2 = false;
    L->compile_file("./jit/test_binop_equal.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2), 1, 1.2, "10", "10");
    ASSERT_FALSE(ret1);
    ASSERT_TRUE(ret2);
}

TEST(jitter, test_const_binop_equal) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    bool ret1 = false;
    bool ret2 = false;
    L->compile_file("./jit/test_const_binop_equal.lua", {});
    L->call("test", std::tie(ret1, ret2));
    ASSERT_FALSE(ret1);
    ASSERT_TRUE(ret2);

    ret1 = false;
    ret2 = false;
    L->compile_file("./jit/test_const_binop_equal.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2));
    ASSERT_FALSE(ret1);
    ASSERT_TRUE(ret2);
}

TEST(jitter, test_binop_not_equal) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    bool ret1 = false;
    bool ret2 = false;
    L->compile_file("./jit/test_binop_not_equal.lua", {});
    L->call("test", std::tie(ret1, ret2), 1, 1.2, "10", "10");
    ASSERT_TRUE(ret1);
    ASSERT_FALSE(ret2);

    ret1 = false;
    ret2 = false;
    L->compile_file("./jit/test_binop_not_equal.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2), 1, 1.2, "10", "10");
    ASSERT_TRUE(ret1);
    ASSERT_FALSE(ret2);
}

TEST(jitter, test_const_binop_not_equal) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    bool ret1 = false;
    bool ret2 = false;
    L->compile_file("./jit/test_const_binop_not_equal.lua", {});
    L->call("test", std::tie(ret1, ret2));
    ASSERT_TRUE(ret1);
    ASSERT_FALSE(ret2);

    ret1 = false;
    ret2 = false;
    L->compile_file("./jit/test_const_binop_not_equal.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2));
    ASSERT_TRUE(ret1);
    ASSERT_FALSE(ret2);
}

TEST(jitter, test_binop_and) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    float ret1 = 0;
    cvar ret2 = {};
    auto v = (var &) ret2;
    L->compile_file("./jit/test_binop_and.lua", {});
    L->call("test", std::tie(ret1, ret2), 1, 1.2, nullptr, "10");
    ASSERT_NEAR(ret1, 1.2, 0.001);
    ASSERT_EQ(v.type(), var_type::VAR_NIL);

    ret1 = 0;
    ret2 = {};
    L->compile_file("./jit/test_binop_and.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2), 1, 1.2, nullptr, "10");
    ASSERT_NEAR(ret1, 1.2, 0.001);
    ASSERT_EQ(v.type(), var_type::VAR_NIL);
}

TEST(jitter, test_binop_and_bool) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    bool ret1 = false;
    bool ret2 = false;
    L->compile_file("./jit/test_binop_and_bool.lua", {});
    L->call("test", std::tie(ret1, ret2), true, false);
    ASSERT_FALSE(ret1);
    ASSERT_TRUE(ret2);

    ret1 = false;
    ret2 = false;
    L->compile_file("./jit/test_binop_and_bool.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2), true, false);
    ASSERT_FALSE(ret1);
    ASSERT_TRUE(ret2);
}

TEST(jitter, test_binop_and_or) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int ret1 = 0;
    float ret2 = 0;
    L->compile_file("./jit/test_binop_and_or.lua", {});
    L->call("test", std::tie(ret1, ret2), 1, 2, 3, nullptr);
    ASSERT_EQ(ret1, 3);
    ASSERT_EQ(ret2, 4);

    ret1 = 0;
    ret2 = 0;
    L->compile_file("./jit/test_binop_and_or.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2), 1, 2, 3, nullptr);
    ASSERT_EQ(ret1, 3);
    ASSERT_EQ(ret2, 4);
}

TEST(jitter, test_const_binop_and) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int ret1 = 0;
    cvar ret2 = {};
    auto v = (var &) ret2;
    L->compile_file("./jit/test_const_binop_and.lua", {});
    L->call("test", std::tie(ret1, ret2));
    ASSERT_EQ(ret1, 3);
    ASSERT_EQ(v.type(), var_type::VAR_BOOL);
    ASSERT_EQ(v.get_bool(), false);

    ret1 = 0;
    ret2 = {};
    L->compile_file("./jit/test_const_binop_and.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2));
    ASSERT_EQ(ret1, 3);
    ASSERT_EQ(v.type(), var_type::VAR_BOOL);
    ASSERT_EQ(v.get_bool(), false);
}

TEST(jitter, test_binop_or) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int ret1 = 0;
    int ret2 = 0;
    L->compile_file("./jit/test_binop_or.lua", {});
    L->call("test", std::tie(ret1, ret2), 1, 1.2, nullptr, "10");
    ASSERT_EQ(ret1, 3);
    ASSERT_EQ(ret2, 9);

    ret1 = 0;
    ret2 = 0;
    L->compile_file("./jit/test_binop_or.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2), 1, 1.2, nullptr, "10");
    ASSERT_EQ(ret1, 3);
    ASSERT_EQ(ret2, 9);
}

TEST(jitter, test_const_binop_or) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int ret1 = 0;
    float ret2 = 0;
    L->compile_file("./jit/test_const_binop_or.lua", {});
    L->call("test", std::tie(ret1, ret2));
    ASSERT_EQ(ret1, 21);
    ASSERT_NEAR(ret2, 2.2, 0.001);

    ret1 = 0;
    ret2 = 0;
    L->compile_file("./jit/test_const_binop_or.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2));
    ASSERT_EQ(ret1, 21);
    ASSERT_NEAR(ret2, 2.2, 0.001);
}

TEST(jitter, test_unop_minus) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    float ret = 0;
    L->compile_file("./jit/test_unop_minus.lua", {});
    L->call("test", std::tie(ret), 2, "2.2");
    ASSERT_NEAR(ret, -3.4, 0.001);

    ret = 0;
    L->compile_file("./jit/test_unop_minus.lua", {.debug_mode = false});
    L->call("test", std::tie(ret), 2, "2.2");
    ASSERT_NEAR(ret, -3.4, 0.001);
}

TEST(jitter, test_const_unop_minus) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int ret = 0;
    L->compile_file("./jit/test_const_unop_minus.lua", {});
    L->call("test", std::tie(ret));
    ASSERT_EQ(ret, 24);

    ret = 0;
    L->compile_file("./jit/test_const_unop_minus.lua", {.debug_mode = false});
    L->call("test", std::tie(ret));
    ASSERT_EQ(ret, 24);
}

TEST(jitter, test_unop_not) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    bool ret1 = false;
    bool ret2 = false;
    L->compile_file("./jit/test_unop_not.lua", {});
    L->call("test", std::tie(ret1, ret2), 2, nullptr);
    ASSERT_FALSE(ret1);
    ASSERT_TRUE(ret2);

    ret1 = false;
    ret2 = false;
    L->compile_file("./jit/test_unop_not.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2), 2, nullptr);
    ASSERT_FALSE(ret1);
    ASSERT_TRUE(ret2);
}

TEST(jitter, test_const_unop_not) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    bool ret1 = false;
    bool ret2 = false;
    L->compile_file("./jit/test_const_unop_not.lua", {});
    L->call("test", std::tie(ret1, ret2));
    ASSERT_FALSE(ret1);
    ASSERT_TRUE(ret2);

    ret1 = false;
    ret2 = false;
    L->compile_file("./jit/test_const_unop_not.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2));
    ASSERT_FALSE(ret1);
    ASSERT_TRUE(ret2);
}

TEST(jitter, test_unop_len) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int ret1 = 0;
    int ret2 = 0;
    L->compile_file("./jit/test_unop_len.lua", {});
    L->call("test", std::tie(ret1, ret2), "abc", "123");
    ASSERT_EQ(ret1, 3);
    ASSERT_EQ(ret2, 3);

    ret1 = 0;
    ret2 = 0;
    L->compile_file("./jit/test_unop_len.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2), "abc", "123");
    ASSERT_EQ(ret1, 3);
    ASSERT_EQ(ret2, 3);
}

TEST(jitter, test_const_unop_len) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int ret1 = 0;
    int ret2 = 0;
    L->compile_file("./jit/test_const_unop_len.lua", {});
    L->call("test", std::tie(ret1, ret2));
    ASSERT_EQ(ret1, 2);
    ASSERT_EQ(ret2, 3);

    ret1 = 0;
    ret2 = 0;
    L->compile_file("./jit/test_const_unop_len.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2));
    ASSERT_EQ(ret1, 2);
    ASSERT_EQ(ret2, 3);
}

TEST(jitter, test_unop_bitnot) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int ret1 = 0;
    int ret2 = 0;
    L->compile_file("./jit/test_unop_bitnot.lua", {});
    L->call("test", std::tie(ret1, ret2), 123, -123);
    ASSERT_EQ(ret1, -124);
    ASSERT_EQ(ret2, 122);

    ret1 = 0;
    ret2 = 0;
    L->compile_file("./jit/test_unop_bitnot.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2), 123, -123);
    ASSERT_EQ(ret1, -124);
    ASSERT_EQ(ret2, 122);
}

TEST(jitter, test_const_unop_bitnot) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int ret1 = 0;
    int ret2 = 0;
    L->compile_file("./jit/test_const_unop_bitnot.lua", {});
    L->call("test", std::tie(ret1, ret2));
    ASSERT_EQ(ret1, -124);
    ASSERT_EQ(ret2, 122);

    ret1 = 0;
    ret2 = 0;
    L->compile_file("./jit/test_const_unop_bitnot.lua", {.debug_mode = false});
    L->call("test", std::tie(ret1, ret2));
    ASSERT_EQ(ret1, -124);
    ASSERT_EQ(ret2, 122);
}

TEST(jitter, test_local_func_call) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    bool ret = false;
    L->compile_file("./jit/test_local_func_call.lua", {});
    L->call("test", std::tie(ret), 2, 1);
    ASSERT_TRUE(ret);
    L->call("test", std::tie(ret), 1, 2);
    ASSERT_FALSE(ret);

    ret = false;
    L->compile_file("./jit/test_local_func_call.lua", {.debug_mode = false});
    L->call("test", std::tie(ret), 2, 1);
    ASSERT_TRUE(ret);
    L->call("test", std::tie(ret), 1, 2);
    ASSERT_FALSE(ret);
}

TEST(jitter, test_global_func_call) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    bool ret = false;
    L->compile_file("./jit/test_global_func_call.lua", {});
    L->call("test", std::tie(ret), 2, 1);
    ASSERT_TRUE(ret);

    ret = false;
    L->compile_file("./jit/test_global_func_call.lua", {.debug_mode = false});
    L->call("test", std::tie(ret), 2, 1);
    ASSERT_TRUE(ret);
}

TEST(jitter, test_assign_table_var) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int a = 0;
    L->compile_file("./jit/test_assign_table_var.lua", {});
    L->call("test", std::tie(a), "test", 1);
    ASSERT_EQ(a, 1);

    a = 0;
    L->compile_file("./jit/test_assign_simple_var.lua", {.debug_mode = false});
    L->call("test", std::tie(a), "test", 1);
    ASSERT_EQ(a, 1);
}

TEST(jitter, test_do_block) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int a = 0;
    std::string b;
    L->compile_file("./jit/test_do_block.lua", {});
    L->call("test", std::tie(a, b), true, 1.1);
    ASSERT_EQ(a, 1);
    ASSERT_EQ(b, "2");

    a = 0;
    b.clear();
    L->compile_file("./jit/test_do_block.lua", {.debug_mode = false});
    L->call("test", std::tie(a, b), true, 1.1);
    ASSERT_EQ(a, 1);
    ASSERT_EQ(b, "2");
}

TEST(jitter, test_while) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int a = 0;
    std::string b;
    L->compile_file("./jit/test_while.lua", {});
    L->call("test", std::tie(a, b), 1, "a");
    ASSERT_EQ(a, 3);
    ASSERT_EQ(b, "a22");

    a = 0;
    b.clear();
    L->compile_file("./jit/test_while.lua", {.debug_mode = false});
    L->call("test", std::tie(a, b), 1, "a");
    ASSERT_EQ(a, 3);
    ASSERT_EQ(b, "a22");
}

TEST(jitter, test_repeat) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int a = 0;
    std::string b;
    L->compile_file("./jit/test_repeat.lua", {});
    L->call("test", std::tie(a, b), 1, "a");
    ASSERT_EQ(a, 3);
    ASSERT_EQ(b, "a22");

    a = 0;
    b.clear();
    L->compile_file("./jit/test_repeat.lua", {.debug_mode = false});
    L->call("test", std::tie(a, b), 1, "a");
    ASSERT_EQ(a, 3);
    ASSERT_EQ(b, "a22");
}

TEST(jitter, test_while_double) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int a = 0;
    L->compile_file("./jit/test_while_double.lua", {});
    L->call("test", std::tie(a), 3, 4);
    ASSERT_EQ(a, 18);

    a = 0;
    L->compile_file("./jit/test_while_double.lua", {.debug_mode = false});
    L->call("test", std::tie(a), 3, 4);
    ASSERT_EQ(a, 18);
}

TEST(jitter, test_repeat_double) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int a = 0;
    L->compile_file("./jit/test_repeat_double.lua", {});
    L->call("test", std::tie(a), 4, 3);
    ASSERT_EQ(a, 18);

    a = 0;
    L->compile_file("./jit/test_repeat_double.lua", {.debug_mode = false});
    L->call("test", std::tie(a), 4, 3);
    ASSERT_EQ(a, 18);
}

TEST(jitter, test_if) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int a = 0;
    L->compile_file("./jit/test_if.lua", {});
    L->call("test", std::tie(a), 5, 6);
    ASSERT_EQ(a, 4);
    L->call("test", std::tie(a), 5, 2);
    ASSERT_EQ(a, 3);
    L->call("test", std::tie(a), 3, 2);
    ASSERT_EQ(a, 2);
    L->call("test", std::tie(a), 0, 2);
    ASSERT_EQ(a, 0);

    a = 0;
    L->compile_file("./jit/test_if.lua", {.debug_mode = false});
    L->call("test", std::tie(a), 5, 6);
    ASSERT_EQ(a, 4);
    L->call("test", std::tie(a), 5, 2);
    ASSERT_EQ(a, 3);
    L->call("test", std::tie(a), 3, 2);
    ASSERT_EQ(a, 2);
    L->call("test", std::tie(a), 0, 2);
    ASSERT_EQ(a, 0);
}

TEST(jitter, test_if_simple) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int a = 0;
    L->compile_file("./jit/test_if_simple.lua", {});
    L->call("test", std::tie(a), 5, 6);
    ASSERT_EQ(a, 4);
    L->call("test", std::tie(a), 0, 2);
    ASSERT_EQ(a, 0);

    a = 0;
    L->compile_file("./jit/test_if_simple.lua", {.debug_mode = false});
    L->call("test", std::tie(a), 5, 6);
    ASSERT_EQ(a, 4);
    L->call("test", std::tie(a), 0, 2);
    ASSERT_EQ(a, 0);
}

TEST(jitter, test_if_elseif) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int a = 0;
    L->compile_file("./jit/test_if_elseif.lua", {});
    L->call("test", std::tie(a), 5, 5);
    ASSERT_EQ(a, 4);
    L->call("test", std::tie(a), 4, 3);
    ASSERT_EQ(a, 3);
    L->call("test", std::tie(a), 3, 2);
    ASSERT_EQ(a, 2);
    L->call("test", std::tie(a), 2, 1);
    ASSERT_EQ(a, 1);
    L->call("test", std::tie(a), 0, 0);
    ASSERT_EQ(a, 0);

    a = 0;
    L->compile_file("./jit/test_if_elseif.lua", {.debug_mode = false});
    L->call("test", std::tie(a), 5, 5);
    ASSERT_EQ(a, 4);
    L->call("test", std::tie(a), 4, 3);
    ASSERT_EQ(a, 3);
    L->call("test", std::tie(a), 3, 2);
    ASSERT_EQ(a, 2);
    L->call("test", std::tie(a), 2, 1);
    ASSERT_EQ(a, 1);
    L->call("test", std::tie(a), 0, 0);
    ASSERT_EQ(a, 0);
}

TEST(jitter, test_if_elseif_normal) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int a = 0;
    L->compile_file("./jit/test_if_elseif_normal.lua", {});
    L->call("test", std::tie(a), 5, 5);
    ASSERT_EQ(a, 4);
    L->call("test", std::tie(a), 4, 3);
    ASSERT_EQ(a, 3);
    L->call("test", std::tie(a), 3, 2);
    ASSERT_EQ(a, 2);
    L->call("test", std::tie(a), 2, 1);
    ASSERT_EQ(a, 1);
    L->call("test", std::tie(a), 0, 0);
    ASSERT_EQ(a, 0);

    a = 0;
    L->compile_file("./jit/test_if_elseif_normal.lua", {.debug_mode = false});
    L->call("test", std::tie(a), 5, 5);
    ASSERT_EQ(a, 4);
    L->call("test", std::tie(a), 4, 3);
    ASSERT_EQ(a, 3);
    L->call("test", std::tie(a), 3, 2);
    ASSERT_EQ(a, 2);
    L->call("test", std::tie(a), 2, 1);
    ASSERT_EQ(a, 1);
    L->call("test", std::tie(a), 0, 0);
    ASSERT_EQ(a, 0);
}

TEST(jitter, test_if_elseif_return) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int a = 0;
    L->compile_file("./jit/test_if_elseif_return.lua", {});
    L->call("test", std::tie(a), 5, 5);
    ASSERT_EQ(a, 4);
    L->call("test", std::tie(a), 4, 3);
    ASSERT_EQ(a, 3);
    L->call("test", std::tie(a), 3, 2);
    ASSERT_EQ(a, 2);
    L->call("test", std::tie(a), 2, 1);
    ASSERT_EQ(a, 1);
    L->call("test", std::tie(a), 0, 0);
    ASSERT_EQ(a, 0);

    a = 0;
    L->compile_file("./jit/test_if_elseif_return.lua", {.debug_mode = false});
    L->call("test", std::tie(a), 5, 5);
    ASSERT_EQ(a, 4);
    L->call("test", std::tie(a), 4, 3);
    ASSERT_EQ(a, 3);
    L->call("test", std::tie(a), 3, 2);
    ASSERT_EQ(a, 2);
    L->call("test", std::tie(a), 2, 1);
    ASSERT_EQ(a, 1);
    L->call("test", std::tie(a), 0, 0);
    ASSERT_EQ(a, 0);
}

TEST(jitter, test_if_else) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int a = 0;
    L->compile_file("./jit/test_if_else.lua", {});
    L->call("test", std::tie(a), 5, 6);
    ASSERT_EQ(a, 4);
    L->call("test", std::tie(a), 0, 2);
    ASSERT_EQ(a, 0);

    a = 0;
    L->compile_file("./jit/test_if_else.lua", {.debug_mode = false});
    L->call("test", std::tie(a), 5, 6);
    ASSERT_EQ(a, 4);
    L->call("test", std::tie(a), 0, 2);
    ASSERT_EQ(a, 0);
}

TEST(jitter, test_while_return) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int a = 0;
    L->compile_file("./jit/test_while_return.lua", {});
    L->call("test", std::tie(a), 4, 4);
    ASSERT_EQ(a, 4);
    L->call("test", std::tie(a), 2, 4);
    ASSERT_EQ(a, 2);

    a = 0;
    L->compile_file("./jit/test_while_return.lua", {.debug_mode = false});
    L->call("test", std::tie(a), 4, 4);
    ASSERT_EQ(a, 4);
    L->call("test", std::tie(a), 2, 4);
    ASSERT_EQ(a, 2);
}

TEST(jitter, test_repeat_return) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int a = 0;
    L->compile_file("./jit/test_repeat_return.lua", {});
    L->call("test", std::tie(a), 4, 4);
    ASSERT_EQ(a, 4);

    a = 0;
    L->compile_file("./jit/test_repeat_return.lua", {.debug_mode = false});
    L->call("test", std::tie(a), 4, 4);
    ASSERT_EQ(a, 4);
}

TEST(jitter, test_while_break) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int a = 0;
    L->compile_file("./jit/test_while_break.lua", {});
    L->call("test", std::tie(a), 4, 4);
    ASSERT_EQ(a, 11);

    a = 0;
    L->compile_file("./jit/test_while_break.lua", {.debug_mode = false});
    L->call("test", std::tie(a), 4, 4);
    ASSERT_EQ(a, 11);
}

TEST(jitter, test_repeat_break) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int a = 0;
    L->compile_file("./jit/test_repeat_break.lua", {});
    L->call("test", std::tie(a), 4, 4);
    ASSERT_EQ(a, 11);

    a = 0;
    L->compile_file("./jit/test_repeat_break.lua", {.debug_mode = false});
    L->call("test", std::tie(a), 4, 4);
    ASSERT_EQ(a, 11);
}

TEST(jitter, test_while_if_return) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int a = 0;
    L->compile_file("./jit/test_while_if_return.lua", {});
    L->call("test", std::tie(a), 3, 4);
    ASSERT_EQ(a, 3);
    L->call("test", std::tie(a), 4, 4);
    ASSERT_EQ(a, 4);

    a = 0;
    L->compile_file("./jit/test_while_if_return.lua", {.debug_mode = false});
    L->call("test", std::tie(a), 3, 4);
    ASSERT_EQ(a, 3);
    L->call("test", std::tie(a), 4, 4);
    ASSERT_EQ(a, 4);
}

TEST(jitter, test_repeat_if_return) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int a = 0;
    L->compile_file("./jit/test_repeat_if_return.lua", {});
    L->call("test", std::tie(a), 3, 4);
    ASSERT_EQ(a, 3);
    L->call("test", std::tie(a), 4, 4);
    ASSERT_EQ(a, 4);

    a = 0;
    L->compile_file("./jit/test_repeat_if_return.lua", {.debug_mode = false});
    L->call("test", std::tie(a), 3, 4);
    ASSERT_EQ(a, 3);
    L->call("test", std::tie(a), 4, 4);
    ASSERT_EQ(a, 4);
}

TEST(jitter, test_for_loop) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int a = 0;
    L->compile_file("./jit/test_for_loop.lua", {});
    L->call("test", std::tie(a), 3, 5);
    ASSERT_EQ(a, 8);

    a = 0;
    L->compile_file("./jit/test_for_loop.lua", {.debug_mode = false});
    L->call("test", std::tie(a), 3, 5);
    ASSERT_EQ(a, 8);
}

TEST(jitter, test_for_loop_default) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int a = 0;
    L->compile_file("./jit/test_for_loop_default.lua", {});
    L->call("test", std::tie(a), 3, 5);
    ASSERT_EQ(a, 12);

    a = 0;
    L->compile_file("./jit/test_for_loop_default.lua", {.debug_mode = false});
    L->call("test", std::tie(a), 3, 5);
    ASSERT_EQ(a, 12);
}

TEST(jitter, test_for_loop_return) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int a = 0;
    L->compile_file("./jit/test_for_loop_return.lua", {});
    L->call("test", std::tie(a), 3, 3);
    ASSERT_EQ(a, 3);
    L->call("test", std::tie(a), 4, 3);
    ASSERT_EQ(a, 0);

    a = 0;
    L->compile_file("./jit/test_for_loop_return.lua", {.debug_mode = false});
    L->call("test", std::tie(a), 3, 3);
    ASSERT_EQ(a, 3);
    L->call("test", std::tie(a), 4, 3);
    ASSERT_EQ(a, 0);
}

TEST(jitter, test_while_just_break) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int a = 0;
    L->compile_file("./jit/test_while_just_break.lua", {});
    L->call("test", std::tie(a), 3, 4);
    ASSERT_EQ(a, 4);
    L->call("test", std::tie(a), 5, 4);
    ASSERT_EQ(a, 5);

    a = 0;
    L->compile_file("./jit/test_while_just_break.lua", {.debug_mode = false});
    L->call("test", std::tie(a), 3, 4);
    ASSERT_EQ(a, 4);
    L->call("test", std::tie(a), 5, 4);
    ASSERT_EQ(a, 5);
}

TEST(jitter, test_repeat_just_break) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int a = 0;
    L->compile_file("./jit/test_repeat_just_break.lua", {});
    L->call("test", std::tie(a), 3, 4);
    ASSERT_EQ(a, 4);
    L->call("test", std::tie(a), 5, 4);
    ASSERT_EQ(a, 4);

    a = 0;
    L->compile_file("./jit/test_repeat_just_break.lua", {.debug_mode = false});
    L->call("test", std::tie(a), 3, 4);
    ASSERT_EQ(a, 4);
    L->call("test", std::tie(a), 5, 4);
    ASSERT_EQ(a, 4);
}

TEST(jitter, test_for_loop_double) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int a = 0;
    L->compile_file("./jit/test_for_loop_double.lua", {});
    L->call("test", std::tie(a), 3, 5);
    ASSERT_EQ(a, 21);

    a = 0;
    L->compile_file("./jit/test_for_loop_double.lua", {.debug_mode = false});
    L->call("test", std::tie(a), 3, 5);
    ASSERT_EQ(a, 21);
}

TEST(jitter, test_for_loop_break) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int a = 0;
    L->compile_file("./jit/test_for_loop_break.lua", {});
    L->call("test", std::tie(a), 3, 5);
    ASSERT_EQ(a, 4);

    a = 0;
    L->compile_file("./jit/test_for_loop_break.lua", {.debug_mode = false});
    L->call("test", std::tie(a), 3, 5);
    ASSERT_EQ(a, 4);
}

TEST(jitter, test_for_loop_if_return) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int a = 0;
    L->compile_file("./jit/test_for_loop_if_return.lua", {});
    L->call("test", std::tie(a), 3, 5);
    ASSERT_EQ(a, 3);
    L->call("test", std::tie(a), 6, 10);
    ASSERT_EQ(a, 10);

    a = 0;
    L->compile_file("./jit/test_for_loop_if_return.lua", {.debug_mode = false});
    L->call("test", std::tie(a), 3, 5);
    ASSERT_EQ(a, 3);
    L->call("test", std::tie(a), 6, 10);
    ASSERT_EQ(a, 10);
}

TEST(jitter, test_for_loop_just_break) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int a = 0;
    L->compile_file("./jit/test_for_loop_just_break.lua", {});
    L->call("test", std::tie(a), 3, 5);
    ASSERT_EQ(a, 5);
    L->call("test", std::tie(a), 5, 3);
    ASSERT_EQ(a, 5);

    a = 0;
    L->compile_file("./jit/test_for_loop_just_break.lua", {.debug_mode = false});
    L->call("test", std::tie(a), 3, 5);
    ASSERT_EQ(a, 5);
    L->call("test", std::tie(a), 5, 3);
    ASSERT_EQ(a, 5);
}

TEST(jitter, test_for_in) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int a = 0;
    L->compile_file("./jit/test_for_in.lua", {});
    L->call("test", std::tie(a), 3, 4);
    ASSERT_EQ(a, 32);

    a = 0;
    L->compile_file("./jit/test_for_in.lua", {.debug_mode = false});
    L->call("test", std::tie(a), 3, 4);
    ASSERT_EQ(a, 32);
}

TEST(jitter, test_for_in_double) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int a = 0;
    L->compile_file("./jit/test_for_in_double.lua", {});
    L->call("test", std::tie(a), 3, 4);
    ASSERT_EQ(a, 320);

    a = 0;
    L->compile_file("./jit/test_for_in_double.lua", {.debug_mode = false});
    L->call("test", std::tie(a), 3, 4);
    ASSERT_EQ(a, 320);
}

TEST(jitter, test_for_in_break) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int a = 0;
    L->compile_file("./jit/test_for_in_break.lua", {});
    L->call("test", std::tie(a), 3, 4);
    ASSERT_EQ(a, 30);

    a = 0;
    L->compile_file("./jit/test_for_in_break.lua", {.debug_mode = false});
    L->call("test", std::tie(a), 3, 4);
    ASSERT_EQ(a, 30);
}

TEST(jitter, test_for_in_if_return) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int a = 0;
    L->compile_file("./jit/test_for_in_if_return.lua", {});
    L->call("test", std::tie(a), 3, 4);
    ASSERT_EQ(a, 3);
    L->call("test", std::tie(a), 5, 4);
    ASSERT_EQ(a, 4);

    a = 0;
    L->compile_file("./jit/test_for_in_if_return.lua", {.debug_mode = false});
    L->call("test", std::tie(a), 3, 4);
    ASSERT_EQ(a, 3);
    L->call("test", std::tie(a), 5, 4);
    ASSERT_EQ(a, 4);
}

TEST(jitter, test_for_in_just_break) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int a = 0;
    L->compile_file("./jit/test_for_in_just_break.lua", {});
    L->call("test", std::tie(a), 3, 4);
    ASSERT_EQ(a, 3);

    a = 0;
    L->compile_file("./jit/test_for_in_just_break.lua", {.debug_mode = false});
    L->call("test", std::tie(a), 3, 4);
    ASSERT_EQ(a, 3);
}

TEST(jitter, test_for_in_return) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int a = 0;
    L->compile_file("./jit/test_for_in_return.lua", {});
    L->call("test", std::tie(a), 3, 4);
    ASSERT_EQ(a, 3);

    a = 0;
    L->compile_file("./jit/test_for_in_return.lua", {.debug_mode = false});
    L->call("test", std::tie(a), 3, 4);
    ASSERT_EQ(a, 3);
}

TEST(jitter, test_for_in_return_fallback) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int a = 0;
    L->compile_file("./jit/test_for_in_return_fallback.lua", {});
    L->call("test", std::tie(a), 3, 4);
    ASSERT_EQ(a, 0);

    a = 0;
    L->compile_file("./jit/test_for_in_return_fallback.lua", {.debug_mode = false});
    L->call("test", std::tie(a), 3, 4);
    ASSERT_EQ(a, 0);
}

TEST(jitter, test_local_func_call_table_construct) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    bool ret = false;
    L->compile_file("./jit/test_local_func_call_table_construct.lua", {});
    L->call("test", std::tie(ret), 2, 1);
    ASSERT_TRUE(ret);
    L->call("test", std::tie(ret), 1, 2);
    ASSERT_FALSE(ret);

    ret = false;
    L->compile_file("./jit/test_local_func_call_table_construct.lua", {.debug_mode = false});
    L->call("test", std::tie(ret), 2, 1);
    ASSERT_TRUE(ret);
    L->call("test", std::tie(ret), 1, 2);
    ASSERT_FALSE(ret);
}

TEST(jitter, test_local_func_call_string) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    std::string ret;
    L->compile_file("./jit/test_local_func_call_string.lua", {});
    L->call("test", std::tie(ret), 2, 1);
    ASSERT_EQ(ret, "test_test");

    ret.clear();
    L->compile_file("./jit/test_local_func_call_string.lua", {.debug_mode = false});
    L->call("test", std::tie(ret), 2, 1);
    ASSERT_EQ(ret, "test_test");
}

TEST(jitter, test_var_func_call) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    bool ret = false;
    L->compile_file("./jit/test_var_func_call.lua", {});
    L->call("test", std::tie(ret), 2, 2);
    ASSERT_TRUE(ret);
    L->call("test", std::tie(ret), 1, 2);
    ASSERT_FALSE(ret);

    ret = false;
    L->compile_file("./jit/test_var_func_call.lua", {.debug_mode = false});
    L->call("test", std::tie(ret), 2, 2);
    ASSERT_TRUE(ret);
    L->call("test", std::tie(ret), 1, 2);
    ASSERT_FALSE(ret);
}

TEST(jitter, test_table_var_func_call) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    bool ret = false;
    L->compile_file("./jit/test_table_var_func_call.lua", {});
    L->call("test", std::tie(ret), 2, 2);
    ASSERT_TRUE(ret);
    L->call("test", std::tie(ret), 1, 2);
    ASSERT_FALSE(ret);

    ret = false;
    L->compile_file("./jit/test_table_var_func_call.lua", {.debug_mode = false});
    L->call("test", std::tie(ret), 2, 2);
    ASSERT_TRUE(ret);
    L->call("test", std::tie(ret), 1, 2);
    ASSERT_FALSE(ret);
}

TEST(jitter, test_empty_func_call) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int ret = 0;
    L->compile_file("./jit/test_empty_func_call.lua", {});
    L->call("test", std::tie(ret), 2, 2);
    ASSERT_EQ(ret, 1);

    ret = 0;
    L->compile_file("./jit/test_empty_func_call.lua", {.debug_mode = false});
    L->call("test", std::tie(ret), 2, 2);
    ASSERT_EQ(ret, 1);
}

TEST(jitter, test_table_get_set) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);

    int ret = 0;
    L->compile_file("./jit/test_table_get_set.lua", {});
    L->call("test", std::tie(ret), 1, 2);
    ASSERT_EQ(ret, 3);

    ret = 0;
    L->compile_file("./jit/test_table_get_set.lua", {.debug_mode = false});
    L->call("test", std::tie(ret), 1, 2);
    ASSERT_EQ(ret, 3);
}
