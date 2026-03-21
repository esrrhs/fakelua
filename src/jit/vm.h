#pragma once

#include "compile/syntax_tree.h"
#include "fakelua.h"
#include "gcc_jit_handle.h"
#include "var/var.h"
#include "vm_function.h"

namespace fakelua {

// store all compiled jit functions and some other running used data.
// every state has one Vm.
class Vm {
public:
    Vm() = default;

    ~Vm() = default;

    // register function
    void RegisterFunction(const std::string &name, const VmFunctionPtr &func) {
        vm_functions_[name] = func;
    }

    // get function
    VmFunctionPtr GetFunction(const std::string &name) const {
        const auto iter = vm_functions_.find(name);
        if (iter == vm_functions_.end()) {
            return nullptr;
        }
        return iter->second;
    }

    // get all functions
    const std::unordered_map<std::string, VmFunctionPtr> &GetFunctions() {
        return vm_functions_;
    }

    // alloc global name, life cycle is all through the process, so put it in Vm
    std::string AllocGlobalName() {
        return std::format("__fakelua_global_{}__", global_name_++);
    }

private:
    // all registered functions
    std::unordered_map<std::string, VmFunctionPtr> vm_functions_;
    // global name counter
    uint64_t global_name_ = 0;
};

extern "C" __attribute__((used)) Var *NewVarTable(FakeluaState *s, GccJitHandle *h, bool is_const, int n, ...);

extern "C" __attribute__((used)) Var WrapReturnVar(FakeluaState *s, bool is_const, int n, ...);

extern "C" __attribute__((used)) void AssignVar(FakeluaState *s, GccJitHandle *h, bool is_const, int left_n, int right_n, ...);

extern "C" __attribute__((used)) Var *BinopPlus(FakeluaState *s, GccJitHandle *h, bool is_const, Var *l, Var *r);

extern "C" __attribute__((used)) Var *BinopPlus(FakeluaState *s, GccJitHandle *h, bool is_const, Var *l, Var *r);

extern "C" __attribute__((used)) Var *BinopMinus(FakeluaState *s, GccJitHandle *h, bool is_const, Var *l, Var *r);

extern "C" __attribute__((used)) Var *BinopStar(FakeluaState *s, GccJitHandle *h, bool is_const, Var *l, Var *r);

extern "C" __attribute__((used)) Var *BinopSlash(FakeluaState *s, GccJitHandle *h, bool is_const, Var *l, Var *r);

extern "C" __attribute__((used)) Var *BinopDoubleSlash(FakeluaState *s, GccJitHandle *h, bool is_const, Var *l, Var *r);

extern "C" __attribute__((used)) Var *BinopPow(FakeluaState *s, GccJitHandle *h, bool is_const, Var *l, Var *r);

extern "C" __attribute__((used)) Var *BinopMod(FakeluaState *s, GccJitHandle *h, bool is_const, Var *l, Var *r);

extern "C" __attribute__((used)) Var *BinopBitand(FakeluaState *s, GccJitHandle *h, bool is_const, Var *l, Var *r);

extern "C" __attribute__((used)) Var *BinopXor(FakeluaState *s, GccJitHandle *h, bool is_const, Var *l, Var *r);

extern "C" __attribute__((used)) Var *BinopBitor(FakeluaState *s, GccJitHandle *h, bool is_const, Var *l, Var *r);

extern "C" __attribute__((used)) Var *BinopRightShift(FakeluaState *s, GccJitHandle *h, bool is_const, Var *l, Var *r);

extern "C" __attribute__((used)) Var *BinopLeftShift(FakeluaState *s, GccJitHandle *h, bool is_const, Var *l, Var *r);

extern "C" __attribute__((used)) Var *BinopConcat(FakeluaState *s, GccJitHandle *h, bool is_const, Var *l, Var *r);

extern "C" __attribute__((used)) Var *BinopLess(FakeluaState *s, GccJitHandle *h, bool is_const, Var *l, Var *r);

extern "C" __attribute__((used)) Var *BinopLessEqual(FakeluaState *s, GccJitHandle *h, bool is_const, Var *l, Var *r);

extern "C" __attribute__((used)) Var *BinopMore(FakeluaState *s, GccJitHandle *h, bool is_const, Var *l, Var *r);

extern "C" __attribute__((used)) Var *BinopMoreEqual(FakeluaState *s, GccJitHandle *h, bool is_const, Var *l, Var *r);

extern "C" __attribute__((used)) Var *BinopEqual(FakeluaState *s, GccJitHandle *h, bool is_const, Var *l, Var *r);

extern "C" __attribute__((used)) Var *BinopNotEqual(FakeluaState *s, GccJitHandle *h, bool is_const, Var *l, Var *r);

extern "C" __attribute__((used)) bool TestVar(FakeluaState *s, GccJitHandle *h, bool is_const, Var *v);

extern "C" __attribute__((used)) bool TestNotVar(FakeluaState *s, GccJitHandle *h, bool is_const, Var *v);

extern "C" __attribute__((used)) Var *UnopMinus(FakeluaState *s, GccJitHandle *h, bool is_const, Var *r);

extern "C" __attribute__((used)) Var *UnopNot(FakeluaState *s, GccJitHandle *h, bool is_const, Var *r);

extern "C" __attribute__((used)) Var *UnopNumberSign(FakeluaState *s, GccJitHandle *h, bool is_const, Var *r);

extern "C" __attribute__((used)) Var *UnopBitnot(FakeluaState *s, GccJitHandle *h, bool is_const, Var *r);

extern "C" __attribute__((used)) Var *CallVar(FakeluaState *s, GccJitHandle *h, bool is_const, Var *func, Var *col_key, int n, ...);

extern "C" __attribute__((used)) Var *TableIndexByVar(FakeluaState *s, GccJitHandle *h, bool is_const, Var *table, Var *key);

extern "C" __attribute__((used)) Var *TableIndexByName(FakeluaState *s, GccJitHandle *h, bool is_const, Var *table, const char *key,
                                                       int len);

extern "C" __attribute__((used)) Var *TableSet(FakeluaState *s, GccJitHandle *h, bool is_const, Var *table, Var *key, Var *val);

extern "C" __attribute__((used)) size_t TableSize(FakeluaState *s, GccJitHandle *h, bool is_const, Var *table);

extern "C" __attribute__((used)) Var *TableKeyByPos(FakeluaState *s, GccJitHandle *h, bool is_const, Var *table, size_t pos);

extern "C" __attribute__((used)) Var *TableValueByPos(FakeluaState *s, GccJitHandle *h, bool is_const, Var *table, size_t pos);

}// namespace fakelua
