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

extern "C" __attribute__((used)) Var *NewVarTable(State *s, GccJitHandle *h, bool is_const, int n, ...);

extern "C" __attribute__((used)) Var WrapReturnVar(State *s, bool is_const, int n, ...);

extern "C" __attribute__((used)) void AssignVar(State *s, GccJitHandle *h, bool is_const, int left_n, int right_n, ...);

extern "C" __attribute__((used)) Var *BinopPlus(State *s, GccJitHandle *h, bool is_const, Var *l, Var *r);

extern "C" __attribute__((used)) Var *BinopPlus(State *s, GccJitHandle *h, bool is_const, Var *l, Var *r);

extern "C" __attribute__((used)) Var *BinopMinus(State *s, GccJitHandle *h, bool is_const, Var *l, Var *r);

extern "C" __attribute__((used)) Var *BinopStar(State *s, GccJitHandle *h, bool is_const, Var *l, Var *r);

extern "C" __attribute__((used)) Var *BinopSlash(State *s, GccJitHandle *h, bool is_const, Var *l, Var *r);

extern "C" __attribute__((used)) Var *BinopDoubleSlash(State *s, GccJitHandle *h, bool is_const, Var *l, Var *r);

extern "C" __attribute__((used)) Var *BinopPow(State *s, GccJitHandle *h, bool is_const, Var *l, Var *r);

extern "C" __attribute__((used)) Var *BinopMod(State *s, GccJitHandle *h, bool is_const, Var *l, Var *r);

extern "C" __attribute__((used)) Var *BinopBitand(State *s, GccJitHandle *h, bool is_const, Var *l, Var *r);

extern "C" __attribute__((used)) Var *BinopXor(State *s, GccJitHandle *h, bool is_const, Var *l, Var *r);

extern "C" __attribute__((used)) Var *BinopBitor(State *s, GccJitHandle *h, bool is_const, Var *l, Var *r);

extern "C" __attribute__((used)) Var *BinopRightShift(State *s, GccJitHandle *h, bool is_const, Var *l, Var *r);

extern "C" __attribute__((used)) Var *BinopLeftShift(State *s, GccJitHandle *h, bool is_const, Var *l, Var *r);

extern "C" __attribute__((used)) Var *BinopConcat(State *s, GccJitHandle *h, bool is_const, Var *l, Var *r);

extern "C" __attribute__((used)) Var *BinopLess(State *s, GccJitHandle *h, bool is_const, Var *l, Var *r);

extern "C" __attribute__((used)) Var *BinopLessEqual(State *s, GccJitHandle *h, bool is_const, Var *l, Var *r);

extern "C" __attribute__((used)) Var *BinopMore(State *s, GccJitHandle *h, bool is_const, Var *l, Var *r);

extern "C" __attribute__((used)) Var *BinopMoreEqual(State *s, GccJitHandle *h, bool is_const, Var *l, Var *r);

extern "C" __attribute__((used)) Var *BinopEqual(State *s, GccJitHandle *h, bool is_const, Var *l, Var *r);

extern "C" __attribute__((used)) Var *BinopNotEqual(State *s, GccJitHandle *h, bool is_const, Var *l, Var *r);

extern "C" __attribute__((used)) bool TestVar(State *s, GccJitHandle *h, bool is_const, Var *v);

extern "C" __attribute__((used)) bool TestNotVar(State *s, GccJitHandle *h, bool is_const, Var *v);

extern "C" __attribute__((used)) Var *UnopMinus(State *s, GccJitHandle *h, bool is_const, Var *r);

extern "C" __attribute__((used)) Var *UnopNot(State *s, GccJitHandle *h, bool is_const, Var *r);

extern "C" __attribute__((used)) Var *UnopNumberSign(State *s, GccJitHandle *h, bool is_const, Var *r);

extern "C" __attribute__((used)) Var *UnopBitnot(State *s, GccJitHandle *h, bool is_const, Var *r);

extern "C" __attribute__((used)) Var *CallVar(State *s, GccJitHandle *h, bool is_const, Var *func, Var *col_key, int n, ...);

extern "C" __attribute__((used)) Var *TableIndexByVar(State *s, GccJitHandle *h, bool is_const, Var *table, Var *key);

extern "C" __attribute__((used)) Var *TableIndexByName(State *s, GccJitHandle *h, bool is_const, Var *table, const char *key,
                                                       int len);

extern "C" __attribute__((used)) Var *TableSet(State *s, GccJitHandle *h, bool is_const, Var *table, Var *key, Var *val);

extern "C" __attribute__((used)) size_t TableSize(State *s, GccJitHandle *h, bool is_const, Var *table);

extern "C" __attribute__((used)) Var *TableKeyByPos(State *s, GccJitHandle *h, bool is_const, Var *table, size_t pos);

extern "C" __attribute__((used)) Var *TableValueByPos(State *s, GccJitHandle *h, bool is_const, Var *table, size_t pos);

}// namespace fakelua
