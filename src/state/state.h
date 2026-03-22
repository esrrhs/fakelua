#pragma once

#include "fakelua.h"
#include "jit/Vm.h"
#include "stack.h"
#include "const_string.h"
#include "heap.h"
#include "var_table_heap.h"

namespace fakelua {

// the state contains the running environment we need.
class State final : public FakeluaState {
public:
    State(StateConfig config = {});

    ~State() override = default;

    void CompileFile(const std::string &filename, const CompileConfig &cfg) override;

    void CompileString(const std::string &str, const CompileConfig &cfg) override;

    // call before running. this will reset the state. just for speed.
    void reset() {
        DEBUG_ASSERT(reentrant_count_ == 0);
        var_string_heap_.reset();
        var_table_heap_.reset();
        stack_.reset();
    }

    Heap &get_var_string_heap() {
        return var_string_heap_;
    }

    VarTableHeap &get_var_table_heap() {
        return var_table_heap_;
    }

    Vm &get_vm() {
        return vm_;
    }

    int &get_reentrant_count() {
        return reentrant_count_;
    }

    stack &get_stack() {
        return stack_;
    }

private:
    VarStringHeap var_string_heap_;
    VarTableHeap var_table_heap_;
    Vm vm_;
    int reentrant_count_ = 0;// used to reset
    stack stack_;
};

}// namespace fakelua
