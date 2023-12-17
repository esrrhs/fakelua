#pragma once

#include "compile/syntax_tree.h"
#include "fakelua.h"
#include "gcc_jit_handle.h"
#include "var/var.h"

namespace fakelua {

// wrapper of gcc_jit_function call
class vm_function {
public:
    vm_function(gcc_jit_handle_ptr gcc_jit_handle, void *gcc_jit_func, int arg_count, bool is_variadic)
        : gcc_jit_handle_(gcc_jit_handle), gcc_jit_func_(gcc_jit_func), arg_count_(arg_count), is_variadic_(is_variadic) {
    }

    ~vm_function() = default;

    void *get_addr() {
        return gcc_jit_func_;
    }

    gcc_jit_handle_ptr get_gcc_jit_handle() {
        return gcc_jit_handle_;
    }

    int get_arg_count() {
        return arg_count_;
    }

    bool is_variadic() {
        return is_variadic_;
    }

private:
    gcc_jit_handle_ptr gcc_jit_handle_;
    void *gcc_jit_func_ = nullptr;
    int arg_count_ = 0;
    bool is_variadic_ = false;
};

typedef std::shared_ptr<vm_function> vm_function_ptr;

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

private:
    std::unordered_map<std::string, vm_function_ptr> vm_functions_;
};


extern "C" __attribute__((used)) var *new_const_var_nil(gcc_jit_handle *h);

extern "C" __attribute__((used)) var *new_const_var_false(gcc_jit_handle *h);

extern "C" __attribute__((used)) var *new_const_var_true(gcc_jit_handle *h);

extern "C" __attribute__((used)) var *new_const_var_int(gcc_jit_handle *h, int64_t val);

extern "C" __attribute__((used)) var *new_const_var_float(gcc_jit_handle *h, double val);

extern "C" __attribute__((used)) var *new_const_var_string(gcc_jit_handle *h, const char *val, int len);

extern "C" __attribute__((used)) var *new_var_nil(fakelua_state *s);

extern "C" __attribute__((used)) var *new_var_false(fakelua_state *s);

extern "C" __attribute__((used)) var *new_var_true(fakelua_state *s);

extern "C" __attribute__((used)) var *new_var_int(fakelua_state *s, int64_t val);

extern "C" __attribute__((used)) var *new_var_float(fakelua_state *s, double val);

extern "C" __attribute__((used)) var *new_var_string(fakelua_state *s, const char *val, int len);

extern "C" __attribute__((used)) var *new_var_wrap(fakelua_state *s, var *val);

extern "C" __attribute__((used)) var *wrap_return_var(fakelua_state *s, int n, ...);

}// namespace fakelua
