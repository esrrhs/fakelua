#pragma once

// 解释器热路径：宏 + always_inline，展开进 InterpreterExecute，
// 让 GCC 把分发循环和算术当成同一函数体优化。冷路径（concat/table/closure）仍走 runtime.cpp。

#include "fakelua.h"
#include "util/exception.h"
#include "var/var_multi.h"
#include "var/var_type.h"
#include <cmath>
#include <cstdint>
#include <limits>

namespace fakelua::interp_rt {

#if defined(__GNUC__)
#define FL_VM_INLINE inline __attribute__((always_inline))
#define FL_VM_LIKELY(x) __builtin_expect(!!(x), 1)
#define FL_VM_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define FL_VM_INLINE inline
#define FL_VM_LIKELY(x) (x)
#define FL_VM_UNLIKELY(x) (x)
#endif

inline constexpr int kNil = static_cast<int>(::fakelua::VarType::Nil);
inline constexpr int kBool = static_cast<int>(::fakelua::VarType::Bool);
inline constexpr int kInt = static_cast<int>(::fakelua::VarType::Int);
inline constexpr int kFloat = static_cast<int>(::fakelua::VarType::Float);
inline constexpr int kString = static_cast<int>(::fakelua::VarType::String);
inline constexpr int kStringId = static_cast<int>(::fakelua::VarType::StringId);
inline constexpr int kTable = static_cast<int>(::fakelua::VarType::Table);
inline constexpr int kMulti = static_cast<int>(::fakelua::VarType::Multi);
inline constexpr int kClosure = static_cast<int>(::fakelua::VarType::Closure);
inline constexpr int kConstFlag = 0x1;

// HasBoxes=false：寄存器就是 stk[r]，无 boxes[] 检查、无 16 字节中转拷贝。
// HasBoxes=true：被捕获的局部在 heap box 上，读写走 box，并镜像回 stk。
template<bool HasBoxes>
FL_VM_INLINE CVar &VmSlot(CVar *stk, CVar **box, int r) {
    const size_t i = static_cast<size_t>(r);
    if constexpr (HasBoxes) {
        CVar *b = box[i];
        return b ? *b : stk[i];
    } else {
        return stk[i];
    }
}

template<bool HasBoxes>
FL_VM_INLINE void VmWrite(CVar *stk, CVar **box, int r, const CVar &v) {
    const size_t i = static_cast<size_t>(r);
    stk[i] = v;
    if constexpr (HasBoxes) {
        if (box[i]) {
            *box[i] = v;
        }
    }
}

template<bool HasBoxes>
FL_VM_INLINE void VmWriteInt(CVar *stk, CVar **box, int r, int64_t n) {
    const size_t i = static_cast<size_t>(r);
    CVar &s = stk[i];
    s.type_ = kInt;
    s.flag_ = 0;
    s.data_.i = n;
    if constexpr (HasBoxes) {
        if (CVar *b = box[i]) {
            b->type_ = kInt;
            b->flag_ = 0;
            b->data_.i = n;
        }
    }
}

template<bool HasBoxes>
FL_VM_INLINE void VmClearBox(CVar **box, int r) {
    if constexpr (HasBoxes) {
        box[static_cast<size_t>(r)] = nullptr;
    }
}

FL_VM_INLINE CVar Nil() {
    return CVar{kNil};
}

FL_VM_INLINE CVar Bool(bool v) {
    CVar r{};
    r.type_ = kBool;
    r.data_.b = v;
    return r;
}

FL_VM_INLINE CVar Int(int64_t v) {
    CVar r{};
    r.type_ = kInt;
    r.data_.i = v;
    return r;
}

FL_VM_INLINE CVar Float(double v) {
    CVar r{};
    r.type_ = kFloat;
    r.data_.f = v;
    return r;
}

FL_VM_INLINE bool IsTrue(const CVar &v) {
    return v.type_ != kNil && (v.type_ != kBool || v.data_.b);
}

FL_VM_INLINE void CheckNum(const CVar &v) {
    if (FL_VM_UNLIKELY(v.type_ != kInt && v.type_ != kFloat)) {
        ThrowFakeluaException("attempt to perform arithmetic on non-numeric value");
    }
}

FL_VM_INLINE bool DoubleFitsInt64(double d, int64_t *out) {
    if (!std::isfinite(d)) return false;
    double ip = 0;
    if (std::modf(d, &ip) != 0.0) return false;
    constexpr double kExcl = 9223372036854775808.0;
    if (ip < static_cast<double>(INT64_MIN) || ip >= kExcl) return false;
    *out = static_cast<int64_t>(ip);
    return true;
}

FL_VM_INLINE int64_t CheckInt(const CVar &v) {
    if (FL_VM_LIKELY(v.type_ == kInt)) return v.data_.i;
    if (v.type_ == kFloat) {
        int64_t i = 0;
        if (!DoubleFitsInt64(v.data_.f, &i)) {
            ThrowFakeluaException("number has no integer representation");
        }
        return i;
    }
    ThrowFakeluaException("attempt to perform bitwise operation on non-numeric value");
}

FL_VM_INLINE double ToDouble(const CVar &v) {
    return v.type_ == kInt ? static_cast<double>(v.data_.i) : v.data_.f;
}

FL_VM_INLINE int64_t FloorDivQuotient(int64_t a, int64_t b) {
    if (b == static_cast<int64_t>(-1)) {
        return static_cast<int64_t>(static_cast<uint64_t>(0) - static_cast<uint64_t>(a));
    }
    int64_t q = a / b;
    if (((a ^ b) < 0) && (a % b != 0)) {
        q -= 1;
    }
    return q;
}

FL_VM_INLINE CVar BinAdd(const CVar &a, const CVar &b) {
    if (FL_VM_LIKELY(a.type_ == kInt && b.type_ == kInt)) {
        return Int(static_cast<int64_t>(static_cast<uint64_t>(a.data_.i) + static_cast<uint64_t>(b.data_.i)));
    }
    CheckNum(a);
    CheckNum(b);
    return Float(ToDouble(a) + ToDouble(b));
}

FL_VM_INLINE CVar BinSub(const CVar &a, const CVar &b) {
    if (FL_VM_LIKELY(a.type_ == kInt && b.type_ == kInt)) {
        return Int(static_cast<int64_t>(static_cast<uint64_t>(a.data_.i) - static_cast<uint64_t>(b.data_.i)));
    }
    CheckNum(a);
    CheckNum(b);
    return Float(ToDouble(a) - ToDouble(b));
}

FL_VM_INLINE CVar BinMul(const CVar &a, const CVar &b) {
    if (FL_VM_LIKELY(a.type_ == kInt && b.type_ == kInt)) {
        return Int(static_cast<int64_t>(static_cast<uint64_t>(a.data_.i) * static_cast<uint64_t>(b.data_.i)));
    }
    CheckNum(a);
    CheckNum(b);
    return Float(ToDouble(a) * ToDouble(b));
}

FL_VM_INLINE CVar BinDiv(const CVar &a, const CVar &b) {
    CheckNum(a);
    CheckNum(b);
    return Float(ToDouble(a) / ToDouble(b));
}

FL_VM_INLINE CVar BinIdiv(const CVar &a, const CVar &b) {
    CheckNum(a);
    CheckNum(b);
    if (FL_VM_LIKELY(a.type_ == kInt && b.type_ == kInt)) {
        if (FL_VM_UNLIKELY(b.data_.i == 0)) ThrowFakeluaException("floor division by zero");
        return Int(FloorDivQuotient(a.data_.i, b.data_.i));
    }
    return Float(std::floor(ToDouble(a) / ToDouble(b)));
}

FL_VM_INLINE CVar BinMod(const CVar &a, const CVar &b) {
    CheckNum(a);
    CheckNum(b);
    if (FL_VM_LIKELY(a.type_ == kInt && b.type_ == kInt)) {
        if (FL_VM_UNLIKELY(b.data_.i == 0)) ThrowFakeluaException("modulo by zero");
        if (b.data_.i == static_cast<int64_t>(-1)) return Int(0);
        const int64_t q = FloorDivQuotient(a.data_.i, b.data_.i);
        return Int(a.data_.i - b.data_.i * q);
    }
    const double fa = ToDouble(a);
    const double fb = ToDouble(b);
    return Float(fa - fb * std::floor(fa / fb));
}

FL_VM_INLINE CVar BinPow(const CVar &a, const CVar &b) {
    CheckNum(a);
    CheckNum(b);
    return Float(std::pow(ToDouble(a), ToDouble(b)));
}

FL_VM_INLINE CVar BinBand(const CVar &a, const CVar &b) {
    return Int(CheckInt(a) & CheckInt(b));
}

FL_VM_INLINE CVar BinBxor(const CVar &a, const CVar &b) {
    return Int(CheckInt(a) ^ CheckInt(b));
}

FL_VM_INLINE CVar BinBor(const CVar &a, const CVar &b) {
    return Int(CheckInt(a) | CheckInt(b));
}

FL_VM_INLINE CVar BinShl(const CVar &a, const CVar &b) {
    const int64_t ai = CheckInt(a);
    const int64_t bi = CheckInt(b);
    if (bi >= 64 || bi <= -64) return Int(0);
    if (bi >= 0) return Int(static_cast<int64_t>(static_cast<uint64_t>(ai) << bi));
    return Int(static_cast<int64_t>(static_cast<uint64_t>(ai) >> (-bi)));
}

FL_VM_INLINE CVar BinShr(const CVar &a, const CVar &b) {
    const int64_t ai = CheckInt(a);
    const int64_t bi = CheckInt(b);
    if (bi >= 64 || bi <= -64) return Int(0);
    if (bi >= 0) return Int(static_cast<int64_t>(static_cast<uint64_t>(ai) >> bi));
    return Int(static_cast<int64_t>(static_cast<uint64_t>(ai) << (-bi)));
}

FL_VM_INLINE CVar UnMinus(const CVar &a) {
    CheckNum(a);
    if (FL_VM_LIKELY(a.type_ == kInt)) {
        return Int(static_cast<int64_t>(0ull - static_cast<uint64_t>(a.data_.i)));
    }
    return Float(-a.data_.f);
}

FL_VM_INLINE CVar UnNot(const CVar &a) {
    return Bool(!IsTrue(a));
}

FL_VM_INLINE CVar UnBnot(const CVar &a) {
    return Int(~CheckInt(a));
}

FL_VM_INLINE CVar CmpLt(const CVar &a, const CVar &b) {
    if (FL_VM_LIKELY(a.type_ == kInt && b.type_ == kInt)) return Bool(a.data_.i < b.data_.i);
    CheckNum(a);
    CheckNum(b);
    return Bool(ToDouble(a) < ToDouble(b));
}

FL_VM_INLINE CVar CmpLe(const CVar &a, const CVar &b) {
    if (FL_VM_LIKELY(a.type_ == kInt && b.type_ == kInt)) return Bool(a.data_.i <= b.data_.i);
    CheckNum(a);
    CheckNum(b);
    return Bool(ToDouble(a) <= ToDouble(b));
}

FL_VM_INLINE CVar UnboxMulti(const CVar &v, uint32_t idx) {
    if (v.type_ != kMulti) {
        return idx == 0 ? v : Nil();
    }
    VarMulti *m = v.data_.m;
    return idx < m->GetCount() ? m->GetVars()[idx] : Nil();
}

FL_VM_INLINE bool ForStepPositive(const CVar &step) {
    if (FL_VM_LIKELY(step.type_ == kInt)) {
        if (FL_VM_UNLIKELY(step.data_.i == 0)) {
            ThrowFakeluaException("'for' step is zero");
        }
        return step.data_.i > 0;
    }
    if (step.type_ == kFloat) {
        if (step.data_.f == 0.0) {
            ThrowFakeluaException("'for' step is zero");
        }
        return step.data_.f > 0.0;
    }
    ThrowFakeluaException("'for' step must be a number");
}

FL_VM_INLINE bool ForLoopInRange(const CVar &ctrl, const CVar &limit, const CVar &step) {
    if (FL_VM_LIKELY(ctrl.type_ == kInt && limit.type_ == kInt && step.type_ == kInt)) {
        return step.data_.i > 0 ? (ctrl.data_.i <= limit.data_.i) : (ctrl.data_.i >= limit.data_.i);
    }
    CheckNum(ctrl);
    CheckNum(limit);
    const double c = ToDouble(ctrl);
    const double l = ToDouble(limit);
    return ForStepPositive(step) ? (c <= l) : (l <= c);
}

// 把 for 的 limit 收成 int64。超出整数范围时按 Lua 5.4 forlimit 裁剪/跳过。
// *skip=true 表示循环一次都不应执行。
FL_VM_INLINE int64_t ForLimitToInt(const CVar &limit, int64_t step, bool *skip) {
    if (limit.type_ == kInt) {
        return limit.data_.i;
    }
    CheckNum(limit);
    const double flim = limit.data_.f;
    int64_t p = 0;
    if (DoubleFitsInt64(flim, &p)) {
        return p;
    }
    constexpr double kMaxP1 = 9223372036854775808.0;
    if (!std::isfinite(flim)) {
        if (flim > 0.0) {
            if (step < 0) {
                *skip = true;
                return 0;
            }
            return std::numeric_limits<int64_t>::max();
        }
        if (step > 0) {
            *skip = true;
            return 0;
        }
        return std::numeric_limits<int64_t>::min();
    }
    if (flim >= kMaxP1) {
        if (step < 0) {
            *skip = true;
            return 0;
        }
        return std::numeric_limits<int64_t>::max();
    }
    if (flim < static_cast<double>(std::numeric_limits<int64_t>::min())) {
        if (step > 0) {
            *skip = true;
            return 0;
        }
        return std::numeric_limits<int64_t>::min();
    }
    return step > 0 ? static_cast<int64_t>(std::floor(flim)) : static_cast<int64_t>(std::ceil(flim));
}

// 准备 numeric for。true=整段跳过。
// 整数：R(A) 仍是 idx，R(A+1) 改成剩余次数（不含本轮），R(A+2) step，loopvar=init。
// 浮点：三槽都改成 float，R(A+1) 仍是 limit。
FL_VM_INLINE bool ForPrep(CVar &idx, CVar &limit, CVar &step, CVar &loopvar) {
    if (FL_VM_LIKELY(idx.type_ == kInt && step.type_ == kInt)) {
        const int64_t init = idx.data_.i;
        const int64_t st = step.data_.i;
        if (FL_VM_UNLIKELY(st == 0)) {
            ThrowFakeluaException("'for' step is zero");
        }
        loopvar = idx;
        if (limit.type_ == kInt || limit.type_ == kFloat) {
            bool skip = false;
            const int64_t lim = ForLimitToInt(limit, st, &skip);
            if (skip || (st > 0 ? init > lim : init < lim)) {
                return true;
            }
            uint64_t count = 0;
            if (st > 0) {
                count = static_cast<uint64_t>(lim) - static_cast<uint64_t>(init);
                if (st != 1) {
                    count /= static_cast<uint64_t>(st);
                }
            } else {
                count = static_cast<uint64_t>(init) - static_cast<uint64_t>(lim);
                count /= static_cast<uint64_t>(-(st + 1)) + 1u;
            }
            limit = Int(static_cast<int64_t>(count));
            return false;
        }
    }
    CheckNum(idx);
    CheckNum(limit);
    CheckNum(step);
    const double i = ToDouble(idx);
    const double l = ToDouble(limit);
    const double st = ToDouble(step);
    if (FL_VM_UNLIKELY(st == 0.0)) {
        ThrowFakeluaException("'for' step is zero");
    }
    idx = Float(i);
    limit = Float(l);
    step = Float(st);
    loopvar = idx;
    return st > 0.0 ? (l < i) : (i < l);
}

// 二元算术：int/int 快路径就地写 type_/data_.i，不构造临时 CVar。
#define FL_VM_ARITH_INT(HAS, stk, box, ra, rb, rc, OP, SLOWFN)                                                                             \
    do {                                                                                                                                   \
        const CVar &fl_rb_ = ::fakelua::interp_rt::VmSlot<HAS>(stk, box, rb);                                                               \
        const CVar &fl_rc_ = ::fakelua::interp_rt::VmSlot<HAS>(stk, box, rc);                                                               \
        if (FL_VM_LIKELY(fl_rb_.type_ == ::fakelua::interp_rt::kInt && fl_rc_.type_ == ::fakelua::interp_rt::kInt)) {                       \
            ::fakelua::interp_rt::VmWriteInt<HAS>(stk, box, ra,                                                                            \
                static_cast<int64_t>(static_cast<uint64_t>(fl_rb_.data_.i) OP static_cast<uint64_t>(fl_rc_.data_.i)));                     \
        } else {                                                                                                                           \
            ::fakelua::interp_rt::VmWrite<HAS>(stk, box, ra, (SLOWFN)(fl_rb_, fl_rc_));                                                     \
        }                                                                                                                                  \
    } while (0)

#define FL_VM_BINOP(HAS, stk, box, ra, rb, rc, FN)                                                                                         \
    do {                                                                                                                                   \
        ::fakelua::interp_rt::VmWrite<HAS>(stk, box, ra, (FN)(::fakelua::interp_rt::VmSlot<HAS>(stk, box, rb),                              \
                                                               ::fakelua::interp_rt::VmSlot<HAS>(stk, box, rc)));                          \
    } while (0)

#define FL_VM_UNOP(HAS, stk, box, ra, rb, FN)                                                                                              \
    do {                                                                                                                                   \
        ::fakelua::interp_rt::VmWrite<HAS>(stk, box, ra, (FN)(::fakelua::interp_rt::VmSlot<HAS>(stk, box, rb)));                             \
    } while (0)

}// namespace fakelua::interp_rt
