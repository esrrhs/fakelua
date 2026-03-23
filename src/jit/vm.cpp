#include "vm.h"
#include "fakelua.h"
#include "state/State.h"
#include "util/common.h"
#include "var/var_table.h"

namespace fakelua {

static Var *alloc_val_helper(FakeluaState *s, GccJitHandle *h, bool is_const) {
    DEBUG_ASSERT(s);
    DEBUG_ASSERT(h);
    return nullptr;// TODO
}

// Expand the list of expressions that may contain variable parameters.
// eg: function test() return 1,2,3 end
// 1,2,test() => 1,2,1,2,3
// 1,2,test(),3 => 1,2,1,3
static void ExpandVarList(std::vector<Var *> &params) {
    for (size_t i = 0; i < params.size(); i++) {
        const auto param = params[i];
        DEBUG_ASSERT(param);
        DEBUG_ASSERT(param->Type() >= VarType::Min && param->Type() <= VarType::Max);
        // if (param->IsVariadic()) {
        //     DEBUG_ASSERT(param->Type() == VarType::Table);
        //     const auto table = param->GetTable();
        //     if (i == params.size() - 1) {
        //         params.pop_back();
        //         for (int j = 1; j <= (int) table->size(); j++) {
        //             var tmp;
        //             tmp.SetInt(j);
        //             auto value = table->get(tmp);
        //             params.push_back(&value);
        //         }
        //     } else {
        //         var tmp;
        //         tmp.SetInt(1);
        //         const auto value = table->get(tmp);
        //         params[i] = (var *) &value;
        //     }
        // }
    }
}

static void ExpandVarList(std::vector<Var> &params) {
    for (size_t i = 0; i < params.size(); i++) {
        const auto param = params[i];
        DEBUG_ASSERT(param.Type() >= VarType::Min && param.Type() <= VarType::Max);
        // if (param.IsVariadic()) {
        //     DEBUG_ASSERT(param.Type() == VarType::Table);
        //     const auto table = param.GetTable();
        //     if (i == params.size() - 1) {
        //         params.pop_back();
        //         for (int j = 1; j <= static_cast<int>(table->size()); j++) {
        //             var tmp;
        //             tmp.SetInt(j);
        //             auto value = table->get(tmp);
        //             params.push_back(value);
        //         }
        //     } else {
        //         var tmp;
        //         tmp.SetInt(1);
        //         const auto value = table->get(tmp);
        //         params[i] = value;
        //     }
        // }
    }
}

extern "C" __attribute__((used)) Var *NewVarTable(FakeluaState *s, GccJitHandle *h, bool is_const, int n, ...) {
    DEBUG_ASSERT(n % 2 == 0);
    std::vector<Var *> keys;
    std::vector<Var *> values;
    va_list args;
    va_start(args, n);
    for (int i = 0; i < n; i += 2) {
        auto arg = va_arg(args, Var *);
        DEBUG_ASSERT(!arg || (arg->Type() >= VarType::Min && arg->Type() <= VarType::Max));
        keys.push_back(arg);
        arg = va_arg(args, Var *);
        DEBUG_ASSERT(arg->Type() >= VarType::Min && arg->Type() <= VarType::Max);
        values.push_back(arg);
    }
    va_end(args);

    auto ret = alloc_val_helper(s, h, is_const);
    ret->SetTable(s);

    ExpandVarList(values);

    int index = 1;
    for (size_t i = 0; i < values.size(); i++) {
        auto k = i < keys.size() ? keys[i] : nullptr;
        if (!k) {
            k = alloc_val_helper(s, h, is_const);
            k->SetInt(index);
            index++;
        }
        auto v = values[i];
        ret->GetTable()->Set(*k, *v, false);
    }
    return ret;
}

extern "C" __attribute__((used)) Var WrapReturnVar(FakeluaState *s, bool is_const, int n, ...) {
    DEBUG_ASSERT(n >= 0);
    std::vector<Var> params;
    va_list args;
    va_start(args, n);
    for (int i = 0; i < n; i++) {
        auto arg = va_arg(args, Var);
        DEBUG_ASSERT(arg.Type() >= VarType::Min && arg.Type() <= VarType::Max);
        params.push_back(arg);
    }
    va_end(args);

    Var ret;
    ret.SetTable(s);

    ExpandVarList(params);

    // push to ret
    for (size_t i = 0; i < params.size(); i++) {
        auto arg = params[i];
        Var k;
        k.SetInt(static_cast<int64_t>(i) + 1);
        ret.GetTable()->Set(k, arg, true);
    }
    return ret;
}

extern "C" __attribute__((used)) void AssignVar(FakeluaState *s, GccJitHandle *h, bool is_const, int left_n, int right_n, ...) {
    DEBUG_ASSERT(left_n >= 0);
    DEBUG_ASSERT(right_n >= 0);
    std::vector<Var **> left;
    std::vector<Var *> right;
    va_list args;
    va_start(args, right_n);
    for (int i = 0; i < left_n; i++) {
        auto dst = va_arg(args, Var **);
        DEBUG_ASSERT(dst);
        DEBUG_ASSERT(!*dst || (*dst && (*dst)->Type() >= VarType::Min && (*dst)->Type() <= VarType::Max));
        left.push_back(dst);
    }
    for (int i = 0; i < right_n; i++) {
        auto arg = va_arg(args, Var *);
        DEBUG_ASSERT(arg);
        DEBUG_ASSERT(arg->Type() >= VarType::Min && arg->Type() <= VarType::Max);
        right.push_back(arg);
    }
    va_end(args);

    ExpandVarList(right);

    // local a, b, c = x, y, z
    for (size_t i = 0; i < left.size(); i++) {
        auto dst = left[i];
        if (i < right.size()) {
            *dst = right[i];
        } else {
            break;
        }
    }
}

extern "C" __attribute__((used)) Var *BinopPlus(FakeluaState *s, GccJitHandle *h, bool is_const, Var *l, Var *r) {
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->Type() >= VarType::Min && l->Type() <= VarType::Max);
    DEBUG_ASSERT(r->Type() >= VarType::Min && r->Type() <= VarType::Max);
    auto ret = alloc_val_helper(s, h, is_const);
    l->Plus(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) Var *BinopMinus(FakeluaState *s, GccJitHandle *h, bool is_const, Var *l, Var *r) {
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->Type() >= VarType::Min && l->Type() <= VarType::Max);
    DEBUG_ASSERT(r->Type() >= VarType::Min && r->Type() <= VarType::Max);
    auto ret = alloc_val_helper(s, h, is_const);
    l->Minus(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) Var *BinopStar(FakeluaState *s, GccJitHandle *h, bool is_const, Var *l, Var *r) {
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->Type() >= VarType::Min && l->Type() <= VarType::Max);
    DEBUG_ASSERT(r->Type() >= VarType::Min && r->Type() <= VarType::Max);
    auto ret = alloc_val_helper(s, h, is_const);
    l->Star(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) Var *BinopSlash(FakeluaState *s, GccJitHandle *h, bool is_const, Var *l, Var *r) {
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->Type() >= VarType::Min && l->Type() <= VarType::Max);
    DEBUG_ASSERT(r->Type() >= VarType::Min && r->Type() <= VarType::Max);
    auto ret = alloc_val_helper(s, h, is_const);
    l->Slash(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) Var *BinopDoubleSlash(FakeluaState *s, GccJitHandle *h, bool is_const, Var *l, Var *r) {
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->Type() >= VarType::Min && l->Type() <= VarType::Max);
    DEBUG_ASSERT(r->Type() >= VarType::Min && r->Type() <= VarType::Max);
    auto ret = alloc_val_helper(s, h, is_const);
    l->DoubleSlash(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) Var *BinopPow(FakeluaState *s, GccJitHandle *h, bool is_const, Var *l, Var *r) {
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->Type() >= VarType::Min && l->Type() <= VarType::Max);
    DEBUG_ASSERT(r->Type() >= VarType::Min && r->Type() <= VarType::Max);
    auto ret = alloc_val_helper(s, h, is_const);
    l->Pow(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) Var *BinopMod(FakeluaState *s, GccJitHandle *h, bool is_const, Var *l, Var *r) {
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->Type() >= VarType::Min && l->Type() <= VarType::Max);
    DEBUG_ASSERT(r->Type() >= VarType::Min && r->Type() <= VarType::Max);
    auto ret = alloc_val_helper(s, h, is_const);
    l->Mod(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) Var *BinopBitand(FakeluaState *s, GccJitHandle *h, bool is_const, Var *l, Var *r) {
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->Type() >= VarType::Min && l->Type() <= VarType::Max);
    DEBUG_ASSERT(r->Type() >= VarType::Min && r->Type() <= VarType::Max);
    auto ret = alloc_val_helper(s, h, is_const);
    l->Bitand(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) Var *BinopXor(FakeluaState *s, GccJitHandle *h, bool is_const, Var *l, Var *r) {
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->Type() >= VarType::Min && l->Type() <= VarType::Max);
    DEBUG_ASSERT(r->Type() >= VarType::Min && r->Type() <= VarType::Max);
    auto ret = alloc_val_helper(s, h, is_const);
    l->Xor(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) Var *BinopBitor(FakeluaState *s, GccJitHandle *h, bool is_const, Var *l, Var *r) {
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->Type() >= VarType::Min && l->Type() <= VarType::Max);
    DEBUG_ASSERT(r->Type() >= VarType::Min && r->Type() <= VarType::Max);
    auto ret = alloc_val_helper(s, h, is_const);
    l->Bitor(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) Var *BinopRightShift(FakeluaState *s, GccJitHandle *h, bool is_const, Var *l, Var *r) {
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->Type() >= VarType::Min && l->Type() <= VarType::Max);
    DEBUG_ASSERT(r->Type() >= VarType::Min && r->Type() <= VarType::Max);
    auto ret = alloc_val_helper(s, h, is_const);
    l->RightShift(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) Var *BinopLeftShift(FakeluaState *s, GccJitHandle *h, bool is_const, Var *l, Var *r) {
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->Type() >= VarType::Min && l->Type() <= VarType::Max);
    DEBUG_ASSERT(r->Type() >= VarType::Min && r->Type() <= VarType::Max);
    auto ret = alloc_val_helper(s, h, is_const);
    l->LeftShift(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) Var *BinopConcat(FakeluaState *s, GccJitHandle *h, bool is_const, Var *l, Var *r) {
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->Type() >= VarType::Min && l->Type() <= VarType::Max);
    DEBUG_ASSERT(r->Type() >= VarType::Min && r->Type() <= VarType::Max);
    auto ret = alloc_val_helper(s, h, is_const);
    l->Concat(s, *r, *ret);
    return ret;
}

