#pragma once

#include "compile/compiler.h"
#include "fakelua.h"
#include "jit/vm.h"
#include "state/const_string.h"
#include "state/heap.h"
#include "state/stack.h"

struct TCCState;

namespace fakelua {

// 单个运行实例
class State {
public:
    explicit State(const StateConfig &config = {});

    ~State() = default;

    void Reset() {
        DEBUG_ASSERT(reentrant_count_ == 0);
        heap_.Reset();
        stack_.reset();
    }

    const StateConfig &GetStateConfig() const {
        return config_;
    }

    Compiler &GetCompiler() {
        return compiler_;
    }

    Heap &GetHeap() {
        return heap_;
    }

    ConstString &GetConstString() {
        return const_string_;
    }

    Vm &GetVM() {
        return vm_;
    }

    int GetReentrantCount() const {
        return reentrant_count_;
    }

    void AddReentrantCount() {
        ++reentrant_count_;
    }

    void SubReentrantCount() {
        --reentrant_count_;
    }

    stack &get_stack() {
        return stack_;
    }

    void SetVarInterfaceNewFunc(const std::function<VarInterface *()> &func) {
        DEBUG_ASSERT(func != nullptr);
        var_interface_new_func_ = func;
    }

    std::function<VarInterface *()> &GetVarInterfaceNewFunc() {
        return var_interface_new_func_;
    }

private:
    std::function<VarInterface *()> var_interface_new_func_;
    int reentrant_count_ = 0;
    StateConfig config_;
    Compiler compiler_;
    Heap heap_;
    ConstString const_string_;
    Vm vm_;
    stack stack_;
};

}// namespace fakelua
