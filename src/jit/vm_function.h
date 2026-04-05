#pragma once

#include "compile/syntax_tree.h"
#include "fakelua.h"
#include "var/var.h"

namespace fakelua {

// wrapper of gcc_jit_function call
class VmFunction {
public:
    typedef void (*ContextFreeFunc)(void *);

    VmFunction(void *func_ptr, int arg_count, bool is_variadic, void *ctx = nullptr,
               ContextFreeFunc ctx_free_func = nullptr)
        : func_ptr_(func_ptr), arg_count_(arg_count), is_variadic_(is_variadic), ctx_(ctx),
          ctx_free_func_(ctx_free_func) {
    }

    ~VmFunction() {
        if (ctx_ && ctx_free_func_) {
            ctx_free_func_(ctx_);
        }
    }

    void *get_addr() const {
        return func_ptr_;
    }

    int GetArgCount() const {
        return arg_count_;
    }

    bool IsVariadic() const {
        return is_variadic_;
    }

private:
    void *func_ptr_ = nullptr;
    int arg_count_ = 0;
    bool is_variadic_ = false;
    void *ctx_ = nullptr;
    ContextFreeFunc ctx_free_func_ = nullptr;
};

typedef std::shared_ptr<VmFunction> VmFunctionPtr;

}// namespace fakelua
