#pragma once

#include "compile/syntax_tree.h"
#include "fakelua.h"
#include "var/var.h"

namespace fakelua {

// wrapper of gcc_jit_function call
class VmFunction {
public:
    VmFunction(void *gcc_jit_func, int arg_count, bool IsVariadic)
        : gcc_jit_func_(gcc_jit_func), arg_count_(arg_count), is_variadic_(IsVariadic) {
    }

    ~VmFunction() = default;

    void *get_addr() const {
        return gcc_jit_func_;
    }

    int GetArgCount() const {
        return arg_count_;
    }

    bool IsVariadic() const {
        return is_variadic_;
    }

private:
    void *gcc_jit_func_ = nullptr;
    int arg_count_ = 0;
    bool is_variadic_ = false;
};

typedef std::shared_ptr<VmFunction> VmFunctionPtr;

}// namespace fakelua
