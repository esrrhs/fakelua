#pragma once

#include "util/exception.h"
#include "var/var_type.h"
#include <cstdio>
#include <string>
#include <string_view>

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

}// namespace fakelua
