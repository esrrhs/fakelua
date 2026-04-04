#pragma once

#include "compile/syntax_tree.h"
#include "fakelua.h"
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

extern "C" void *FakeluaAllocTemp(State *s, size_t size);
extern "C" void FakeluaThrowError(State *s, const char *msg);

//
// extern "C" __attribute__((used)) Var *NewVarTable(State *s, bool is_const, int n, ...);
//
// extern "C" __attribute__((used)) Var WrapReturnVar(State *s, bool is_const, int n, ...);
//
// extern "C" __attribute__((used)) void AssignVar(State *s, bool is_const, int left_n, int right_n, ...);
//
// extern "C" __attribute__((used)) Var *BinopPlus(State *s, bool is_const, Var *l, Var *r);
//
// extern "C" __attribute__((used)) Var *BinopPlus(State *s, bool is_const, Var *l, Var *r);
//
// extern "C" __attribute__((used)) Var *BinopMinus(State *s, bool is_const, Var *l, Var *r);
//
// extern "C" __attribute__((used)) Var *BinopStar(State *s, bool is_const, Var *l, Var *r);
//
// extern "C" __attribute__((used)) Var *BinopSlash(State *s, bool is_const, Var *l, Var *r);
//
// extern "C" __attribute__((used)) Var *BinopDoubleSlash(State *s, bool is_const, Var *l, Var *r);
//
// extern "C" __attribute__((used)) Var *BinopPow(State *s, bool is_const, Var *l, Var *r);
//
// extern "C" __attribute__((used)) Var *BinopMod(State *s, bool is_const, Var *l, Var *r);
//
// extern "C" __attribute__((used)) Var *BinopBitand(State *s, bool is_const, Var *l, Var *r);
//
// extern "C" __attribute__((used)) Var *BinopXor(State *s, bool is_const, Var *l, Var *r);
//
// extern "C" __attribute__((used)) Var *BinopBitor(State *s, bool is_const, Var *l, Var *r);
//
// extern "C" __attribute__((used)) Var *BinopRightShift(State *s, bool is_const, Var *l, Var *r);
//
// extern "C" __attribute__((used)) Var *BinopLeftShift(State *s, bool is_const, Var *l, Var *r);
//
// extern "C" __attribute__((used)) Var *BinopConcat(State *s, bool is_const, Var *l, Var *r);
//
// extern "C" __attribute__((used)) Var *BinopLess(State *s, bool is_const, Var *l, Var *r);
//
// extern "C" __attribute__((used)) Var *BinopLessEqual(State *s, bool is_const, Var *l, Var *r);
//
// extern "C" __attribute__((used)) Var *BinopMore(State *s, bool is_const, Var *l, Var *r);
//
// extern "C" __attribute__((used)) Var *BinopMoreEqual(State *s, bool is_const, Var *l, Var *r);
//
// extern "C" __attribute__((used)) Var *BinopEqual(State *s, bool is_const, Var *l, Var *r);
//
// extern "C" __attribute__((used)) Var *BinopNotEqual(State *s, bool is_const, Var *l, Var *r);
//
// extern "C" __attribute__((used)) bool TestVar(State *s, bool is_const, Var *v);
//
// extern "C" __attribute__((used)) bool TestNotVar(State *s, bool is_const, Var *v);
//
// extern "C" __attribute__((used)) Var *UnopMinus(State *s, bool is_const, Var *r);
//
// extern "C" __attribute__((used)) Var *UnopNot(State *s, bool is_const, Var *r);
//
// extern "C" __attribute__((used)) Var *UnopNumberSign(State *s, bool is_const, Var *r);
//
// extern "C" __attribute__((used)) Var *UnopBitnot(State *s, bool is_const, Var *r);
//
// extern "C" __attribute__((used)) Var *CallVar(State *s, bool is_const, Var *func, Var *col_key, int n, ...);
//
// extern "C" __attribute__((used)) Var *TableIndexByVar(State *s, bool is_const, Var *table, Var *key);
//
// extern "C" __attribute__((used)) Var *TableIndexByName(State *s, bool is_const, Var *table, const char *key,
//                                                        int len);
//
// extern "C" __attribute__((used)) Var *TableSet(State *s, bool is_const, Var *table, Var *key, Var *val);
//
// extern "C" __attribute__((used)) size_t TableSize(State *s, bool is_const, Var *table);
//
// extern "C" __attribute__((used)) Var *TableKeyByPos(State *s, bool is_const, Var *table, size_t pos);
//
// extern "C" __attribute__((used)) Var *TableValueByPos(State *s, bool is_const, Var *table, size_t pos);

}// namespace fakelua
