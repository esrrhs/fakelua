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

}// namespace fakelua
