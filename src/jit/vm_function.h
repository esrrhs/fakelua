#pragma once

#include "fakelua.h"
#include "jit/tcc_handle.h"
#include "util/debug.h"

namespace fakelua {

class VmFunction {
public:
    VmFunction() = default;

    VmFunction(const std::string &name, int arg_count, void *tcc_func_addr, const TCCHandlePtr &tcc_handle)
        : name_(name), arg_count_(arg_count) {
        func_addr_[JIT_TCC] = tcc_func_addr;
        handle_[JIT_TCC] = tcc_handle;
    }

    ~VmFunction() = default;

    [[nodiscard]] bool Empty() const {
        return name_.empty();
    }

    [[nodiscard]] const std::string &GetName() const {
        return name_;
    }

    [[nodiscard]] int GetArgCount() const {
        return arg_count_;
    }

    [[nodiscard]] void *GetAddr(JITType type) const {
        DEBUG_ASSERT(type >= 0 && type < JIT_MAX);
        return func_addr_[type];
    }

    [[nodiscard]] JITHandlePtr GetHandle(JITType type) const {
        DEBUG_ASSERT(type >= 0 && type < JIT_MAX);
        return handle_[type];
    }

    void Merge(const VmFunction &func) {
        for (int i = 0; i < JIT_MAX; ++i) {
            if (!func.func_addr_[i]) {
                continue;
            }
            func_addr_[i] = func.func_addr_[i];
            handle_[i] = func.handle_[i];
        }
    }

private:
    std::string name_;
    int arg_count_ = 0;
    void *func_addr_[JIT_MAX] = {nullptr};
    JITHandlePtr handle_[JIT_MAX];
};

}// namespace fakelua
