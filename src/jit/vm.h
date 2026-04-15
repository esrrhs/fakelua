#pragma once

#include "fakelua.h"
#include "jit/vm_function.h"
#include "var/var.h"

namespace fakelua {

// 负责运行时
class Vm {
public:
    Vm() = default;

    ~Vm() = default;

    // 注册函数
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