extern "C" __attribute__((used)) Var *BinopLess(FakeluaState *s, GccJitHandle *h, bool is_const, Var *l, Var *r) {
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->Type() >= VarType::Min && l->Type() <= VarType::Max);
    DEBUG_ASSERT(r->Type() >= VarType::Min && r->Type() <= VarType::Max);
    auto ret = alloc_val_helper(s, h, is_const);
    l->Less(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) Var *BinopLessEqual(FakeluaState *s, GccJitHandle *h, bool is_const, Var *l, Var *r) {
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->Type() >= VarType::Min && l->Type() <= VarType::Max);
    DEBUG_ASSERT(r->Type() >= VarType::Min && r->Type() <= VarType::Max);
    auto ret = alloc_val_helper(s, h, is_const);
    l->LessEqual(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) Var *BinopMore(FakeluaState *s, GccJitHandle *h, bool is_const, Var *l, Var *r) {
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->Type() >= VarType::Min && l->Type() <= VarType::Max);
    DEBUG_ASSERT(r->Type() >= VarType::Min && r->Type() <= VarType::Max);
    auto ret = alloc_val_helper(s, h, is_const);
    l->More(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) Var *BinopMoreEqual(FakeluaState *s, GccJitHandle *h, bool is_const, Var *l, Var *r) {
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->Type() >= VarType::Min && l->Type() <= VarType::Max);
    DEBUG_ASSERT(r->Type() >= VarType::Min && r->Type() <= VarType::Max);
    auto ret = alloc_val_helper(s, h, is_const);
    l->MoreEqual(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) Var *BinopEqual(FakeluaState *s, GccJitHandle *h, bool is_const, Var *l, Var *r) {
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->Type() >= VarType::Min && l->Type() <= VarType::Max);
    DEBUG_ASSERT(r->Type() >= VarType::Min && r->Type() <= VarType::Max);
    auto ret = alloc_val_helper(s, h, is_const);
    l->Equal(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) Var *BinopNotEqual(FakeluaState *s, GccJitHandle *h, bool is_const, Var *l, Var *r) {
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->Type() >= VarType::Min && l->Type() <= VarType::Max);
    DEBUG_ASSERT(r->Type() >= VarType::Min && r->Type() <= VarType::Max);
    auto ret = alloc_val_helper(s, h, is_const);
    l->NotEqual(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) bool TestVar(FakeluaState *s, GccJitHandle *h, bool is_const, Var *v) {
    DEBUG_ASSERT(v);
    DEBUG_ASSERT(v->Type() >= VarType::Min && v->Type() <= VarType::Max);
    return v->TestTrue();
}

