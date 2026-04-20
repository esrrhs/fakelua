#pragma once

#include "fakelua.h"
#include "jit/vm_function.h"
#include "var/var.h"

namespace fakelua {

// 负责运行时
//
// 线程模型：整个 fakelua 运行时假定单线程使用——一个 State 同一时刻只会被一个线程持有，
// 注册/查找 VmFunction、分配全局名都发生在编译或初始化阶段，不会与执行期读者并发。
// 因此本类所有成员（vm_functions_ 的 find+emplace、global_name_ 的自增）都没有加锁。
// 若将来需要支持多线程共享同一 State，需要在此处补同步；否则请保持调用侧线程亲和性。
class Vm {
public:
    Vm() = default;

    ~Vm() = default;

    // 注册函数（单线程调用，见类注释）
    void RegisterFunction(const VmFunction &func) {
        const auto &name = func.GetName();
        const auto it = vm_functions_.find(name);
        if (it == vm_functions_.end()) {
            vm_functions_.emplace(name, func);
            return;
        }
        it->second.Merge(func);
    }

    // 获取函数
    VmFunction GetFunction(const std::string &name) const {
        const auto iter = vm_functions_.find(name);
        if (iter == vm_functions_.end()) {
            return {};
        }
        return iter->second;
    }

    // 申请全局变量名字
    std::string AllocGlobalName() {
        return std::format("__fakelua_global_{}__", global_name_++);
    }

private:
    std::unordered_map<std::string, VmFunction> vm_functions_;
    uint64_t global_name_ = 0;
};

extern "C" void *FakeluaAllocTemp(State *s, size_t size);

extern "C" void FakeluaThrowError(State *s, const char *msg);

extern "C" CVar FakeluaCallByName(State *s, int jit_type, const char *name, int arg_num, ...);

}// namespace fakelua
