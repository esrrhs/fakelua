#include "native/native_math.h"
#include "native/native_common.h"
#include "var/var.h"
#include <cmath>
#include <cstdlib>
#include <ctime>

namespace fakelua {

// Use shared CheckNumberArg from native_common.h

void RegisterMathLibraryApi(State *s) {
    if (!s) return;

    RegisterNativeFunction(s, "math.abs", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckNumberArg(a0, 1, "math.abs");
        if (a0.type_ == static_cast<int>(VarType::Int)) {
            // std::abs(INT64_MIN) 是 UB（绝对值无法存入 int64）。
            // 与 Lua 5.4 对齐：返回 float 9.2233720368548e+18。
            if (a0.data_.i == INT64_MIN) {
                return inter::NativeToFakeluaFloat(state, static_cast<double>(a0.data_.i) * -1.0);
            }
            return inter::NativeToFakeluaInt(state, std::abs(a0.data_.i));
        }
        if (a0.type_ == static_cast<int>(VarType::Float)) return inter::NativeToFakeluaFloat(state, std::abs(a0.data_.f));
        double f = inter::CVarToNumber(a0, std::numeric_limits<double>::quiet_NaN());
        if (!std::isnan(f)) {
            if (static_cast<double>(static_cast<int64_t>(f)) == f) {
                return inter::NativeToFakeluaInt(state, std::abs(static_cast<int64_t>(f)));
            }
            return inter::NativeToFakeluaFloat(state, std::abs(f));
        }
        return inter::NativeToFakeluaNil(state);
    });

