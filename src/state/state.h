#pragma once

#include "compile/compiler.h"
#include "fakelua.h"
#include "jit/vm.h"
#include "state/const_string.h"
#include "state/heap.h"

struct TCCState;

namespace fakelua {

// 单个运行实例
//
// 线程模型：State 是单线程实体，由调用方保证同一时刻只有一个线程访问它。
// 本类所有成员（包括 reentrant_count_ 的 Add/Sub、heap_、vm_ 等）都未加锁：
//  - reentrant_count_ 只在脚本执行入口/出口处自增自减，用来检测重入而非跨线程计数；
//  - heap_ / vm_ / const_string_ 也依赖调用方的线程亲和性。
// 如果将来需要跨线程共享同一 State，需要在此处引入同步或改为每线程一份 State。
class State {
public:
    explicit State(const StateConfig &config = {});

    ~State() = default;

    void Reset() {
        DEBUG_ASSERT(reentrant_count_ == 0);
        heap_.Reset();
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

    // 单线程调用（见类注释），无需原子操作。
    void AddReentrantCount() {
        ++reentrant_count_;
    }

    // 单线程调用（见类注释），无需原子操作。
    void SubReentrantCount() {
        --reentrant_count_;
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
};

}// namespace fakelua
