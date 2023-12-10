#pragma once

#include "compile/syntax_tree.h"
#include "fakelua.h"
#include "gcc_jit_handle.h"
#include "var/var.h"

namespace fakelua {

// wrapper of gcc_jit_function call
class vm_function {
public:
    vm_function(gcc_jit_handle_ptr gcc_jit_handle, void *gcc_jit_func) : gcc_jit_handle_(gcc_jit_handle), gcc_jit_func_(gcc_jit_func) {
    }
    
    ~vm_function() = default;

private:
    gcc_jit_handle_ptr gcc_jit_handle_;
    void *gcc_jit_func_ = nullptr;
};

typedef std::shared_ptr<vm_function> vm_function_ptr;

// store all compiled jit functions and some other running used data.
// every state has one vm.
class vm {
public:
    vm() = default;

    ~vm() = default;

    // register function
    void register_vm_function(const std::string &name, vm_function_ptr func) {
        vm_functions_[name] = func;
    }

private:
    std::unordered_map<std::string, vm_function_ptr> vm_functions_;
};

extern "C" __attribute__((used)) var *new_var_nil(fakelua_state *s);

extern "C" __attribute__((used)) var *wrap_return_var(fakelua_state *s, ...);

}// namespace fakelua
