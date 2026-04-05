#pragma once

#include "jit/tcc_handle.h"
#include <utility>

namespace fakelua {

class VmFunction {
public:
    VmFunction() = default;

    VmFunction(int arg_count, void *tcc_func_addr, TCCHandlePtr tcc_handle)
        : arg_count_(arg_count), tcc_func_addr_(tcc_func_addr), tcc_handle_(std::move(tcc_handle)) {
    }

    ~VmFunction() = default;

    [[nodiscard]] int GetArgCount() const {
        return arg_count_;
    }

    [[nodiscard]] void *GetTCCAddr() const {
        return tcc_func_addr_;
    }

    [[nodiscard]] TCCHandlePtr GetTCCHandle() const {
        return tcc_handle_;
    }

private:
    int arg_count_ = 0;
    void *tcc_func_addr_ = nullptr;
    TCCHandlePtr tcc_handle_;
};

}// namespace fakelua