extern "C" __attribute__((used)) bool TestNotVar(FakeluaState *s, GccJitHandle *h, bool is_const, Var *v) {
    DEBUG_ASSERT(v);
    DEBUG_ASSERT(v->Type() >= VarType::Min && v->Type() <= VarType::Max);
    return !v->TestTrue();
}

extern "C" __attribute__((used)) Var *UnopMinus(FakeluaState *s, GccJitHandle *h, bool is_const, Var *r) {
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(r->Type() >= VarType::Min && r->Type() <= VarType::Max);
    auto ret = alloc_val_helper(s, h, is_const);
    r->UnopMinus(*ret);
    return ret;
}

extern "C" __attribute__((used)) Var *UnopNot(FakeluaState *s, GccJitHandle *h, bool is_const, Var *r) {
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(r->Type() >= VarType::Min && r->Type() <= VarType::Max);
    auto ret = alloc_val_helper(s, h, is_const);
    r->UnopNot(*ret);
    return ret;
}

extern "C" __attribute__((used)) Var *UnopNumberSign(FakeluaState *s, GccJitHandle *h, bool is_const, Var *r) {
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(r->Type() >= VarType::Min && r->Type() <= VarType::Max);
    auto ret = alloc_val_helper(s, h, is_const);
    r->UnopNumberSign(*ret);
    return ret;
}

