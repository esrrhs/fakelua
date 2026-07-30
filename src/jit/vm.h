#pragma once

#include "fakelua.h"
#include "jit/vm_function.h"
#include "var/var.h"
#include <functional>

namespace fakelua {

// 虚拟机：负责运行时函数注册与查找
//
// 线程模型：整个 fakelua 运行时假定单线程使用——一个 State 同一时刻只会被一个线程持有，
// 注册/查找 VmFunction、分配全局名都发生在编译或初始化阶段，不会与执行期读者并发。
// 因此本类所有成员（vm_functions_ 的 find+emplace、global_name_ 的自增）都没有加锁。
// 若将来需要支持多线程共享同一 State，需要在此处补同步；否则请保持调用侧线程亲和性。
// C++ 原生函数回调：接收当前 State*、参数数组和参数个数，返回 CVar
using NativeFuncCallback = std::function<CVar(State *, CVar *, int)>;

// 原生函数注册条目
struct NativeFuncEntry {
    int arg_count = 0;
    bool is_vararg = false;
    NativeFuncCallback callback;
};

class Vm {
public:
    Vm() = default;

    // 注册 JIT 编译的 fakelua 函数（单线程调用，见类注释）
    void RegisterFunction(const VmFunction &func) {
        const auto &name = func.GetName();
        if (const auto iter = vm_functions_.find(name); iter == vm_functions_.end()) {
            vm_functions_.emplace(name, func);
        } else {
            iter->second.Merge(func);
        }
    }

    // 获取 JIT 函数
    [[nodiscard]] VmFunction GetFunction(const std::string &name) const {
        if (const auto iter = vm_functions_.find(name); iter != vm_functions_.end()) {
            return iter->second;
        }
        return {};
    }

    // 注册 C++ 原生函数（可在编译前或编译后调用）
    // 注册后可在 lua 脚本中直接以函数名调用，通过 FakeluaCallByName 分发
    void RegisterNativeFunction(const std::string &name, int arg_count, bool is_vararg, NativeFuncCallback callback) {
        native_functions_[name] = NativeFuncEntry{arg_count, is_vararg, std::move(callback)};
    }

    // 查找原生函数条目（供 FakeluaCallByName 使用）
    [[nodiscard]] const NativeFuncEntry *FindNativeFunction(const std::string &name) const {
        const auto it = native_functions_.find(name);
        return it != native_functions_.end() ? &it->second : nullptr;
    }

    // 分配一个唯一的全局变量名
    std::string AllocGlobalName() {
        return std::format("__fakelua_global_{}__", global_name_++);
    }

private:
    std::unordered_map<std::string, VmFunction> vm_functions_;
    std::unordered_map<std::string, NativeFuncEntry> native_functions_;
    uint64_t global_name_ = 0;
};

extern "C" void *FakeluaAlloc(State *state, size_t size, bool is_const);

extern "C" void FakeluaThrowError(State *state, const char *msg);

extern "C" CVar FakeluaCallByName(State *state, int jit_type, const char *name, int arg_num, ...);

extern "C" CVar FlEvalLoadClosure(State *state, VarClosure *cl, int arg_num, const CVar *args);

}// namespace fakelua
