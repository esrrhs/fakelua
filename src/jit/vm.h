#pragma once

#include "compile/syntax_tree.h"
#include "fakelua.h"
#include "gcc_jit_handle.h"
#include "var/var.h"
#include "vm_function.h"

namespace fakelua {

// store all compiled jit functions and some other running used data.
// every state has one vm.
class vm {
public:
    vm() = default;

    ~vm() = default;

    // register function
    void register_function(const std::string &name, vm_function_ptr func) {
        vm_functions_[name] = func;
    }

    // get function
    vm_function_ptr get_function(const std::string &name) {
        auto iter = vm_functions_.find(name);
        if (iter == vm_functions_.end()) {
            return nullptr;
        }
        return iter->second;
    }

    // get all function
    const std::unordered_map<std::string, vm_function_ptr> &get_functions() {
        return vm_functions_;
    }

    std::string alloc_special_function_name() {
        return std::format("__fakelua_special_function_{}__", special_function_name_++);
    }

private:
    std::unordered_map<std::string, vm_function_ptr> vm_functions_;
    uint64_t special_function_name_ = 0;
};

extern "C" __attribute__((used)) var *new_const_var_nil(gcc_jit_handle *h);

extern "C" __attribute__((used)) var *new_const_var_false(gcc_jit_handle *h);

extern "C" __attribute__((used)) var *new_const_var_true(gcc_jit_handle *h);

extern "C" __attribute__((used)) var *new_const_var_int(gcc_jit_handle *h, int64_t val);

extern "C" __attribute__((used)) var *new_const_var_float(gcc_jit_handle *h, double val);

extern "C" __attribute__((used)) var *new_const_var_string(gcc_jit_handle *h, const char *val, int len);

extern "C" __attribute__((used)) var *new_const_var_table(gcc_jit_handle *h, int n, ...);

extern "C" __attribute__((used)) var *binop_const_plus(gcc_jit_handle *h, var *l, var *r);

extern "C" __attribute__((used)) var *binop_const_minus(gcc_jit_handle *h, var *l, var *r);

extern "C" __attribute__((used)) var *binop_const_star(gcc_jit_handle *h, var *l, var *r);

extern "C" __attribute__((used)) var *binop_const_slash(gcc_jit_handle *h, var *l, var *r);

extern "C" __attribute__((used)) var *binop_const_double_slash(gcc_jit_handle *h, var *l, var *r);

extern "C" __attribute__((used)) var *binop_const_pow(gcc_jit_handle *h, var *l, var *r);

extern "C" __attribute__((used)) var *binop_const_mod(gcc_jit_handle *h, var *l, var *r);

extern "C" __attribute__((used)) var *binop_const_bitand(gcc_jit_handle *h, var *l, var *r);

extern "C" __attribute__((used)) var *binop_const_xor(gcc_jit_handle *h, var *l, var *r);

extern "C" __attribute__((used)) var *binop_const_bitor(gcc_jit_handle *h, var *l, var *r);

extern "C" __attribute__((used)) var *binop_const_right_shift(gcc_jit_handle *h, var *l, var *r);

extern "C" __attribute__((used)) var *binop_const_left_shift(gcc_jit_handle *h, var *l, var *r);

extern "C" __attribute__((used)) var *binop_const_concat(gcc_jit_handle *h, var *l, var *r);

extern "C" __attribute__((used)) var *binop_const_less(gcc_jit_handle *h, var *l, var *r);

extern "C" __attribute__((used)) var *binop_const_less_equal(gcc_jit_handle *h, var *l, var *r);

extern "C" __attribute__((used)) var *binop_const_more(gcc_jit_handle *h, var *l, var *r);

extern "C" __attribute__((used)) var *binop_const_more_equal(gcc_jit_handle *h, var *l, var *r);

extern "C" __attribute__((used)) var *binop_const_equal(gcc_jit_handle *h, var *l, var *r);

extern "C" __attribute__((used)) var *binop_const_not_equal(gcc_jit_handle *h, var *l, var *r);

extern "C" __attribute__((used)) bool test_const_var(gcc_jit_handle *h, var *v);

extern "C" __attribute__((used)) bool test_const_not_var(gcc_jit_handle *h, var *v);

extern "C" __attribute__((used)) var *unop_const_minus(gcc_jit_handle *h, var *r);

extern "C" __attribute__((used)) var *unop_const_not(gcc_jit_handle *h, var *r);

extern "C" __attribute__((used)) var *unop_const_number_sign(gcc_jit_handle *h, var *r);

extern "C" __attribute__((used)) var *unop_const_bitnot(gcc_jit_handle *h, var *r);

///////////////////////////////////////////////////////////////////////////////////

extern "C" __attribute__((used)) var *new_var_nil(fakelua_state *s);

extern "C" __attribute__((used)) var *new_var_false(fakelua_state *s);

extern "C" __attribute__((used)) var *new_var_true(fakelua_state *s);

extern "C" __attribute__((used)) var *new_var_int(fakelua_state *s, int64_t val);

extern "C" __attribute__((used)) var *new_var_float(fakelua_state *s, double val);

extern "C" __attribute__((used)) var *new_var_string(fakelua_state *s, const char *val, int len);

extern "C" __attribute__((used)) var *new_var_table(fakelua_state *s, int n, ...);

extern "C" __attribute__((used)) var *wrap_return_var(fakelua_state *s, int n, ...);

extern "C" __attribute__((used)) void assign_var(fakelua_state *s, int left_n, int right_n, ...);

extern "C" __attribute__((used)) var *binop_plus(fakelua_state *s, var *l, var *r);

extern "C" __attribute__((used)) var *binop_plus(fakelua_state *s, var *l, var *r);

extern "C" __attribute__((used)) var *binop_minus(fakelua_state *s, var *l, var *r);

extern "C" __attribute__((used)) var *binop_star(fakelua_state *s, var *l, var *r);

extern "C" __attribute__((used)) var *binop_slash(fakelua_state *s, var *l, var *r);

extern "C" __attribute__((used)) var *binop_double_slash(fakelua_state *s, var *l, var *r);

extern "C" __attribute__((used)) var *binop_pow(fakelua_state *s, var *l, var *r);

extern "C" __attribute__((used)) var *binop_mod(fakelua_state *s, var *l, var *r);

extern "C" __attribute__((used)) var *binop_bitand(fakelua_state *s, var *l, var *r);

extern "C" __attribute__((used)) var *binop_xor(fakelua_state *s, var *l, var *r);

extern "C" __attribute__((used)) var *binop_bitor(fakelua_state *s, var *l, var *r);

extern "C" __attribute__((used)) var *binop_right_shift(fakelua_state *s, var *l, var *r);

extern "C" __attribute__((used)) var *binop_left_shift(fakelua_state *s, var *l, var *r);

extern "C" __attribute__((used)) var *binop_concat(fakelua_state *s, var *l, var *r);

extern "C" __attribute__((used)) var *binop_less(fakelua_state *s, var *l, var *r);

extern "C" __attribute__((used)) var *binop_less_equal(fakelua_state *s, var *l, var *r);

extern "C" __attribute__((used)) var *binop_more(fakelua_state *s, var *l, var *r);

extern "C" __attribute__((used)) var *binop_more_equal(fakelua_state *s, var *l, var *r);

extern "C" __attribute__((used)) var *binop_equal(fakelua_state *s, var *l, var *r);

extern "C" __attribute__((used)) var *binop_not_equal(fakelua_state *s, var *l, var *r);

extern "C" __attribute__((used)) bool test_var(fakelua_state *s, var *v);

extern "C" __attribute__((used)) bool test_not_var(fakelua_state *s, var *v);

extern "C" __attribute__((used)) var *unop_minus(fakelua_state *s, var *r);

extern "C" __attribute__((used)) var *unop_not(fakelua_state *s, var *r);

extern "C" __attribute__((used)) var *unop_number_sign(fakelua_state *s, var *r);

extern "C" __attribute__((used)) var *unop_bitnot(fakelua_state *s, var *r);

extern "C" __attribute__((used)) var *call_var(fakelua_state *s, var *func, int n, ...);

extern "C" __attribute__((used)) var *table_index_var(fakelua_state *s, var *table, var *key);

}// namespace fakelua