extern "C" __attribute__((used)) Var *UnopBitnot(FakeluaState *s, GccJitHandle *h, bool is_const, Var *r) {
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(r->Type() >= VarType::Min && r->Type() <= VarType::Max);
    auto ret = alloc_val_helper(s, h, is_const);
    r->UnopBitnot(*ret);
    return ret;
}

extern "C" __attribute__((used)) Var *CallVar(FakeluaState *s, GccJitHandle *h, bool is_const, Var *func, Var *col_key, int n, ...) {
    DEBUG_ASSERT(func);
    DEBUG_ASSERT(func->Type() >= VarType::Min && func->Type() <= VarType::Max);
    DEBUG_ASSERT(n >= 0);

    // prepare params
    std::vector<Var *> params;
    va_list args;
    va_start(args, n);
    for (int i = 0; i < n; i++) {
        auto arg = va_arg(args, Var *);
        DEBUG_ASSERT(arg);
        DEBUG_ASSERT(arg->Type() >= VarType::Min && arg->Type() <= VarType::Max);
        params.push_back(arg);
    }
    va_end(args);

    ExpandVarList(params);

    // it is a colon call, we need to get function from table
    if (col_key) {
        // now func must be table type
        if (func->Type() != VarType::Table) {
            ThrowFakeluaException(std::format("CallVar: colon func must be table type, but got {}", func->ToString()));
        }
        auto real_func = func->TableGet(*col_key);
        // add the 1st self param
        params.insert(params.begin(), func);
        func = &real_func;
    }

    // func must be a string type
    if (func->Type() != VarType::String) {
        ThrowFakeluaException(std::format("CallVar: func must be string type, but got {}", func->ToString()));
    }

    // get function address
    auto name = func->GetString();
    auto function = dynamic_cast<State *>(s)->get_vm().GetFunction(std::string(name->Str()));
    if (!function) {
        ThrowFakeluaException(std::format("CallVar: function {} not found", name->Str()));
    }

    // check params count
    // if (!func->IsVariadic() && (int) params.size() != function->GetArgCount()) {
    //     ThrowFakeluaException(
    //             std::format("CallVar: function {} expect {} params, but got {}", name->str(), function->GetArgCount(), params.size()));
    // }

    // call function
    DEBUG_ASSERT(function->get_addr());
    auto func_addr = reinterpret_cast<Var *(*) (...)>(function->get_addr());
    auto ret = CallVarFunc(func_addr, params);
    DEBUG_ASSERT(ret);

    return ret;
}

