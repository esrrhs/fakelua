#include "vm.h"
#include "fakelua.h"
#include "state/state.h"
#include "util/common.h"
#include "var/var.h"
#include "var/var_table.h"
#include "var/var_string.h"
#include <stdarg.h>

namespace fakelua {

extern "C" void *FakeluaAllocTemp(State *s, size_t size) {
    return s->GetHeap().GetTempAllocator().Alloc(size);
}

extern "C" void FakeluaThrowError(State *s, const char *msg) {
    ThrowFakeluaException(msg);
}

extern "C" __attribute__((used)) CVar FakeluaCallByName(State *s, int jit_type, const char *name, int arg_num, ...) {
    const auto func = s->GetVM().GetFunction(std::string(name));
    if (func.Empty()) {
        ThrowFakeluaException(std::format("FakeluaCallByName: function '{}' not found", name));
    }
    void *addr = func.GetAddr(static_cast<JITType>(jit_type));
    if (!addr) {
        ThrowFakeluaException(std::format("FakeluaCallByName: function '{}' has no address for jit_type {}", name, jit_type));
    }

    // Maximum 8 arguments supported (matches the switch below).
    if (arg_num > 8) {
        ThrowFakeluaException(
                std::format("FakeluaCallByName: too many arguments ({}) for function '{}', max is 8", arg_num, name));
    }

    CVar arg_arr[8];
    va_list vl;
    va_start(vl, arg_num);
    for (int i = 0; i < arg_num; ++i) {
        arg_arr[i] = va_arg(vl, CVar);
    }
    va_end(vl);

    // Uses the same variadic function pointer cast pattern as Call() in fakelua.h.
    // All JIT-compiled functions accept/return CVar, so the ABI is uniform.
    auto fn = reinterpret_cast<CVar (*)(...)>(addr);
    switch (arg_num) {
        case 0: return fn();
        case 1: return fn(arg_arr[0]);
        case 2: return fn(arg_arr[0], arg_arr[1]);
        case 3: return fn(arg_arr[0], arg_arr[1], arg_arr[2]);
        case 4: return fn(arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3]);
        case 5: return fn(arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4]);
        case 6: return fn(arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5]);
        case 7: return fn(arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6]);
        case 8: return fn(arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7]);
        default:
            ThrowFakeluaException(
                    std::format("FakeluaCallByName: too many arguments ({}) for function '{}'", arg_num, name));
    }
}

