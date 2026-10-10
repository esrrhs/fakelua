#include "native/math/native_math.h"
#include "native/native_common.h"
#include "util/number_util.h"
#include "var/var.h"
#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdlib>
#include <ctime>

namespace fakelua::math {

// Lua 5.4 pushnumint：floor/ceil/modf 的整数部分若能无损落进 int64 就返回
// integer，否则保留 float（inf/-inf/nan/超出 2^63 时不报错，仍是 float）。
static CVar PushNumInt(State *state, double d) {
    if (TryConvertDoubleToInt64(d).has_value()) {
        return inter::NativeToFakeluaInt(state, static_cast<int64_t>(d));
    }
    return inter::NativeToFakeluaFloat(state, d);
}

static int CompareIntDouble(int64_t integer, double floating) {
    constexpr double kInt64LowerBound = -9223372036854775808.0;
    constexpr double kInt64UpperBound = 9223372036854775808.0;
    if (std::isnan(floating)) return 0;
    if (floating >= kInt64UpperBound) return -1;
    if (floating < kInt64LowerBound) return 1;

    const int64_t truncated = static_cast<int64_t>(floating);
    if (integer < truncated) return -1;
    if (integer > truncated) return 1;
    const double truncated_as_double = static_cast<double>(truncated);
    if (floating > truncated_as_double) return -1;
    if (floating < truncated_as_double) return 1;
    return 0;
}

static int CompareNumbers(CVar lhs, double lhs_value, CVar rhs, double rhs_value) {
    const int int_type = static_cast<int>(VarType::Int);
    if (lhs.type_ == int_type && rhs.type_ == int_type) {
        if (lhs.data_.i < rhs.data_.i) return -1;
        if (lhs.data_.i > rhs.data_.i) return 1;
        return 0;
    }
    if (lhs.type_ == int_type) return CompareIntDouble(lhs.data_.i, rhs_value);
    if (rhs.type_ == int_type) return -CompareIntDouble(rhs.data_.i, lhs_value);
    if (lhs_value < rhs_value) return -1;
    if (lhs_value > rhs_value) return 1;
    return 0;
}

static double CheckMathNumberArg(const CVar &arg, int argno, const char *fname) {
    CheckNumberArg(arg, argno, fname);
    const double value = inter::CVarToNumber(arg, std::numeric_limits<double>::quiet_NaN());
    if ((arg.type_ == static_cast<int>(VarType::String) || arg.type_ == static_cast<int>(VarType::StringId)) &&
        std::isnan(value)) {
        ThrowBadArgument(argno, fname, "number expected");
    }
    return value;
}

static int64_t RandomInt64() {
    uint64_t value = 0;
    for (int i = 0; i < 5; ++i) {
        value = (value << 15) | (static_cast<uint64_t>(std::rand()) & 0x7fff);
    }
    return std::bit_cast<int64_t>(value);
}

// Use shared CheckNumberArg from native_common.h

void RegisterMathLibraryApi(State *s) {
    if (!s) return;

    RegisterNativeFunction(s, "math.abs", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckMathNumberArg(a0, 1, "math.abs");
        if (a0.type_ == static_cast<int>(VarType::Int)) {
            int64_t n = a0.data_.i;
            if (n < 0) n = static_cast<int64_t>(0ull - static_cast<uint64_t>(n));
            return inter::NativeToFakeluaInt(state, n);
        }
        if (a0.type_ == static_cast<int>(VarType::Float)) return inter::NativeToFakeluaFloat(state, std::abs(a0.data_.f));
        double f = inter::CVarToNumber(a0, std::numeric_limits<double>::quiet_NaN());
        if (!std::isnan(f)) {
            int64_t iv = 0;
            if (DoubleFitsInt64(f, &iv)) {
                if (iv < 0) iv = static_cast<int64_t>(0ull - static_cast<uint64_t>(iv));
                return inter::NativeToFakeluaInt(state, iv);
            }
            return inter::NativeToFakeluaFloat(state, std::abs(f));
        }
        return inter::NativeToFakeluaNil(state);
    });