extern "C" __attribute__((used)) Var *TableIndexByVar(FakeluaState *s, GccJitHandle *h, bool is_const, Var *table, Var *key) {
    DEBUG_ASSERT(table);
    DEBUG_ASSERT(table->Type() >= VarType::Min && table->Type() <= VarType::Max);
    DEBUG_ASSERT(key);
    DEBUG_ASSERT(key->Type() >= VarType::Min && key->Type() <= VarType::Max);
    return nullptr;//&table->TableGet(*key);
}

extern "C" __attribute__((used)) Var *TableSet(FakeluaState *s, GccJitHandle *h, bool is_const, Var *table, Var *key, Var *val) {
    DEBUG_ASSERT(table);
    DEBUG_ASSERT(table->Type() >= VarType::Min && table->Type() <= VarType::Max);
    DEBUG_ASSERT(key);
    DEBUG_ASSERT(key->Type() >= VarType::Min && key->Type() <= VarType::Max);
    DEBUG_ASSERT(val);
    DEBUG_ASSERT(val->Type() >= VarType::Min && val->Type() <= VarType::Max);
    table->TableSet(*key, *val, false);
    return table;
}

extern "C" __attribute__((used)) size_t TableSize(FakeluaState *s, GccJitHandle *h, bool is_const, Var *table) {
    DEBUG_ASSERT(table);
    DEBUG_ASSERT(table->Type() >= VarType::Min && table->Type() <= VarType::Max);
    return table->TableSize();
}

extern "C" __attribute__((used)) Var *TableKeyByPos(FakeluaState *s, GccJitHandle *h, bool is_const, Var *table, size_t pos) {
    DEBUG_ASSERT(table);
    DEBUG_ASSERT(table->Type() >= VarType::Min && table->Type() <= VarType::Max);
    DEBUG_ASSERT(pos < table->TableSize());
    return nullptr;//&table->TableKeyAt(pos);
}

extern "C" __attribute__((used)) Var *TableValueByPos(FakeluaState *s, GccJitHandle *h, bool is_const, Var *table, size_t pos) {
    DEBUG_ASSERT(table);
    DEBUG_ASSERT(table->Type() >= VarType::Min && table->Type() <= VarType::Max);
    DEBUG_ASSERT(pos < table->TableSize());
    return nullptr;//&table->TableValueAt(pos);
}

}// namespace fakelua
