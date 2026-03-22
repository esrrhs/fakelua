#pragma once

#include "const_string.h"
#include "fakelua.h"
#include "heap.h"
#include "jit/Vm.h"
#include "stack.h"
#include "var_table_heap.h"

namespace fakelua {

// 单个运行实例
class State {
public:
    explicit State(StateConfig config = {});

    ~State() = default;

    void CompileFile(const std::string &filename, const CompileConfig &cfg);

    void CompileString(const std::string &str, const CompileConfig &cfg);

    void Reset() {
        DEBUG_ASSERT(reentrant_count_ == 0);
        heap_.Reset();
        var_table_heap_.reset();
        stack_.reset();
    }

    Heap &GetHeap() {
        return heap_;
    }

    VarTableHeap &get_var_table_heap() {
        return var_table_heap_;
    }

    Vm &get_vm() {
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
    StateConfig config_;
    Heap heap_;
    ConstString const_string_;
    VarTableHeap var_table_heap_;
    Vm vm_;
    int reentrant_count_ = 0;// used to reset
    stack stack_;
};

}// namespace fakelua
