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
    void register_function(const std::string &name, const vm_function_ptr &func) {
        vm_functions_[name] = func;
    }

    // get function
    vm_function_ptr get_function(const std::string &name) const {
        const auto iter = vm_functions_.find(name);
        if (iter == vm_functions_.end()) {
            return nullptr;
        }
        return iter->second;
    }

    // get all functions
    const std::unordered_map<std::string, vm_function_ptr> &get_functions() {
        return vm_functions_;
    }

    // alloc global name, life cycle is all through the process, so put it in vm
    std::string alloc_global_name() {
        return std::format("__fakelua_global_{}__", global_name_++);
    }

private:
    // all registered functions
    std::unordered_map<std::string, vm_function_ptr> vm_functions_;
    // global name counter
    uint64_t global_name_ = 0;
};

extern "C" __attribute__((used)) var *new_var_table(fakelua_state *s, gcc_jit_handle *h, bool is_const, int n, ...);

extern "C" __attribute__((used)) var *wrap_return_var(fakelua_state *s, gcc_jit_handle *h, bool is_const, int n, ...);

extern "C" __attribute__((used)) void assign_var(fakelua_state *s, gcc_jit_handle *h, bool is_const, int left_n, int right_n, ...);

extern "C" __attribute__((used)) var *binop_plus(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *l, var *r);

extern "C" __attribute__((used)) var *binop_plus(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *l, var *r);

extern "C" __attribute__((used)) var *binop_minus(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *l, var *r);

extern "C" __attribute__((used)) var *binop_star(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *l, var *r);

extern "C" __attribute__((used)) var *binop_slash(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *l, var *r);

extern "C" __attribute__((used)) var *binop_double_slash(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *l, var *r);

extern "C" __attribute__((used)) var *binop_pow(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *l, var *r);

extern "C" __attribute__((used)) var *binop_mod(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *l, var *r);

extern "C" __attribute__((used)) var *binop_bitand(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *l, var *r);

extern "C" __attribute__((used)) var *binop_xor(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *l, var *r);

extern "C" __attribute__((used)) var *binop_bitor(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *l, var *r);

extern "C" __attribute__((used)) var *binop_right_shift(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *l, var *r);

extern "C" __attribute__((used)) var *binop_left_shift(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *l, var *r);

extern "C" __attribute__((used)) var *binop_concat(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *l, var *r);

extern "C" __attribute__((used)) var *binop_less(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *l, var *r);

extern "C" __attribute__((used)) var *binop_less_equal(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *l, var *r);

extern "C" __attribute__((used)) var *binop_more(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *l, var *r);

extern "C" __attribute__((used)) var *binop_more_equal(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *l, var *r);

extern "C" __attribute__((used)) var *binop_equal(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *l, var *r);

extern "C" __attribute__((used)) var *binop_not_equal(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *l, var *r);

extern "C" __attribute__((used)) bool test_var(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *v);

extern "C" __attribute__((used)) bool test_not_var(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *v);

extern "C" __attribute__((used)) var *unop_minus(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *r);

extern "C" __attribute__((used)) var *unop_not(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *r);

extern "C" __attribute__((used)) var *unop_number_sign(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *r);

extern "C" __attribute__((used)) var *unop_bitnot(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *r);

extern "C" __attribute__((used)) var *call_var(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *func, var *col_key, int n, ...);

extern "C" __attribute__((used)) var *table_index_by_var(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *table, var *key);

extern "C" __attribute__((used)) var *table_index_by_name(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *table, const char *key,
                                                          int len);

extern "C" __attribute__((used)) var *table_set(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *table, var *key, var *val);

extern "C" __attribute__((used)) size_t table_size(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *table);

extern "C" __attribute__((used)) var *table_key_by_pos(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *table, size_t pos);

extern "C" __attribute__((used)) var *table_value_by_pos(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *table, size_t pos);

}// namespace fakelua
