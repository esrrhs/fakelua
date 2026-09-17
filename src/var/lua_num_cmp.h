#pragma once

// Lua 5.4.7 整数/浮点混合比较（lvm.c：l_intfitsf / luaV_flttointeger / LTintfloat 等）。
// |i| > 2^53 时不能把整数转成 double 再比，否则 maxinteger == maxinteger+0.0 会误判为真。

#include <climits>
#include <cmath>
#include <cstdint>
#include <limits>

namespace fakelua {

inline constexpr double kLuaMaxIntFitsF = 9007199254740992.0;// 2^53
inline constexpr double kLuaInt64FloatExcl = 9223372036854775808.0;// 2^63

enum class LuaF2I : int { Eq = 0, Floor = 1, Ceil = 2 };

inline bool LuaIntFitsFloat(int64_t i) {
    return (static_cast<uint64_t>(i) + 9007199254740992ull) <= 18014398509481984ull;
}

inline bool LuaFltToInt(double n, int64_t *p, LuaF2I mode) {
    double f = std::floor(n);
    if (n != f) {
        if (mode == LuaF2I::Eq) return false;
        if (mode == LuaF2I::Ceil) f += 1.0;
    }
    if (!std::isfinite(f)) return false;
    if (f < static_cast<double>(INT64_MIN) || f >= kLuaInt64FloatExcl) return false;
    *p = static_cast<int64_t>(f);
    return true;
}

inline bool LuaEqIntFloat(int64_t i, double f) {
    if (LuaIntFitsFloat(i)) return static_cast<double>(i) == f;
    int64_t fi = 0;
    return LuaFltToInt(f, &fi, LuaF2I::Eq) && fi == i;
}

inline bool LuaLtIntFloat(int64_t i, double f) {
    if (LuaIntFitsFloat(i)) return static_cast<double>(i) < f;
    int64_t fi = 0;
    if (LuaFltToInt(f, &fi, LuaF2I::Ceil)) return i < fi;
    return f > 0;
}

inline bool LuaLeIntFloat(int64_t i, double f) {
    if (LuaIntFitsFloat(i)) return static_cast<double>(i) <= f;
    int64_t fi = 0;
    if (LuaFltToInt(f, &fi, LuaF2I::Floor)) return i <= fi;
    return f > 0;
}

inline bool LuaLtFloatInt(double f, int64_t i) {
    if (LuaIntFitsFloat(i)) return f < static_cast<double>(i);
    int64_t fi = 0;
    if (LuaFltToInt(f, &fi, LuaF2I::Floor)) return fi < i;
    return f < 0;
}

inline bool LuaLeFloatInt(double f, int64_t i) {
    if (LuaIntFitsFloat(i)) return f <= static_cast<double>(i);
    int64_t fi = 0;
    if (LuaFltToInt(f, &fi, LuaF2I::Ceil)) return fi <= i;
    return f < 0;
}

}// namespace fakelua
