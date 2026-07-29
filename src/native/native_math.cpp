#include "native/native_math.h"
#include "var/var.h"
#include <cmath>

namespace fakelua {

void RegisterMathLibraryApi(State *s) {
    if (!s) return;

    RegisterNativeFunction(s, "math.abs", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        if (a0.type_ == static_cast<int>(VarType::Int)) return inter::NativeToFakeluaInt(state, std::abs(a0.data_.i));
        if (a0.type_ == static_cast<int>(VarType::Float)) return inter::NativeToFakeluaFloat(state, std::abs(a0.data_.f));
        return inter::NativeToFakeluaNil(state);
    });

    RegisterNativeFunction(s, "math.floor", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        if (a0.type_ == static_cast<int>(VarType::Int)) return a0;
        if (a0.type_ == static_cast<int>(VarType::Float)) return inter::NativeToFakeluaFloat(state, std::floor(a0.data_.f));
        return inter::NativeToFakeluaNil(state);
    });

    RegisterNativeFunction(s, "math.ceil", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        if (a0.type_ == static_cast<int>(VarType::Int)) return a0;
        if (a0.type_ == static_cast<int>(VarType::Float)) return inter::NativeToFakeluaFloat(state, std::ceil(a0.data_.f));
        return inter::NativeToFakeluaNil(state);
    });

    RegisterNativeFunction(s, "math.max", 2, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CVar a1 = inter::GetNativeArg(state, args, n, 1);
        double v0 = (a0.type_ == static_cast<int>(VarType::Int)) ? a0.data_.i : (a0.type_ == static_cast<int>(VarType::Float) ? a0.data_.f : 0.0);
        double v1 = (a1.type_ == static_cast<int>(VarType::Int)) ? a1.data_.i : (a1.type_ == static_cast<int>(VarType::Float) ? a1.data_.f : 0.0);
        return (v0 >= v1) ? a0 : a1;
    });

    RegisterNativeFunction(s, "math.min", 2, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CVar a1 = inter::GetNativeArg(state, args, n, 1);
        double v0 = (a0.type_ == static_cast<int>(VarType::Int)) ? a0.data_.i : (a0.type_ == static_cast<int>(VarType::Float) ? a0.data_.f : 0.0);
        double v1 = (a1.type_ == static_cast<int>(VarType::Int)) ? a1.data_.i : (a1.type_ == static_cast<int>(VarType::Float) ? a1.data_.f : 0.0);
        return (v0 <= v1) ? a0 : a1;
    });

    RegisterNativeFunction(s, "math.sqrt", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        double v0 = (a0.type_ == static_cast<int>(VarType::Int)) ? a0.data_.i : (a0.type_ == static_cast<int>(VarType::Float) ? a0.data_.f : 0.0);
        return inter::NativeToFakeluaFloat(state, std::sqrt(v0));
    });

    RegisterNativeFunction(s, "math.sin", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        double v0 = (a0.type_ == static_cast<int>(VarType::Int)) ? a0.data_.i : (a0.type_ == static_cast<int>(VarType::Float) ? a0.data_.f : 0.0);
        return inter::NativeToFakeluaFloat(state, std::sin(v0));
    });

    RegisterNativeFunction(s, "math.cos", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        double v0 = (a0.type_ == static_cast<int>(VarType::Int)) ? a0.data_.i : (a0.type_ == static_cast<int>(VarType::Float) ? a0.data_.f : 0.0);
        return inter::NativeToFakeluaFloat(state, std::cos(v0));
    });

    RegisterNativeFunction(s, "math.tan", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        double v0 = (a0.type_ == static_cast<int>(VarType::Int)) ? a0.data_.i : (a0.type_ == static_cast<int>(VarType::Float) ? a0.data_.f : 0.0);
        return inter::NativeToFakeluaFloat(state, std::tan(v0));
    });

    RegisterNativeFunction(s, "math.pow", 2, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CVar a1 = inter::GetNativeArg(state, args, n, 1);
        double v0 = (a0.type_ == static_cast<int>(VarType::Int)) ? a0.data_.i : (a0.type_ == static_cast<int>(VarType::Float) ? a0.data_.f : 0.0);
        double v1 = (a1.type_ == static_cast<int>(VarType::Int)) ? a1.data_.i : (a1.type_ == static_cast<int>(VarType::Float) ? a1.data_.f : 0.0);
        return inter::NativeToFakeluaFloat(state, std::pow(v0, v1));
    });

    RegisterNativeFunction(s, "math.asin", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        double v0 = (a0.type_ == static_cast<int>(VarType::Int)) ? a0.data_.i : (a0.type_ == static_cast<int>(VarType::Float) ? a0.data_.f : 0.0);
        return inter::NativeToFakeluaFloat(state, std::asin(v0));
    });

    RegisterNativeFunction(s, "math.acos", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        double v0 = (a0.type_ == static_cast<int>(VarType::Int)) ? a0.data_.i : (a0.type_ == static_cast<int>(VarType::Float) ? a0.data_.f : 0.0);
        return inter::NativeToFakeluaFloat(state, std::acos(v0));
    });

    RegisterNativeFunction(s, "math.atan", 1, true, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        double v0 = (a0.type_ == static_cast<int>(VarType::Int)) ? a0.data_.i : (a0.type_ == static_cast<int>(VarType::Float) ? a0.data_.f : 0.0);
        if (n >= 2) {
            CVar a1 = inter::GetNativeArg(state, args, n, 1);
            double v1 = (a1.type_ == static_cast<int>(VarType::Int)) ? a1.data_.i : (a1.type_ == static_cast<int>(VarType::Float) ? a1.data_.f : 0.0);
            return inter::NativeToFakeluaFloat(state, std::atan2(v0, v1));
        }
        return inter::NativeToFakeluaFloat(state, std::atan(v0));
    });

    RegisterNativeFunction(s, "math.exp", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        double v0 = (a0.type_ == static_cast<int>(VarType::Int)) ? a0.data_.i : (a0.type_ == static_cast<int>(VarType::Float) ? a0.data_.f : 0.0);
        return inter::NativeToFakeluaFloat(state, std::exp(v0));
    });

    RegisterNativeFunction(s, "math.log", 1, true, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        double v0 = (a0.type_ == static_cast<int>(VarType::Int)) ? a0.data_.i : (a0.type_ == static_cast<int>(VarType::Float) ? a0.data_.f : 0.0);
        if (n >= 2) {
            CVar a1 = inter::GetNativeArg(state, args, n, 1);
            double v1 = (a1.type_ == static_cast<int>(VarType::Int)) ? a1.data_.i : (a1.type_ == static_cast<int>(VarType::Float) ? a1.data_.f : 0.0);
            return inter::NativeToFakeluaFloat(state, std::log(v0) / std::log(v1));
        }
        return inter::NativeToFakeluaFloat(state, std::log(v0));
    });

    RegisterNativeFunction(s, "math.log10", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        double v0 = (a0.type_ == static_cast<int>(VarType::Int)) ? a0.data_.i : (a0.type_ == static_cast<int>(VarType::Float) ? a0.data_.f : 0.0);
        return inter::NativeToFakeluaFloat(state, std::log10(v0));
    });

    RegisterNativeFunction(s, "math.sinh", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        double v0 = (a0.type_ == static_cast<int>(VarType::Int)) ? a0.data_.i : (a0.type_ == static_cast<int>(VarType::Float) ? a0.data_.f : 0.0);
        return inter::NativeToFakeluaFloat(state, std::sinh(v0));
    });

    RegisterNativeFunction(s, "math.cosh", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        double v0 = (a0.type_ == static_cast<int>(VarType::Int)) ? a0.data_.i : (a0.type_ == static_cast<int>(VarType::Float) ? a0.data_.f : 0.0);
        return inter::NativeToFakeluaFloat(state, std::cosh(v0));
    });

    RegisterNativeFunction(s, "math.tanh", 1, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        double v0 = (a0.type_ == static_cast<int>(VarType::Int)) ? a0.data_.i : (a0.type_ == static_cast<int>(VarType::Float) ? a0.data_.f : 0.0);
        return inter::NativeToFakeluaFloat(state, std::tanh(v0));
    });

    RegisterNativeFunction(s, "math.fmod", 2, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CVar a1 = inter::GetNativeArg(state, args, n, 1);
        double v0 = (a0.type_ == static_cast<int>(VarType::Int)) ? a0.data_.i : (a0.type_ == static_cast<int>(VarType::Float) ? a0.data_.f : 0.0);
        double v1 = (a1.type_ == static_cast<int>(VarType::Int)) ? a1.data_.i : (a1.type_ == static_cast<int>(VarType::Float) ? a1.data_.f : 0.0);
        return inter::NativeToFakeluaFloat(state, std::fmod(v0, v1));
    });

    RegisterNativeFunction(s, "math.ldexp", 2, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CVar a1 = inter::GetNativeArg(state, args, n, 1);
        double v0 = (a0.type_ == static_cast<int>(VarType::Int)) ? a0.data_.i : (a0.type_ == static_cast<int>(VarType::Float) ? a0.data_.f : 0.0);
        int v1 = (a1.type_ == static_cast<int>(VarType::Int)) ? a1.data_.i : (a1.type_ == static_cast<int>(VarType::Float) ? static_cast<int>(a1.data_.f) : 0);
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
        if (a0.type_ == static_cast<int>(VarType::Int)) return a0;
        if (a0.type_ == static_cast<int>(VarType::Float)) {
            double f = a0.data_.f;
            if (static_cast<double>(static_cast<int64_t>(f)) == f) {
                return inter::NativeToFakeluaInt(state, static_cast<int64_t>(f));
            }
        }
        return inter::NativeToFakeluaNil(state);
    });

    RegisterNativeFunction(s, "math.ult", 2, false, [](State *state, CVar *args, int n) -> CVar {
        CVar a0 = inter::GetNativeArg(state, args, n, 0);
        CVar a1 = inter::GetNativeArg(state, args, n, 1);
        uint64_t u0 = (a0.type_ == static_cast<int>(VarType::Int)) ? static_cast<uint64_t>(a0.data_.i) : 0;
        uint64_t u1 = (a1.type_ == static_cast<int>(VarType::Int)) ? static_cast<uint64_t>(a1.data_.i) : 0;
        return inter::NativeToFakeluaBool(state, u0 < u1);
    });
}

}// namespace fakelua
