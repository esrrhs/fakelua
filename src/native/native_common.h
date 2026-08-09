#pragma once

#include "state/state.h"
#include "var/var_closure.h"
#include <string>

namespace fakelua {

// ─────────────────────────────────────────────────────────────────────────────
// Shared helpers for native library argument validation and error reporting.
// Used across native_math, native_table, native_utf8, native_string, native_io.
// ─────────────────────────────────────────────────────────────────────────────

// Throw a standardized "bad argument #N to 'fname' (expected)" exception.
inline void ThrowBadArgument(int argno, const char *fname, const char *expected) {
    std::string msg = std::string("bad argument #") + std::to_string(argno) + " to '" + fname + "' (" + expected + ")";
    ThrowFakeluaException(msg);
}

// Reject Bool/Table where a number is expected. Standard Lua 5.3: luaL_checknumber
// converts numeric strings to numbers, so we allow strings; only Bool/Table are invalid.
inline void CheckNumberArg(const CVar &a, int argno, const char *fname) {
    if (a.type_ == static_cast<int>(VarType::Bool) || a.type_ == static_cast<int>(VarType::Table)) {
        ThrowBadArgument(argno, fname, "number expected");
    }
}

// Reject Bool/Table/Nil where a string is expected. Standard Lua 5.3: luaL_checkstring
// converts numbers to strings, so we allow Int/Float; Bool/Table/Nil are invalid.
inline void CheckStringArg(const CVar &a, int argno, const char *fname) {
    if (a.type_ == static_cast<int>(VarType::Bool) || a.type_ == static_cast<int>(VarType::Table) ||
        a.type_ == static_cast<int>(VarType::Nil)) {
        ThrowBadArgument(argno, fname, "string expected");
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Iterator closure construction helper.
//
// Standard pattern used by pairs/ipairs (basic), gmatch (string), file:lines (io):
//   - upvalue 0: State* (for allocating return values)
//   - upvalue 1: opaque iterator state pointer (type-erased)
//   - func_ptr:  the native C function implementing the iterator
//
// The closure is allocated from the state's non-temp arena and is valid for the
// current frame. Returns a CVar of type Closure ready to be returned to Lua.
// ─────────────────────────────────────────────────────────────────────────────
inline CVar MakeIteratorClosure(State *state, void *func_ptr, void *iter_state) {
    auto &alloc = state->GetHeap().GetAllocator(false /* temp */);

    // upvalue 0: State*
    auto *uv0 = static_cast<CVar *>(alloc.Alloc(sizeof(CVar)));
    uv0->type_ = static_cast<int>(VarType::Int);
    uv0->flag_ = 0;
    uv0->data_.i = reinterpret_cast<int64_t>(state);

    // upvalue 1: iterator state (type-erased pointer)
    auto *uv1 = static_cast<CVar *>(alloc.Alloc(sizeof(CVar)));
    uv1->type_ = static_cast<int>(VarType::Int);
    uv1->flag_ = 0;
    uv1->data_.i = reinterpret_cast<int64_t>(iter_state);

    // Allocate the closure: header + 2 upvalue slots
    auto *cl = static_cast<VarClosure *>(alloc.Alloc(sizeof(VarClosure) + 2 * sizeof(CVar *)));
    cl->func_ptr = func_ptr;
    cl->upvalue_count = 2;
    cl->expected_arg_count = 2;
    cl->is_vararg = false;
    cl->code_str = nullptr;
    cl->upvalues[0] = uv0;
    cl->upvalues[1] = uv1;

    CVar res{};
    res.type_ = static_cast<int>(VarType::Closure);
    res.flag_ = 0;
    res.data_.cl = cl;
    return res;
}

}// namespace fakelua