    RegisterNativeFunction(s, "math.floor", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckNumberArg(a0, 1, "math.floor");
        if (a0.type_ == static_cast<int>(VarType::Int)) return a0;
        if (a0.type_ == static_cast<int>(VarType::Float)) return inter::NativeToFakeluaFloat(state, std::floor(a0.data_.f));
        double f = inter::CVarToNumber(a0, std::numeric_limits<double>::quiet_NaN());
        if (!std::isnan(f)) return inter::NativeToFakeluaFloat(state, std::floor(f));
        return inter::NativeToFakeluaNil(state);
    });

    RegisterNativeFunction(s, "math.ceil", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckNumberArg(a0, 1, "math.ceil");
        if (a0.type_ == static_cast<int>(VarType::Int)) return a0;
        if (a0.type_ == static_cast<int>(VarType::Float)) return inter::NativeToFakeluaFloat(state, std::ceil(a0.data_.f));
        double f = inter::CVarToNumber(a0, std::numeric_limits<double>::quiet_NaN());
        if (!std::isnan(f)) return inter::NativeToFakeluaFloat(state, std::ceil(f));
        return inter::NativeToFakeluaNil(state);
    });

    RegisterNativeFunction(s, "math.max", 1, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) return inter::NativeToFakeluaNil(state);
        CVar max_cvar = inter::GetNativeArg(state, args, n, 0);
        CheckNumberArg(max_cvar, 1, "math.max");
        double max_v = inter::CVarToNumber(max_cvar, -std::numeric_limits<double>::infinity());
        for (int i = 1; i < n; ++i) {
            CVar arg_i = inter::GetNativeArg(state, args, n, i);
            CheckNumberArg(arg_i, i + 1, "math.max");
            double v_i = inter::CVarToNumber(arg_i, -std::numeric_limits<double>::infinity());
            if (v_i > max_v) {
                max_v = v_i;
                max_cvar = arg_i;
            }
        }
        if (max_cvar.type_ == static_cast<int>(VarType::String) || max_cvar.type_ == static_cast<int>(VarType::StringId)) {
            if (static_cast<double>(static_cast<int64_t>(max_v)) == max_v) {
                return inter::NativeToFakeluaInt(state, static_cast<int64_t>(max_v));
            }
            return inter::NativeToFakeluaFloat(state, max_v);
        }
        return max_cvar;
    });

    RegisterNativeFunction(s, "math.min", 1, true, [](State *state, CVar *args, int n) -> CVar {
        if (n < 1) return inter::NativeToFakeluaNil(state);
        CVar min_cvar = inter::GetNativeArg(state, args, n, 0);
        CheckNumberArg(min_cvar, 1, "math.min");
        double min_v = inter::CVarToNumber(min_cvar, std::numeric_limits<double>::infinity());
        for (int i = 1; i < n; ++i) {
            CVar arg_i = inter::GetNativeArg(state, args, n, i);
            CheckNumberArg(arg_i, i + 1, "math.min");
            double v_i = inter::CVarToNumber(arg_i, std::numeric_limits<double>::infinity());
            if (v_i < min_v) {
                min_v = v_i;
                min_cvar = arg_i;
            }
        }
        if (min_cvar.type_ == static_cast<int>(VarType::String) || min_cvar.type_ == static_cast<int>(VarType::StringId)) {
            if (static_cast<double>(static_cast<int64_t>(min_v)) == min_v) {
                return inter::NativeToFakeluaInt(state, static_cast<int64_t>(min_v));
            }
            return inter::NativeToFakeluaFloat(state, min_v);
        }
        return min_cvar;
    });

    RegisterNativeFunction(s, "math.sqrt", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckNumberArg(a0, 1, "math.sqrt");
        double v0 = inter::CVarToNumber(a0, 0.0);
        return inter::NativeToFakeluaFloat(state, std::sqrt(v0));
    });

    RegisterNativeFunction(s, "math.sin", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckNumberArg(a0, 1, "math.sin");
        double v0 = inter::CVarToNumber(a0, 0.0);
        return inter::NativeToFakeluaFloat(state, std::sin(v0));
    });

    RegisterNativeFunction(s, "math.cos", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckNumberArg(a0, 1, "math.cos");
        double v0 = inter::CVarToNumber(a0, 0.0);
        return inter::NativeToFakeluaFloat(state, std::cos(v0));
    });

    RegisterNativeFunction(s, "math.tan", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckNumberArg(a0, 1, "math.tan");
        double v0 = inter::CVarToNumber(a0, 0.0);
        return inter::NativeToFakeluaFloat(state, std::tan(v0));
    });

    RegisterNativeFunction(s, "math.pow", 2, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CVar a1 = inter::GetNativeArg(state, args, n, 1);
        CheckNumberArg(a0, 1, "math.pow");
        CheckNumberArg(a1, 2, "math.pow");
        double v0 = inter::CVarToNumber(a0, 0.0);
        double v1 = inter::CVarToNumber(a1, 0.0);
        return inter::NativeToFakeluaFloat(state, std::pow(v0, v1));
    });

    RegisterNativeFunction(s, "math.asin", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckNumberArg(a0, 1, "math.asin");
        double v0 = inter::CVarToNumber(a0, 0.0);
        return inter::NativeToFakeluaFloat(state, std::asin(v0));
    });

    RegisterNativeFunction(s, "math.acos", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckNumberArg(a0, 1, "math.acos");
        double v0 = inter::CVarToNumber(a0, 0.0);
        return inter::NativeToFakeluaFloat(state, std::acos(v0));
    });

    RegisterNativeFunction(s, "math.atan", 1, true, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckNumberArg(a0, 1, "math.atan");
        double v0 = inter::CVarToNumber(a0, 0.0);
        if (n >= 2) {
            CVar a1 = inter::GetNativeArg(state, args, n, 1);
            CheckNumberArg(a1, 2, "math.atan");
            double v1 = inter::CVarToNumber(a1, 0.0);
            return inter::NativeToFakeluaFloat(state, std::atan2(v0, v1));
        }
        return inter::NativeToFakeluaFloat(state, std::atan(v0));
    });

    RegisterNativeFunction(s, "math.atan2", 2, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CVar a1 = inter::GetNativeArg(state, args, n, 1);
        CheckNumberArg(a0, 1, "math.atan2");
        CheckNumberArg(a1, 2, "math.atan2");
        double v0 = inter::CVarToNumber(a0, 0.0);
        double v1 = inter::CVarToNumber(a1, 0.0);
        return inter::NativeToFakeluaFloat(state, std::atan2(v0, v1));
    });

    RegisterNativeFunction(s, "math.copysign", 2, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CVar a1 = inter::GetNativeArg(state, args, n, 1);
        CheckNumberArg(a0, 1, "math.copysign");
        CheckNumberArg(a1, 2, "math.copysign");
        double v0 = inter::CVarToNumber(a0, 0.0);
        double v1 = inter::CVarToNumber(a1, 0.0);
        return inter::NativeToFakeluaFloat(state, std::copysign(v0, v1));
    });

    RegisterNativeFunction(s, "math.exp", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckNumberArg(a0, 1, "math.exp");
        double v0 = inter::CVarToNumber(a0, 0.0);
        return inter::NativeToFakeluaFloat(state, std::exp(v0));
    });

    RegisterNativeFunction(s, "math.log", 1, true, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckNumberArg(a0, 1, "math.log");
        double v0 = inter::CVarToNumber(a0, 0.0);
        if (n >= 2) {
            CVar a1 = inter::GetNativeArg(state, args, n, 1);
            CheckNumberArg(a1, 2, "math.log");
            double base = inter::CVarToNumber(a1, 1.0);
            return inter::NativeToFakeluaFloat(state, std::log(v0) / std::log(base));
        }
        return inter::NativeToFakeluaFloat(state, std::log(v0));
    });

    RegisterNativeFunction(s, "math.log10", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckNumberArg(a0, 1, "math.log10");
        double v0 = inter::CVarToNumber(a0, 0.0);
        return inter::NativeToFakeluaFloat(state, std::log10(v0));
    });

    RegisterNativeFunction(s, "math.sinh", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckNumberArg(a0, 1, "math.sinh");
        double v0 = inter::CVarToNumber(a0, 0.0);
        return inter::NativeToFakeluaFloat(state, std::sinh(v0));
    });

    RegisterNativeFunction(s, "math.cosh", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckNumberArg(a0, 1, "math.cosh");
        double v0 = inter::CVarToNumber(a0, 0.0);
        return inter::NativeToFakeluaFloat(state, std::cosh(v0));
    });

    RegisterNativeFunction(s, "math.tanh", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckNumberArg(a0, 1, "math.tanh");
        double v0 = inter::CVarToNumber(a0, 0.0);
        return inter::NativeToFakeluaFloat(state, std::tanh(v0));
    });

    RegisterNativeFunction(s, "math.fmod", 2, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CVar a1 = inter::GetNativeArg(state, args, n, 1);
        CheckNumberArg(a0, 1, "math.fmod");
        CheckNumberArg(a1, 2, "math.fmod");
        double v0 = inter::CVarToNumber(a0, 0.0);
        double v1 = inter::CVarToNumber(a1, 0.0);
        if (v1 == 0.0) {
            return inter::NativeToFakeluaFloat(state, std::numeric_limits<double>::quiet_NaN());
        }
        return inter::NativeToFakeluaFloat(state, std::fmod(v0, v1));
    });

    RegisterNativeFunction(s, "math.ldexp", 2, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CVar a1 = inter::GetNativeArg(state, args, n, 1);
        CheckNumberArg(a0, 1, "math.ldexp");
        CheckNumberArg(a1, 2, "math.ldexp");
        double v0 = inter::CVarToNumber(a0, 0.0);
        int v1 = static_cast<int>(inter::CVarToInteger(a1, 0));
        return inter::NativeToFakeluaFloat(state, std::ldexp(v0, v1));
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
        if (!std::isnan(f) && static_cast<double>(static_cast<int64_t>(f)) == f) {
            return inter::NativeToFakeluaInt(state, static_cast<int64_t>(f));
        }
        return inter::NativeToFakeluaNil(state);
    });

    RegisterNativeFunction(s, "math.ult", 2, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CVar a1 = inter::GetNativeArg(state, args, n, 1);
        CheckNumberArg(a0, 1, "math.ult");
        CheckNumberArg(a1, 2, "math.ult");
        uint64_t u0 = static_cast<uint64_t>(inter::CVarToInteger(a0, 0));
        uint64_t u1 = static_cast<uint64_t>(inter::CVarToInteger(a1, 0));
        return inter::NativeToFakeluaBool(state, u0 < u1);
    });

    RegisterNativeFunction(s, "math.deg", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckNumberArg(a0, 1, "math.deg");
        double v0 = inter::CVarToNumber(a0, 0.0);
        return inter::NativeToFakeluaFloat(state, v0 * (180.0 / 3.14159265358979323846));
    });

    RegisterNativeFunction(s, "math.rad", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckNumberArg(a0, 1, "math.rad");
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
            if (a0.type_ != static_cast<int>(VarType::Int) && a0.type_ != static_cast<int>(VarType::Float)) {
                ThrowFakeluaException("bad argument #1 to 'math.random' (number expected)");
            }
            int64_t u = inter::CVarToInteger(a0, 0);
            if (u == 0) {
                // Lua 5.4：math.random(0) 特殊情况，返回全范围随机整数。
                // 拼 4 个 rand 填满 64 位，避免 Windows 上 RAND_MAX=32767 导致位数不足。
                uint64_t rv = (static_cast<uint64_t>(std::rand()) << 48) |
                              (static_cast<uint64_t>(std::rand()) << 32) |
                              (static_cast<uint64_t>(std::rand()) << 16) |
                              static_cast<uint64_t>(std::rand());
                return inter::NativeToFakeluaInt(state, static_cast<int64_t>(rv));
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
            if (a0.type_ != static_cast<int>(VarType::Int) && a0.type_ != static_cast<int>(VarType::Float)) {
                ThrowFakeluaException("bad argument #1 to 'math.random' (number expected)");
            }
            if (a1.type_ != static_cast<int>(VarType::Int) && a1.type_ != static_cast<int>(VarType::Float)) {
                ThrowFakeluaException("bad argument #2 to 'math.random' (number expected)");
            }
            int64_t l = inter::CVarToInteger(a0, 0);
            int64_t u = inter::CVarToInteger(a1, 0);
            if (l > u) {
                // Lua 5.4：空区间报 "interval is empty"
                ThrowFakeluaException("bad argument #1 to 'math.random' (interval is empty)");
            }
            if (l == u) return inter::NativeToFakeluaInt(state, l);
            // 用无符号运算求 range，避免 math.random(0, INT64_MAX) 等有符号溢出（UB）。
            // 与 Lua 5.4 project() 思路一致：range 最大到 2^64（此时回绕为 0），
            // 回绕仅发生在 l=INT64_MIN, u=INT64_MAX 的极端情况，此时直接返回 l。
            uint64_t range = static_cast<uint64_t>(u) - static_cast<uint64_t>(l) + 1;
            if (range == 0) return inter::NativeToFakeluaInt(state, l);  // 区间覆盖整个 int64，任意值都可
            uint64_t rv = (static_cast<uint64_t>(std::rand()) << 32) |
                          static_cast<uint64_t>(std::rand());
            int64_t r = l + static_cast<int64_t>(rv % range);
            return inter::NativeToFakeluaInt(state, r);
        }
    });

    RegisterNativeFunction(s, "math.randomseed", 0, true, [](State *state, CVar *args, int n) -> CVar {
        unsigned int seed = static_cast<unsigned int>(std::time(nullptr));
        if (n >= 1) {
            CVar a0 = inter::GetNativeArg(state, args, n, 0);
            // 标准 Lua：math.randomseed 要求 number 参数，Bool/Table/String/Nil 不合法
            if (a0.type_ != static_cast<int>(VarType::Int) && a0.type_ != static_cast<int>(VarType::Float)) {
                ThrowFakeluaException("bad argument #1 to 'math.randomseed' (number expected)");
            }
            seed = static_cast<unsigned int>(inter::CVarToInteger(a0, seed));
        }
        std::srand(seed);
        return inter::NativeToFakeluaNil(state);
    });

    RegisterNativeFunction(s, "math.modf", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckNumberArg(a0, 1, "math.modf");
        if (a0.type_ == static_cast<int>(VarType::Int)) {
            CVar multi = inter::AllocMultiCVar(state, 2);
            inter::SetMultiCVarElement(multi, 0, a0);
            inter::SetMultiCVarElement(multi, 1, inter::NativeToFakeluaFloat(state, 0.0));
            return multi;
        }
        double val = inter::CVarToNumber(a0, 0.0);
        double iptr;
        double frac = std::modf(val, &iptr);
        CVar multi = inter::AllocMultiCVar(state, 2);
        inter::SetMultiCVarElement(multi, 0, inter::NativeToFakeluaFloat(state, iptr));
        inter::SetMultiCVarElement(multi, 1, inter::NativeToFakeluaFloat(state, frac));
        return multi;
    });

    RegisterNativeFunction(s, "math.frexp", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CheckNumberArg(a0, 1, "math.frexp");
        double val = inter::CVarToNumber(a0, 0.0);
        int exp_val = 0;
        double frac = std::frexp(val, &exp_val);
        CVar multi = inter::AllocMultiCVar(state, 2);
        inter::SetMultiCVarElement(multi, 0, inter::NativeToFakeluaFloat(state, frac));
        inter::SetMultiCVarElement(multi, 1, inter::NativeToFakeluaInt(state, exp_val));
        return multi;
    });
}

}// namespace fakelua