//
// static Var *alloc_val_helper(State *s, bool is_const) {
//     if (is_const) {
//         return s->GetHeap().GetConstAllocator().New<Var>();
//     } else {
//         return s->GetHeap().GetTempAllocator().New<Var>();
//     }
// }
//
// // Expand the list of expressions that may contain variable parameters.
// // eg: function test() return 1,2,3 end
// // 1,2,test() => 1,2,1,2,3
// // 1,2,test(),3 => 1,2,1,3
// static void ExpandVarList(std::vector<Var *> &params) {
//     for (size_t i = 0; i < params.size(); i++) {
//         const auto param = params[i];
//         DEBUG_ASSERT(param);
//         DEBUG_ASSERT(param->Type() >= VarType::Min && param->Type() <= VarType::Max);
//     }
// }
//
// static void ExpandVarList(std::vector<Var> &params) {
//     for (size_t i = 0; i < params.size(); i++) {
//         const auto param = params[i];
//         DEBUG_ASSERT(param.Type() >= VarType::Min && param.Type() <= VarType::Max);
//     }
// }
//
// extern "C" __attribute__((used)) Var *NewVarTable(State *s, bool is_const, int n, ...) {
//     DEBUG_ASSERT(n % 2 == 0);
//     std::vector<Var *> keys;
//     std::vector<Var *> values;
//     va_list args;
//     va_start(args, n);
//     for (int i = 0; i < n; i += 2) {
//         auto arg = va_arg(args, Var *);
//         DEBUG_ASSERT(!arg || (arg->Type() >= VarType::Min && arg->Type() <= VarType::Max));
//         keys.push_back(arg);
//         arg = va_arg(args, Var *);
//         DEBUG_ASSERT(arg->Type() >= VarType::Min && arg->Type() <= VarType::Max);
//         values.push_back(arg);
//     }
//     va_end(args);
//
//     auto ret = alloc_val_helper(s, is_const);
//     if (!ret) {
//         return nullptr;
//     }
//     ret->SetTable(s);
//
//     ExpandVarList(values);
//
//     int index = 1;
//     for (size_t i = 0; i < values.size(); i++) {
//         auto k = i < keys.size() ? keys[i] : nullptr;
//         if (!k) {
//             k = alloc_val_helper(s, is_const);
//             k->SetInt(index);
//             index++;
//         }
//         auto v = values[i];
//         ret->TableSet(s, *k, *v, false);
//     }
//     return ret;
// }
//
// extern "C" __attribute__((used)) Var WrapReturnVar(State *s, bool is_const, int n, ...) {
//     DEBUG_ASSERT(n >= 0);
//     // std::vector<Var> params;
//     // va_list args;
//     // va_start(args, n);
//     // for (int i = 0; i < n; i++) {
//     //     auto arg = va_arg(args, Var);
//     //     DEBUG_ASSERT(arg.Type() >= VarType::Min && arg.Type() <= VarType::Max);
//     //     params.push_back(arg);
//     // }
//     // va_end(args);
//     //
//     // Var ret;
//     // ret.SetTable(s);
//     //
//     // ExpandVarList(params);
//     //
//     // // push to ret
//     // for (size_t i = 0; i < params.size(); i++) {
//     //     auto arg = params[i];
//     //     Var k;
//     //     k.SetInt(static_cast<int64_t>(i) + 1);
//     //     ret.TableSet(s, k, arg, true);
//     // }
//     // return ret;
// }
//
// extern "C" __attribute__((used)) void AssignVar(State *s, bool is_const, int left_n, int right_n, ...) {
//     DEBUG_ASSERT(left_n >= 0);
//     DEBUG_ASSERT(right_n >= 0);
//     std::vector<Var **> left;
//     std::vector<Var *> right;
//     va_list args;
//     va_start(args, right_n);
//     for (int i = 0; i < left_n; i++) {
//         auto dst = va_arg(args, Var **);
//         DEBUG_ASSERT(dst);
//         DEBUG_ASSERT(!*dst || (*dst && (*dst)->Type() >= VarType::Min && (*dst)->Type() <= VarType::Max));
//         left.push_back(dst);
//     }
//     for (int i = 0; i < right_n; i++) {
//         auto arg = va_arg(args, Var *);
//         DEBUG_ASSERT(arg);
//         DEBUG_ASSERT(arg->Type() >= VarType::Min && arg->Type() <= VarType::Max);
//         right.push_back(arg);
//     }
//     va_end(args);
//
//     ExpandVarList(right);
//
//     // local a, b, c = x, y, z
//     for (size_t i = 0; i < left.size(); i++) {
//         auto dst = left[i];
//         if (i < right.size()) {
//             *dst = right[i];
//         } else {
//             break;
//         }
//     }
// }
//
// extern "C" __attribute__((used)) Var *BinopPlus(State *s, bool is_const, Var *l, Var *r) {
//     DEBUG_ASSERT(l);
//     DEBUG_ASSERT(r);
//     DEBUG_ASSERT(l->Type() >= VarType::Min && l->Type() <= VarType::Max);
//     DEBUG_ASSERT(r->Type() >= VarType::Min && r->Type() <= VarType::Max);
//     auto ret = alloc_val_helper(s, is_const);
//     if (ret) {
//         l->Plus(*r, *ret);
//     }
//     return ret;
// }
//
// extern "C" __attribute__((used)) Var *BinopMinus(State *s, bool is_const, Var *l, Var *r) {
//     DEBUG_ASSERT(l);
//     DEBUG_ASSERT(r);
//     DEBUG_ASSERT(l->Type() >= VarType::Min && l->Type() <= VarType::Max);
//     DEBUG_ASSERT(r->Type() >= VarType::Min && r->Type() <= VarType::Max);
//     auto ret = alloc_val_helper(s, is_const);
//     if (ret) {
//         l->Minus(*r, *ret);
//     }
//     return ret;
// }
//
// extern "C" __attribute__((used)) Var *BinopStar(State *s, bool is_const, Var *l, Var *r) {
//     DEBUG_ASSERT(l);
//     DEBUG_ASSERT(r);
//     DEBUG_ASSERT(l->Type() >= VarType::Min && l->Type() <= VarType::Max);
//     DEBUG_ASSERT(r->Type() >= VarType::Min && r->Type() <= VarType::Max);
//     auto ret = alloc_val_helper(s, is_const);
//     if (ret) {
//         l->Star(*r, *ret);
//     }
//     return ret;
// }
//
// extern "C" __attribute__((used)) Var *BinopSlash(State *s, bool is_const, Var *l, Var *r) {
//     DEBUG_ASSERT(l);
//     DEBUG_ASSERT(r);
//     DEBUG_ASSERT(l->Type() >= VarType::Min && l->Type() <= VarType::Max);
//     DEBUG_ASSERT(r->Type() >= VarType::Min && r->Type() <= VarType::Max);
//     auto ret = alloc_val_helper(s, is_const);
//     if (ret) {
//         l->Slash(*r, *ret);
//     }
//     return ret;
// }
//
// extern "C" __attribute__((used)) Var *BinopDoubleSlash(State *s, bool is_const, Var *l, Var *r) {
//     DEBUG_ASSERT(l);
//     DEBUG_ASSERT(r);
//     DEBUG_ASSERT(l->Type() >= VarType::Min && l->Type() <= VarType::Max);
//     DEBUG_ASSERT(r->Type() >= VarType::Min && r->Type() <= VarType::Max);
//     auto ret = alloc_val_helper(s, is_const);
//     if (ret) {
//         l->DoubleSlash(*r, *ret);
//     }
//     return ret;
// }
//
// extern "C" __attribute__((used)) Var *BinopPow(State *s, bool is_const, Var *l, Var *r) {
//     DEBUG_ASSERT(l);
//     DEBUG_ASSERT(r);
//     DEBUG_ASSERT(l->Type() >= VarType::Min && l->Type() <= VarType::Max);
//     DEBUG_ASSERT(r->Type() >= VarType::Min && r->Type() <= VarType::Max);
//     auto ret = alloc_val_helper(s, is_const);
//     if (ret) {
//         l->Pow(*r, *ret);
//     }
//     return ret;
// }
//
// extern "C" __attribute__((used)) Var *BinopMod(State *s, bool is_const, Var *l, Var *r) {
//     DEBUG_ASSERT(l);
//     DEBUG_ASSERT(r);
//     DEBUG_ASSERT(l->Type() >= VarType::Min && l->Type() <= VarType::Max);
//     DEBUG_ASSERT(r->Type() >= VarType::Min && r->Type() <= VarType::Max);
//     auto ret = alloc_val_helper(s, is_const);
//     if (ret) {
//         l->Mod(*r, *ret);
//     }
//     return ret;
// }
//
// extern "C" __attribute__((used)) Var *BinopBitand(State *s, bool is_const, Var *l, Var *r) {
//     DEBUG_ASSERT(l);
//     DEBUG_ASSERT(r);
//     DEBUG_ASSERT(l->Type() >= VarType::Min && l->Type() <= VarType::Max);
//     DEBUG_ASSERT(r->Type() >= VarType::Min && r->Type() <= VarType::Max);
//     auto ret = alloc_val_helper(s, is_const);
//     if (ret) {
//         l->Bitand(*r, *ret);
//     }
//     return ret;
// }
//
// extern "C" __attribute__((used)) Var *BinopXor(State *s, bool is_const, Var *l, Var *r) {
//     DEBUG_ASSERT(l);
//     DEBUG_ASSERT(r);
//     DEBUG_ASSERT(l->Type() >= VarType::Min && l->Type() <= VarType::Max);
//     DEBUG_ASSERT(r->Type() >= VarType::Min && r->Type() <= VarType::Max);
//     auto ret = alloc_val_helper(s, is_const);
//     if (ret) {
//         l->Xor(*r, *ret);
//     }
//     return ret;
// }
//
// extern "C" __attribute__((used)) Var *BinopBitor(State *s, bool is_const, Var *l, Var *r) {
//     DEBUG_ASSERT(l);
//     DEBUG_ASSERT(r);
//     DEBUG_ASSERT(l->Type() >= VarType::Min && l->Type() <= VarType::Max);
//     DEBUG_ASSERT(r->Type() >= VarType::Min && r->Type() <= VarType::Max);
//     auto ret = alloc_val_helper(s, is_const);
//     if (ret) {
//         l->Bitor(*r, *ret);
//     }
//     return ret;
// }
//
// extern "C" __attribute__((used)) Var *BinopRightShift(State *s, bool is_const, Var *l, Var *r) {
//     DEBUG_ASSERT(l);
//     DEBUG_ASSERT(r);
//     DEBUG_ASSERT(l->Type() >= VarType::Min && l->Type() <= VarType::Max);
//     DEBUG_ASSERT(r->Type() >= VarType::Min && r->Type() <= VarType::Max);
//     auto ret = alloc_val_helper(s, is_const);
//     if (ret) {
//         l->RightShift(*r, *ret);
//     }
//     return ret;
// }
//
// extern "C" __attribute__((used)) Var *BinopLeftShift(State *s, bool is_const, Var *l, Var *r) {
//     DEBUG_ASSERT(l);
//     DEBUG_ASSERT(r);
//     DEBUG_ASSERT(l->Type() >= VarType::Min && l->Type() <= VarType::Max);
//     DEBUG_ASSERT(r->Type() >= VarType::Min && r->Type() <= VarType::Max);
//     auto ret = alloc_val_helper(s, is_const);
//     if (ret) {
//         l->LeftShift(*r, *ret);
//     }
//     return ret;
// }
//
// extern "C" __attribute__((used)) Var *BinopConcat(State *s, bool is_const, Var *l, Var *r) {
//     DEBUG_ASSERT(l);
//     DEBUG_ASSERT(r);
//     DEBUG_ASSERT(l->Type() >= VarType::Min && l->Type() <= VarType::Max);
//     DEBUG_ASSERT(r->Type() >= VarType::Min && r->Type() <= VarType::Max);
//     auto ret = alloc_val_helper(s, is_const);
//     if (ret) {
//         l->Concat(s, *r, *ret);
//     }
//     return ret;
// }
//
// extern "C" __attribute__((used)) Var *BinopLess(State *s, bool is_const, Var *l, Var *r) {
//     DEBUG_ASSERT(l);
//     DEBUG_ASSERT(r);
//     DEBUG_ASSERT(l->Type() >= VarType::Min && l->Type() <= VarType::Max);
//     DEBUG_ASSERT(r->Type() >= VarType::Min && r->Type() <= VarType::Max);
//     auto ret = alloc_val_helper(s, is_const);
//     if (ret) {
//         l->Less(*r, *ret);
//     }
//     return ret;
// }
//
// extern "C" __attribute__((used)) Var *BinopLessEqual(State *s, bool is_const, Var *l, Var *r) {
//     DEBUG_ASSERT(l);
//     DEBUG_ASSERT(r);
//     DEBUG_ASSERT(l->Type() >= VarType::Min && l->Type() <= VarType::Max);
//     DEBUG_ASSERT(r->Type() >= VarType::Min && r->Type() <= VarType::Max);
//     auto ret = alloc_val_helper(s, is_const);
//     if (ret) {
//         l->LessEqual(*r, *ret);
//     }
//     return ret;
// }
//
// extern "C" __attribute__((used)) Var *BinopMore(State *s, bool is_const, Var *l, Var *r) {
//     DEBUG_ASSERT(l);
//     DEBUG_ASSERT(r);
//     DEBUG_ASSERT(l->Type() >= VarType::Min && l->Type() <= VarType::Max);
//     DEBUG_ASSERT(r->Type() >= VarType::Min && r->Type() <= VarType::Max);
//     auto ret = alloc_val_helper(s, is_const);
//     if (ret) {
//         l->More(*r, *ret);
//     }
//     return ret;
// }
//
// extern "C" __attribute__((used)) Var *BinopMoreEqual(State *s, bool is_const, Var *l, Var *r) {
//     DEBUG_ASSERT(l);
//     DEBUG_ASSERT(r);
//     DEBUG_ASSERT(l->Type() >= VarType::Min && l->Type() <= VarType::Max);
//     DEBUG_ASSERT(r->Type() >= VarType::Min && r->Type() <= VarType::Max);
//     auto ret = alloc_val_helper(s, is_const);
//     if (ret) {
//         l->MoreEqual(*r, *ret);
//     }
//     return ret;
// }
//
// extern "C" __attribute__((used)) Var *BinopEqual(State *s, bool is_const, Var *l, Var *r) {
//     DEBUG_ASSERT(l);
//     DEBUG_ASSERT(r);
//     DEBUG_ASSERT(l->Type() >= VarType::Min && l->Type() <= VarType::Max);
//     DEBUG_ASSERT(r->Type() >= VarType::Min && r->Type() <= VarType::Max);
//     auto ret = alloc_val_helper(s, is_const);
//     if (ret) {
//         l->Equal(*r, *ret);
//     }
//     return ret;
// }
//
// extern "C" __attribute__((used)) Var *BinopNotEqual(State *s, bool is_const, Var *l, Var *r) {
//     DEBUG_ASSERT(l);
//     DEBUG_ASSERT(r);
//     DEBUG_ASSERT(l->Type() >= VarType::Min && l->Type() <= VarType::Max);
//     DEBUG_ASSERT(r->Type() >= VarType::Min && r->Type() <= VarType::Max);
//     auto ret = alloc_val_helper(s, is_const);
//     if (ret) {
//         l->NotEqual(*r, *ret);
//     }
//     return ret;
// }
//
// extern "C" __attribute__((used)) bool TestVar(State *s, bool is_const, Var *v) {
//     DEBUG_ASSERT(v);
//     DEBUG_ASSERT(v->Type() >= VarType::Min && v->Type() <= VarType::Max);
//     return v->TestTrue();
// }
//
// extern "C" __attribute__((used)) bool TestNotVar(State *s, bool is_const, Var *v) {
//     DEBUG_ASSERT(v);
//     DEBUG_ASSERT(v->Type() >= VarType::Min && v->Type() <= VarType::Max);
//     return !v->TestTrue();
// }
//
// extern "C" __attribute__((used)) Var *UnopMinus(State *s, bool is_const, Var *r) {
//     DEBUG_ASSERT(r);
//     DEBUG_ASSERT(r->Type() >= VarType::Min && r->Type() <= VarType::Max);
//     auto ret = alloc_val_helper(s, is_const);
//     if (ret) {
//         r->UnopMinus(*ret);
//     }
//     return ret;
// }
//
// extern "C" __attribute__((used)) Var *UnopNot(State *s, bool is_const, Var *r) {
//     DEBUG_ASSERT(r);
//     DEBUG_ASSERT(r->Type() >= VarType::Min && r->Type() <= VarType::Max);
//     auto ret = alloc_val_helper(s, is_const);
//     if (ret) {
//         r->UnopNot(*ret);
//     }
//     return ret;
// }
//
// extern "C" __attribute__((used)) Var *UnopNumberSign(State *s, bool is_const, Var *r) {
//     DEBUG_ASSERT(r);
//     DEBUG_ASSERT(r->Type() >= VarType::Min && r->Type() <= VarType::Max);
//     auto ret = alloc_val_helper(s, is_const);
//     if (ret) {
//         r->UnopNumberSign(*ret);
//     }
//     return ret;
// }
//
// extern "C" __attribute__((used)) Var *UnopBitnot(State *s, bool is_const, Var *r) {
//     DEBUG_ASSERT(r);
//     DEBUG_ASSERT(r->Type() >= VarType::Min && r->Type() <= VarType::Max);
//     auto ret = alloc_val_helper(s, is_const);
//     if (ret) {
//         r->UnopBitnot(*ret);
//     }
//     return ret;
// }
//
// extern "C" __attribute__((used)) Var *CallVar(State *s, bool is_const, Var *func, Var *col_key, int n, ...) {
//     DEBUG_ASSERT(func);
//     DEBUG_ASSERT(func->Type() >= VarType::Min && func->Type() <= VarType::Max);
//     DEBUG_ASSERT(n >= 0);
//
//     // prepare params
//     std::vector<Var *> params;
//     va_list args;
//     va_start(args, n);
//     for (int i = 0; i < n; i++) {
//         auto arg = va_arg(args, Var *);
//         DEBUG_ASSERT(arg);
//         DEBUG_ASSERT(arg->Type() >= VarType::Min && arg->Type() <= VarType::Max);
//         params.push_back(arg);
//     }
//     va_end(args);
//
//     ExpandVarList(params);
//
//     // it is a colon call, we need to get function from table
//     if (col_key) {
//         // now func must be table type
//         if (func->Type() != VarType::Table) {
//             ThrowFakeluaException(std::format("CallVar: colon func must be table type, but got {}", func->ToString()));
//         }
//         auto real_func = func->TableGet(*col_key);
//         // add the 1st self param
//         params.insert(params.begin(), func);
//         func = &real_func;
//     }
//
//     // func must be a string type
//     if (func->Type() != VarType::String && func->Type() != VarType::StringId) {
//         ThrowFakeluaException(std::format("CallVar: func must be string type, but got {}", func->ToString()));
//     }
//
//     // get function address
//     auto name = func->GetString();
//     auto function = s->get_vm().GetFunction(std::string(name->Str()));
//     if (!function) {
//         ThrowFakeluaException(std::format("CallVar: function {} not found", name->Str()));
//     }
//
//     // check params count
//     // if (!func->IsVariadic() && (int) params.size() != function->GetArgCount()) {
//     //     ThrowFakeluaException(
//     //             std::format("CallVar: function {} expect {} params, but got {}", name->str(), function->GetArgCount(), params.size()));
//     // }
//
//     // call function
//     DEBUG_ASSERT(function->get_addr());
//     auto func_addr = reinterpret_cast<Var *(*) (...)>(function->get_addr());
//     auto ret = CallVarFunc(func_addr, params);
//     DEBUG_ASSERT(ret);
//
//     return ret;
// }
//
// extern "C" __attribute__((used)) Var *TableIndexByVar(State *s, bool is_const, Var *table, Var *key) {
//     DEBUG_ASSERT(table);
//     DEBUG_ASSERT(table->Type() >= VarType::Min && table->Type() <= VarType::Max);
//     DEBUG_ASSERT(key);
//     DEBUG_ASSERT(key->Type() >= VarType::Min && key->Type() <= VarType::Max);
//     return nullptr;//&table->TableGet(*key);
// }
//
// extern "C" __attribute__((used)) Var *TableIndexByName(State *s, bool is_const, Var *table, const char *key, int len) {
//     DEBUG_ASSERT(table);
//     DEBUG_ASSERT(table->Type() >= VarType::Min && table->Type() <= VarType::Max);
//     DEBUG_ASSERT(key);
//     return nullptr;
// }
//
// extern "C" __attribute__((used)) Var *TableSet(State *s, bool is_const, Var *table, Var *key, Var *val) {
//     DEBUG_ASSERT(table);
//     DEBUG_ASSERT(table->Type() >= VarType::Min && table->Type() <= VarType::Max);
//     DEBUG_ASSERT(key);
//     DEBUG_ASSERT(key->Type() >= VarType::Min && key->Type() <= VarType::Max);
//     DEBUG_ASSERT(val);
//     DEBUG_ASSERT(val->Type() >= VarType::Min && val->Type() <= VarType::Max);
//     table->TableSet(s, *key, *val, false);
//     return table;
// }
//
// extern "C" __attribute__((used)) size_t TableSize(State *s, bool is_const, Var *table) {
//     DEBUG_ASSERT(table);
//     DEBUG_ASSERT(table->Type() >= VarType::Min && table->Type() <= VarType::Max);
//     return table->TableSize();
// }
//
// extern "C" __attribute__((used)) Var *TableKeyByPos(State *s, bool is_const, Var *table, size_t pos) {
//     DEBUG_ASSERT(table);
//     DEBUG_ASSERT(table->Type() >= VarType::Min && table->Type() <= VarType::Max);
//     DEBUG_ASSERT(pos < table->TableSize());
//     return nullptr;//&table->TableKeyAt(pos);
// }
//
// extern "C" __attribute__((used)) Var *TableValueByPos(State *s, bool is_const, Var *table, size_t pos) {
//     DEBUG_ASSERT(table);
//     DEBUG_ASSERT(table->Type() >= VarType::Min && table->Type() <= VarType::Max);
//     DEBUG_ASSERT(pos < table->TableSize());
//     return nullptr;//&table->TableValueAt(pos);
// }

}// namespace fakelua
