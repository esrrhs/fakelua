#pragma once

#include "state/state.h"
#include "var/var_closure.h"
#include <cmath>
#include <limits>
#include <sstream>
#include <string>

namespace fakelua {

// ─────────────────────────────────────────────────────────────────────────────
// Shared helpers for native library argument validation and error reporting.
// Used across native_math, native_table, native_utf8, native_string, native_io.
// ─────────────────────────────────────────────────────────────────────────────

// Safe double-to-string conversion. std::to_string has ambiguous overloads
// for double/long double on MinGW GCC 16.2.0, and libc++ uses Ryu Printf
// which produces "1" instead of "1.000000" for whole numbers. Use ::snprintf
// with %f for consistent output ("1.000000") across all platforms.
// Use ::snprintf (global namespace) because std::snprintf may not be in std::
// on MinGW even with __USE_MINGW_ANSI_STDIO=1.
inline std::string DoubleToString(double v) {
    char buf[64];
    ::snprintf(buf, sizeof(buf), "%f", v);
    return buf;
}

// Safe long-double-to-string conversion (same reason as DoubleToString).
inline std::string LongDoubleToString(long double v) {
    char buf[64];
    ::snprintf(buf, sizeof(buf), "%Lf", v);
    return buf;
}

// Throw a standardized "bad argument #N to 'fname' (expected)" exception.
[[noreturn]] inline void ThrowBadArgument(int argno, const char *fname, const char *expected) {
    std::string msg = std::string("bad argument #") + std::to_string(argno) + " to '" + fname + "' (" + expected + ")";
    ThrowFakeluaException(msg);
}

// Reject Bool/Table/Nil where a number is expected. Lua's luaL_checknumber also
// converts numeric strings, so plain String/StringId are left to the caller.
inline void CheckNumberArg(const CVar &a, int argno, const char *fname) {
    if (a.type_ == static_cast<int>(VarType::Bool) || a.type_ == static_cast<int>(VarType::Table) ||
        a.type_ == static_cast<int>(VarType::Nil)) {
        ThrowBadArgument(argno, fname, "number expected");
    }
}

// luaL_checkinteger：Int 直接过；Float 必须能无损落成整数；数字串按同样规则转换；
// 其它类型一律 "number expected"。
//
// 与 Lua 5.4 对齐：float 超出 int64 范围（|d| >= 2^63）时抛
// "number has no integer representation"，而不是 UB 的 static_cast。
inline int64_t CheckIntegerArg(const CVar &a, int argno, const char *fname) {
    if (a.type_ == static_cast<int>(VarType::Int)) {
        return a.data_.i;
    }
    // int64 能表示的 double 范围是 [-2^63, 2^63)。2^63 本身无法存入 int64，
    // 直接 static_cast 是 UB，必须先拦一刀。
    static constexpr double kInt64Limit = static_cast<double>(INT64_MAX) + 1.0;  // == 2^63
    if (a.type_ == static_cast<int>(VarType::Float)) {
        const double d = a.data_.f;
        if (!std::isfinite(d) || std::trunc(d) != d) {
            ThrowBadArgument(argno, fname, "number has no integer representation");
        }
        if (d < -kInt64Limit || d >= kInt64Limit) {
            ThrowBadArgument(argno, fname, "number has no integer representation");
        }
        return static_cast<int64_t>(d);
    }
    if (a.type_ == static_cast<int>(VarType::String) || a.type_ == static_cast<int>(VarType::StringId)) {
        const double d = inter::CVarToNumber(a, std::numeric_limits<double>::quiet_NaN());
        if (!std::isfinite(d)) {
            ThrowBadArgument(argno, fname, "number expected");
        }
        if (std::trunc(d) != d) {
            ThrowBadArgument(argno, fname, "number has no integer representation");
        }
        if (d < -kInt64Limit || d >= kInt64Limit) {
            ThrowBadArgument(argno, fname, "number has no integer representation");
        }
        return static_cast<int64_t>(d);
    }
    ThrowBadArgument(argno, fname, "number expected");
}

// Safe double → int64: false for NaN/Inf, non-integers, and values outside int64.
inline bool DoubleFitsInt64(double d, int64_t *out) {
    if (!std::isfinite(d) || std::trunc(d) != d) return false;
    static constexpr double kInt64Limit = static_cast<double>(INT64_MAX) + 1.0;
    if (d < -kInt64Limit || d >= kInt64Limit) return false;
    *out = static_cast<int64_t>(d);
    return true;
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