    RegisterNativeFunction(s, "math.floor", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckMathNumberArg(a0, 1, "math.floor");
        if (a0.type_ == static_cast<int>(VarType::Int)) return a0;
        double f = inter::CVarToNumber(a0, std::numeric_limits<double>::quiet_NaN());
        return PushNumInt(state, std::floor(f));
    });

    RegisterNativeFunction(s, "math.ceil", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckMathNumberArg(a0, 1, "math.ceil");
        if (a0.type_ == static_cast<int>(VarType::Int)) return a0;
        double f = inter::CVarToNumber(a0, std::numeric_limits<double>::quiet_NaN());
        return PushNumInt(state, std::ceil(f));
    });

    RegisterNativeFunction(s, "math.max", 1, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) return inter::NativeToFakeluaNil(state);
        CVar max_cvar = inter::GetNativeArg(state, args, n, 0);
        double max_v = CheckMathNumberArg(max_cvar, 1, "math.max");
        for (int i = 1; i < n; ++i) {
            CVar arg_i = inter::GetNativeArg(state, args, n, i);
            double v_i = CheckMathNumberArg(arg_i, i + 1, "math.max");
            if (CompareNumbers(max_cvar, max_v, arg_i, v_i) < 0) {
                max_v = v_i;
                max_cvar = arg_i;
            }
        }
        if (max_cvar.type_ == static_cast<int>(VarType::String) || max_cvar.type_ == static_cast<int>(VarType::StringId)) {
            int64_t iv = 0;
            if (DoubleFitsInt64(max_v, &iv)) {
                return inter::NativeToFakeluaInt(state, iv);
            }
            return inter::NativeToFakeluaFloat(state, max_v);
        }
        return max_cvar;
    });

    RegisterNativeFunction(s, "math.min", 1, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) return inter::NativeToFakeluaNil(state);
        CVar min_cvar = inter::GetNativeArg(state, args, n, 0);
        double min_v = CheckMathNumberArg(min_cvar, 1, "math.min");
        for (int i = 1; i < n; ++i) {
            CVar arg_i = inter::GetNativeArg(state, args, n, i);
            double v_i = CheckMathNumberArg(arg_i, i + 1, "math.min");
            if (CompareNumbers(min_cvar, min_v, arg_i, v_i) > 0) {
                min_v = v_i;
                min_cvar = arg_i;
            }
        }
        if (min_cvar.type_ == static_cast<int>(VarType::String) || min_cvar.type_ == static_cast<int>(VarType::StringId)) {
            int64_t iv = 0;
            if (DoubleFitsInt64(min_v, &iv)) {
                return inter::NativeToFakeluaInt(state, iv);
            }
            return inter::NativeToFakeluaFloat(state, min_v);
        }
        return min_cvar;
    });

    RegisterNativeFunction(s, "math.sqrt", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckMathNumberArg(a0, 1, "math.sqrt");
        double v0 = inter::CVarToNumber(a0, 0.0);
        return inter::NativeToFakeluaFloat(state, std::sqrt(v0));
    });

    RegisterNativeFunction(s, "math.sin", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckMathNumberArg(a0, 1, "math.sin");
        double v0 = inter::CVarToNumber(a0, 0.0);
        return inter::NativeToFakeluaFloat(state, std::sin(v0));
    });

    RegisterNativeFunction(s, "math.cos", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckMathNumberArg(a0, 1, "math.cos");
        double v0 = inter::CVarToNumber(a0, 0.0);
        return inter::NativeToFakeluaFloat(state, std::cos(v0));
    });

    RegisterNativeFunction(s, "math.tan", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckMathNumberArg(a0, 1, "math.tan");
        double v0 = inter::CVarToNumber(a0, 0.0);
        return inter::NativeToFakeluaFloat(state, std::tan(v0));
    });

    RegisterNativeFunction(s, "math.pow", 2, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CVar a1 = inter::GetNativeArg(state, args, n, 1);
        CheckMathNumberArg(a0, 1, "math.pow");
        CheckMathNumberArg(a1, 2, "math.pow");
        double v0 = inter::CVarToNumber(a0, 0.0);
        double v1 = inter::CVarToNumber(a1, 0.0);
        return inter::NativeToFakeluaFloat(state, std::pow(v0, v1));
    });

    RegisterNativeFunction(s, "math.asin", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckMathNumberArg(a0, 1, "math.asin");
        double v0 = inter::CVarToNumber(a0, 0.0);
        return inter::NativeToFakeluaFloat(state, std::asin(v0));
    });

    RegisterNativeFunction(s, "math.acos", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckMathNumberArg(a0, 1, "math.acos");
        double v0 = inter::CVarToNumber(a0, 0.0);
        return inter::NativeToFakeluaFloat(state, std::acos(v0));
    });

    RegisterNativeFunction(s, "math.atan", 1, true, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckMathNumberArg(a0, 1, "math.atan");
        double v0 = inter::CVarToNumber(a0, 0.0);
        if (n >= 2) {
            CVar a1 = inter::GetNativeArg(state, args, n, 1);
            CheckMathNumberArg(a1, 2, "math.atan");
            double v1 = inter::CVarToNumber(a1, 0.0);
            return inter::NativeToFakeluaFloat(state, std::atan2(v0, v1));
        }
        return inter::NativeToFakeluaFloat(state, std::atan(v0));
    });

    RegisterNativeFunction(s, "math.atan2", 2, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CVar a1 = inter::GetNativeArg(state, args, n, 1);
        CheckMathNumberArg(a0, 1, "math.atan2");
        CheckMathNumberArg(a1, 2, "math.atan2");
        double v0 = inter::CVarToNumber(a0, 0.0);
        double v1 = inter::CVarToNumber(a1, 0.0);
        return inter::NativeToFakeluaFloat(state, std::atan2(v0, v1));
    });

    RegisterNativeFunction(s, "math.copysign", 2, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CVar a1 = inter::GetNativeArg(state, args, n, 1);
        CheckMathNumberArg(a0, 1, "math.copysign");
        CheckMathNumberArg(a1, 2, "math.copysign");
        double v0 = inter::CVarToNumber(a0, 0.0);
        double v1 = inter::CVarToNumber(a1, 0.0);
        return inter::NativeToFakeluaFloat(state, std::copysign(v0, v1));
    });

    RegisterNativeFunction(s, "math.exp", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckMathNumberArg(a0, 1, "math.exp");
        double v0 = inter::CVarToNumber(a0, 0.0);
        return inter::NativeToFakeluaFloat(state, std::exp(v0));
    });

    RegisterNativeFunction(s, "math.log", 1, true, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckMathNumberArg(a0, 1, "math.log");
        double v0 = inter::CVarToNumber(a0, 0.0);
        if (n >= 2) {
            CVar a1 = inter::GetNativeArg(state, args, n, 1);
            CheckMathNumberArg(a1, 2, "math.log");
            double base = inter::CVarToNumber(a1, 1.0);
            return inter::NativeToFakeluaFloat(state, std::log(v0) / std::log(base));
        }
        return inter::NativeToFakeluaFloat(state, std::log(v0));
    });

    RegisterNativeFunction(s, "math.log10", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckMathNumberArg(a0, 1, "math.log10");
        double v0 = inter::CVarToNumber(a0, 0.0);
        return inter::NativeToFakeluaFloat(state, std::log10(v0));
    });

    RegisterNativeFunction(s, "math.sinh", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckMathNumberArg(a0, 1, "math.sinh");
        double v0 = inter::CVarToNumber(a0, 0.0);
        return inter::NativeToFakeluaFloat(state, std::sinh(v0));
    });

    RegisterNativeFunction(s, "math.cosh", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckMathNumberArg(a0, 1, "math.cosh");
        double v0 = inter::CVarToNumber(a0, 0.0);
        return inter::NativeToFakeluaFloat(state, std::cosh(v0));
    });

    RegisterNativeFunction(s, "math.tanh", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckMathNumberArg(a0, 1, "math.tanh");
        double v0 = inter::CVarToNumber(a0, 0.0);
        return inter::NativeToFakeluaFloat(state, std::tanh(v0));
    });

    RegisterNativeFunction(s, "math.fmod", 2, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CVar a1 = inter::GetNativeArg(state, args, n, 1);
        CheckMathNumberArg(a0, 1, "math.fmod");
        CheckMathNumberArg(a1, 2, "math.fmod");
        // Lua 5.4：两个参数都是 integer 时走整数取模（C 的截断向零语义）
        if (a0.type_ == static_cast<int>(VarType::Int) && a1.type_ == static_cast<int>(VarType::Int)) {
            const int64_t b = a1.data_.i;
            if (b == 0) {
                ThrowFakeluaException("bad argument #2 to 'fmod' (zero)");
            }
            // mininteger % -1 在 C 里是 UB（结果溢出），Lua 直接规定为 0
            if (b == -1) {
                return inter::NativeToFakeluaInt(state, 0);
            }
            return inter::NativeToFakeluaInt(state, a0.data_.i % b);
        }
        // 任一参数是 float：结果恒为 float；浮点除零得到 NaN（不报错）
        double v0 = inter::CVarToNumber(a0, 0.0);
        double v1 = inter::CVarToNumber(a1, 0.0);
        return inter::NativeToFakeluaFloat(state, std::fmod(v0, v1));
    });

    RegisterNativeFunction(s, "math.ldexp", 2, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CVar a1 = inter::GetNativeArg(state, args, n, 1);
        CheckMathNumberArg(a0, 1, "math.ldexp");
        double v0 = inter::CVarToNumber(a0, 0.0);
        // 指数必须是整数；2^63 经 CVarToInteger 会变成 0（ldexp(x,0)），属错误结果。
        int64_t exp = CheckIntegerArg(a1, 2, "math.ldexp");
        const int exp_int = static_cast<int>(std::clamp<int64_t>(exp, std::numeric_limits<int>::min(), std::numeric_limits<int>::max()));
        return inter::NativeToFakeluaFloat(state, std::ldexp(v0, exp_int));
    });

    RegisterNativeFunction(s, "math.type", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        if (a0.type_ == static_cast<int>(VarType::Int)) return inter::NativeToFakeluaStringView(state, "integer");
        if (a0.type_ == static_cast<int>(VarType::Float)) return inter::NativeToFakeluaStringView(state, "float");
        return inter::NativeToFakeluaNil(state);
    });

    RegisterNativeFunction(s, "math.tointeger", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        // 标准 Lua：math.tointeger 的参数必须是 number，Bool/Table 不合法
        CheckNumberArg(a0, 1, "math.tointeger");
        if (a0.type_ == static_cast<int>(VarType::Int)) return a0;
        double f = inter::CVarToNumber(a0, std::numeric_limits<double>::quiet_NaN());
        int64_t iv = 0;
        if (DoubleFitsInt64(f, &iv)) {
            return inter::NativeToFakeluaInt(state, iv);
        }
        return inter::NativeToFakeluaNil(state);
    });

    RegisterNativeFunction(s, "math.ult", 2, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CVar a1 = inter::GetNativeArg(state, args, n, 1);
        uint64_t u0 = static_cast<uint64_t>(CheckIntegerArg(a0, 1, "math.ult"));
        uint64_t u1 = static_cast<uint64_t>(CheckIntegerArg(a1, 2, "math.ult"));
        return inter::NativeToFakeluaBool(state, u0 < u1);
    });

    RegisterNativeFunction(s, "math.deg", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckMathNumberArg(a0, 1, "math.deg");
        double v0 = inter::CVarToNumber(a0, 0.0);
        return inter::NativeToFakeluaFloat(state, v0 * (180.0 / 3.14159265358979323846));
    });

    RegisterNativeFunction(s, "math.rad", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckMathNumberArg(a0, 1, "math.rad");
        double v0 = inter::CVarToNumber(a0, 0.0);
        return inter::NativeToFakeluaFloat(state, v0 * (3.14159265358979323846 / 180.0));
    });

    RegisterNativeFunction(s, "math.random", 0, true, [](State *state, CVar *args, int n) -> CVar {
        if (n == 0) {
            double r = static_cast<double>(std::rand()) / (static_cast<double>(RAND_MAX) + 1.0);
            return inter::NativeToFakeluaFloat(state, r);
        } else if (n == 1) {
            CVar a0 = inter::GetNativeArg(state, args, n, 0);
            // 标准 Lua：math.random 要求 number 参数，Bool/Table/String/Nil 不合法
            const int64_t u = CheckIntegerArg(a0, 1, "math.random");
            if (u == 0) {
                // Lua 5.4：math.random(0) 特殊情况，返回全范围随机整数。
                return inter::NativeToFakeluaInt(state, RandomInt64());
            }
            if (u < 0) {
                // Lua 5.4：math.random(负数) 报 "interval is empty"
                ThrowFakeluaException("bad argument #1 to 'math.random' (interval is empty)");
            }
            int64_t r = 1 + (static_cast<int64_t>(std::rand()) % u);
            return inter::NativeToFakeluaInt(state, r);
        } else {
            CVar a0 = inter::GetNativeArg(state, args, n, 0);
            CVar a1 = inter::GetNativeArg(state, args, n, 1);
            // 标准 Lua：math.random 要求 number 参数，Bool/Table/String/Nil 不合法
            const int64_t l = CheckIntegerArg(a0, 1, "math.random");
            const int64_t u = CheckIntegerArg(a1, 2, "math.random");
            if (l > u) {
                // Lua 5.4：空区间报 "interval is empty"
                ThrowFakeluaException("bad argument #1 to 'math.random' (interval is empty)");
            }
            if (l == u) return inter::NativeToFakeluaInt(state, l);
            // 用无符号运算求 range，避免 math.random(0, INT64_MAX) 等有符号溢出（UB）。
            // 与 Lua 5.4 project() 思路一致：range 最大到 2^64（此时回绕为 0），
            // 回绕仅发生在 l=INT64_MIN, u=INT64_MAX 的极端情况，此时直接返回 l。
            uint64_t range = static_cast<uint64_t>(u) - static_cast<uint64_t>(l) + 1;
            if (range == 0) {
                return inter::NativeToFakeluaInt(state, RandomInt64());
            }
            uint64_t rv = (static_cast<uint64_t>(std::rand()) << 32) | static_cast<uint64_t>(std::rand());
            int64_t r = static_cast<int64_t>(static_cast<uint64_t>(l) + (rv % range));
            return inter::NativeToFakeluaInt(state, r);
        }
    });

    RegisterNativeFunction(s, "math.randomseed", 0, true, [](State *state, CVar *args, int n) -> CVar {
        unsigned int seed = static_cast<unsigned int>(std::time(nullptr));
        if (n >= 1) {
            CVar a0 = inter::GetNativeArg(state, args, n, 0);
            // 标准 Lua：math.randomseed 要求 number 参数，Bool/Table/String/Nil 不合法
            seed = static_cast<unsigned int>(CheckIntegerArg(a0, 1, "math.randomseed"));
        }
        std::srand(seed);
        return inter::NativeToFakeluaNil(state);
    });

    RegisterNativeFunction(s, "math.modf", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckMathNumberArg(a0, 1, "math.modf");
        if (a0.type_ == static_cast<int>(VarType::Int)) {
            CVar multi = inter::AllocMultiCVar(state, 2);
            inter::SetMultiCVarElement(multi, 0, a0);
            inter::SetMultiCVarElement(multi, 1, inter::NativeToFakeluaFloat(state, 0.0));
            return multi;
        }
        double val = inter::CVarToNumber(a0, 0.0);
        // 整数部分向零取整（负数用 ceil），再按 pushnumint 决定 int/float
        double iptr = (val < 0) ? std::ceil(val) : std::floor(val);
        // 无小数部分（整数值或 inf）时小数部分是 +0.0
        double frac = (val == iptr) ? 0.0 : val - iptr;
        CVar multi = inter::AllocMultiCVar(state, 2);
        inter::SetMultiCVarElement(multi, 0, PushNumInt(state, iptr));
        inter::SetMultiCVarElement(multi, 1, inter::NativeToFakeluaFloat(state, frac));
        return multi;
    });

    RegisterNativeFunction(s, "math.frexp", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckMathNumberArg(a0, 1, "math.frexp");
        double val = inter::CVarToNumber(a0, 0.0);
        int exp_val = 0;
        double frac = std::frexp(val, &exp_val);
        CVar multi = inter::AllocMultiCVar(state, 2);
        inter::SetMultiCVarElement(multi, 0, inter::NativeToFakeluaFloat(state, frac));
        inter::SetMultiCVarElement(multi, 1, inter::NativeToFakeluaInt(state, exp_val));
        return multi;
    });

    RegisterNativeFunction(s, "math.erf", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckMathNumberArg(a0, 1, "math.erf");
        double v0 = inter::CVarToNumber(a0, 0.0);
        return inter::NativeToFakeluaFloat(state, std::erf(v0));
    });

    RegisterNativeFunction(s, "math.erfc", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckMathNumberArg(a0, 1, "math.erfc");
        double v0 = inter::CVarToNumber(a0, 0.0);
        return inter::NativeToFakeluaFloat(state, std::erfc(v0));
    });

    RegisterNativeFunction(s, "math.gamma", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckMathNumberArg(a0, 1, "math.gamma");
        double v0 = inter::CVarToNumber(a0, 0.0);
        return inter::NativeToFakeluaFloat(state, std::tgamma(v0));
    });

    RegisterNativeFunction(s, "math.lgamma", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckMathNumberArg(a0, 1, "math.lgamma");
        double v0 = inter::CVarToNumber(a0, 0.0);
        return inter::NativeToFakeluaFloat(state, std::lgamma(v0));
    });

    // math.clamp(x, lo, hi) — integers stay integers when all three are Int
    RegisterNativeFunction(s, "math.clamp", 3, false, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) ThrowBadArgument(1, "math.clamp", "number expected");
        if (n < 2) ThrowBadArgument(2, "math.clamp", "number expected");
        if (n < 3) ThrowBadArgument(3, "math.clamp", "number expected");
        CVar ax = inter::GetNativeArg(state, args, n, 0);
        CVar alo = inter::GetNativeArg(state, args, n, 1);
        CVar ahi = inter::GetNativeArg(state, args, n, 2);
        CheckMathNumberArg(ax, 1, "math.clamp");
        CheckMathNumberArg(alo, 2, "math.clamp");
        CheckMathNumberArg(ahi, 3, "math.clamp");
        if (ax.type_ == static_cast<int>(VarType::Int) && alo.type_ == static_cast<int>(VarType::Int) &&
            ahi.type_ == static_cast<int>(VarType::Int)) {
            if (alo.data_.i > ahi.data_.i) {
                ThrowBadArgument(2, "math.clamp", "low bound must not exceed high bound");
            }
            return inter::NativeToFakeluaInt(state, std::clamp(ax.data_.i, alo.data_.i, ahi.data_.i));
        }
        double x = inter::CVarToNumber(ax, 0.0);
        double lo = inter::CVarToNumber(alo, 0.0);
        double hi = inter::CVarToNumber(ahi, 0.0);
        if (lo > hi) {
            ThrowBadArgument(2, "math.clamp", "low bound must not exceed high bound");
        }
        return inter::NativeToFakeluaFloat(state, std::clamp(x, lo, hi));
    });
}

}// namespace fakelua::math
