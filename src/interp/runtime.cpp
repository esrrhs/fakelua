#include "interp/runtime.h"
#include "interp/func_proto.h"
#include "native/table/native_table.h"
#include "state/const_string.h"
#include "state/state.h"
#include "util/exception.h"
#include "var/var.h"
#include "var/var_closure.h"
#include "var/var_multi.h"
#include "var/var_string.h"
#include "var/var_table.h"
#include <cmath>
#include <cstring>
#include <limits>

namespace fakelua::interp_rt {

namespace {

constexpr int kNil = static_cast<int>(VarType::Nil);
constexpr int kBool = static_cast<int>(VarType::Bool);
constexpr int kInt = static_cast<int>(VarType::Int);
constexpr int kFloat = static_cast<int>(VarType::Float);
constexpr int kString = static_cast<int>(VarType::String);
constexpr int kStringId = static_cast<int>(VarType::StringId);
constexpr int kTable = static_cast<int>(VarType::Table);
constexpr int kMulti = static_cast<int>(VarType::Multi);
constexpr int kClosure = static_cast<int>(VarType::Closure);
constexpr int kConstFlag = 0x1;

bool DoubleFitsInt64(double d, int64_t *out) {
    if (!std::isfinite(d)) return false;
    double ip = 0;
    if (std::modf(d, &ip) != 0.0) return false;
    constexpr double kExcl = 9223372036854775808.0;
    if (ip < static_cast<double>(INT64_MIN) || ip >= kExcl) return false;
    *out = static_cast<int64_t>(ip);
    return true;
}

int64_t FloorDivQuotient(int64_t a, int64_t b) {
    if (b == static_cast<int64_t>(-1)) {
        return static_cast<int64_t>(static_cast<uint64_t>(0) - static_cast<uint64_t>(a));
    }
    int64_t q = a / b;
    if (((a ^ b) < 0) && (a % b != 0)) {
        q -= 1;
    }
    return q;
}

VarString *AsVarString(const CVar &v) {
    if (v.type_ == kString) return v.data_.s;
    if (v.type_ == kStringId) return reinterpret_cast<VarString *>(v.data_.i);
    return nullptr;
}

}// namespace

CVar Nil() {
    return CVar{kNil};
}

CVar Bool(bool v) {
    CVar r{};
    r.type_ = kBool;
    r.data_.b = v;
    return r;
}

CVar Int(int64_t v) {
    CVar r{};
    r.type_ = kInt;
    r.data_.i = v;
    return r;
}

CVar Float(double v) {
    CVar r{};
    r.type_ = kFloat;
    r.data_.f = v;
    return r;
}

bool IsTrue(const CVar &v) {
    return v.type_ != kNil && (v.type_ != kBool || v.data_.b);
}

bool IsEqual(const CVar &a, const CVar &b) {
    return AsVar(a).Equal(AsVar(b));
}

void CheckNum(const CVar &v) {
    if (v.type_ != kInt && v.type_ != kFloat) {
        ThrowFakeluaException("attempt to perform arithmetic on non-numeric value");
    }
}

int64_t CheckInt(const CVar &v) {
    if (v.type_ == kInt) return v.data_.i;
    if (v.type_ == kFloat) {
        int64_t i = 0;
        if (!DoubleFitsInt64(v.data_.f, &i)) {
            ThrowFakeluaException("number has no integer representation");
        }
        return i;
    }
    ThrowFakeluaException("attempt to perform bitwise operation on non-numeric value");
}

double ToDouble(const CVar &v) {
    return v.type_ == kInt ? static_cast<double>(v.data_.i) : v.data_.f;
}

CVar BinAdd(const CVar &a, const CVar &b) {
    CheckNum(a);
    CheckNum(b);
    if (a.type_ == kInt && b.type_ == kInt) return Int(a.data_.i + b.data_.i);
    return Float(ToDouble(a) + ToDouble(b));
}

CVar BinSub(const CVar &a, const CVar &b) {
    CheckNum(a);
    CheckNum(b);
    if (a.type_ == kInt && b.type_ == kInt) return Int(a.data_.i - b.data_.i);
    return Float(ToDouble(a) - ToDouble(b));
}

CVar BinMul(const CVar &a, const CVar &b) {
    CheckNum(a);
    CheckNum(b);
    if (a.type_ == kInt && b.type_ == kInt) return Int(a.data_.i * b.data_.i);
    return Float(ToDouble(a) * ToDouble(b));
}

CVar BinDiv(const CVar &a, const CVar &b) {
    CheckNum(a);
    CheckNum(b);
    return Float(ToDouble(a) / ToDouble(b));
}

CVar BinIdiv(const CVar &a, const CVar &b) {
    CheckNum(a);
    CheckNum(b);
    if (a.type_ == kInt && b.type_ == kInt) {
        if (b.data_.i == 0) ThrowFakeluaException("floor division by zero");
        return Int(FloorDivQuotient(a.data_.i, b.data_.i));
    }
    return Float(std::floor(ToDouble(a) / ToDouble(b)));
}

CVar BinMod(const CVar &a, const CVar &b) {
    CheckNum(a);
    CheckNum(b);
    if (a.type_ == kInt && b.type_ == kInt) {
        if (b.data_.i == 0) ThrowFakeluaException("modulo by zero");
        if (b.data_.i == static_cast<int64_t>(-1)) return Int(0);
        const int64_t q = FloorDivQuotient(a.data_.i, b.data_.i);
        return Int(a.data_.i - b.data_.i * q);
    }
    const double fa = ToDouble(a);
    const double fb = ToDouble(b);
    return Float(fa - fb * std::floor(fa / fb));
}

CVar BinPow(const CVar &a, const CVar &b) {
    CheckNum(a);
    CheckNum(b);
    return Float(std::pow(ToDouble(a), ToDouble(b)));
}

CVar BinBand(const CVar &a, const CVar &b) {
    return Int(CheckInt(a) & CheckInt(b));
}

CVar BinBxor(const CVar &a, const CVar &b) {
    return Int(CheckInt(a) ^ CheckInt(b));
}

CVar BinBor(const CVar &a, const CVar &b) {
    return Int(CheckInt(a) | CheckInt(b));
}

CVar BinShl(const CVar &a, const CVar &b) {
    const int64_t ai = CheckInt(a);
    const int64_t bi = CheckInt(b);
    if (bi >= 64 || bi <= -64) return Int(0);
    if (bi >= 0) return Int(static_cast<int64_t>(static_cast<uint64_t>(ai) << bi));
    return Int(static_cast<int64_t>(static_cast<uint64_t>(ai) >> (-bi)));
}

CVar BinShr(const CVar &a, const CVar &b) {
    const int64_t ai = CheckInt(a);
    const int64_t bi = CheckInt(b);
    if (bi >= 64 || bi <= -64) return Int(0);
    if (bi >= 0) return Int(static_cast<int64_t>(static_cast<uint64_t>(ai) >> bi));
    return Int(static_cast<int64_t>(static_cast<uint64_t>(ai) << (-bi)));
}

CVar BinConcat(State *s, const CVar &a, const CVar &b) {
    auto to_str = [&](const CVar &v, std::string &buf, const char *&p, int &len) {
        if (auto *vs = AsVarString(v)) {
            p = vs->Str().data();
            len = static_cast<int>(vs->Size());
            return;
        }
        if (v.type_ != kInt && v.type_ != kFloat) {
            ThrowFakeluaException("attempt to concatenate a non-string value");
        }
        if (v.type_ == kInt) buf = std::to_string(v.data_.i);
        else {
            char tmp[256];
            std::snprintf(tmp, sizeof(tmp), "%.17g", v.data_.f);
            buf = tmp;
        }
        p = buf.c_str();
        len = static_cast<int>(buf.size());
    };
    std::string ba, bb;
    const char *sa = nullptr;
    const char *sb = nullptr;
    int la = 0, lb = 0;
    to_str(a, ba, sa, la);
    to_str(b, bb, sb, lb);
    const int total = la + lb;
    if (total < la || total < lb) {
        ThrowFakeluaException("string concatenation result too long");
    }
    VarString *vs = VarString::AllocTempRaw(s, static_cast<size_t>(total));
    std::memcpy(vs->MutableData(), sa, static_cast<size_t>(la));
    std::memcpy(vs->MutableData() + la, sb, static_cast<size_t>(lb));
    CVar r{};
    r.type_ = kString;
    r.data_.s = vs;
    return r;
}

CVar UnMinus(const CVar &a) {
    CheckNum(a);
    if (a.type_ == kInt) {
        if (a.data_.i == INT64_MIN) return Float(-static_cast<double>(a.data_.i));
        return Int(-a.data_.i);
    }
    return Float(-a.data_.f);
}

CVar UnNot(const CVar &a) {
    return Bool(!IsTrue(a));
}

CVar UnLen(const CVar &a) {
    if (a.type_ == kString) return Int(static_cast<int64_t>(a.data_.s->Size()));
    if (a.type_ == kStringId) {
        auto *vs = reinterpret_cast<VarString *>(a.data_.i);
        return Int(static_cast<int64_t>(vs->Size()));
    }
    if (a.type_ == kTable) return Int(table::TableHelper::GetTableLen(a));
    ThrowFakeluaException("attempt to get length of a non-string/table value");
}

CVar UnBnot(const CVar &a) {
    return Int(~CheckInt(a));
}

CVar CmpLt(const CVar &a, const CVar &b) {
    CheckNum(a);
    CheckNum(b);
    if (a.type_ == kInt && b.type_ == kInt) return Bool(a.data_.i < b.data_.i);
    return Bool(ToDouble(a) < ToDouble(b));
}

CVar CmpLe(const CVar &a, const CVar &b) {
    CheckNum(a);
    CheckNum(b);
    if (a.type_ == kInt && b.type_ == kInt) return Bool(a.data_.i <= b.data_.i);
    return Bool(ToDouble(a) <= ToDouble(b));
}

CVar GetTable(State *s, CVar t, CVar k) {
    if (t.type_ != kTable) {
        ThrowFakeluaException("attempt to index a non-table value");
    }
    if (k.type_ == kNil) {
        ThrowFakeluaException("table index is nil");
    }
    return table::TableHelper::GetTable(s, t, k);
}

void SetTable(State *s, CVar t, CVar k, CVar v) {
    if (t.type_ != kTable) {
        ThrowFakeluaException("attempt to index a non-table value");
    }
    if (k.type_ == kNil) {
        ThrowFakeluaException("table index is nil");
    }
    if (t.flag_ & kConstFlag) {
        ThrowFakeluaException("attempt to modify a const table");
    }
    table::TableHelper::SetTable(s, t, k, v);
}

CVar NewTable(State *s) {
    VarTable *vtbl = static_cast<VarTable *>(Alloc(s, sizeof(VarTable)));
    *vtbl = VarTable{};
    for (auto &qd: vtbl->quick_data_) {
        qd.key.type_ = kNil;
        qd.val.type_ = kNil;
    }
    vtbl->free_list_idx_ = VarTable::INVALID_INDEX;
    vtbl->seq_len_ = 0;
    vtbl->seq_len_valid_ = 1;
    CVar tbl{};
    tbl.type_ = kTable;
    tbl.data_.t = vtbl;
    return tbl;
}

void TableExpandMulti(State *s, CVar t, int64_t start_idx, CVar v) {
    if (v.type_ != kMulti) {
        SetTable(s, t, Int(start_idx), v);
        return;
    }
    VarMulti *m = v.data_.m;
    for (uint32_t i = 0; i < m->GetCount(); ++i) {
        SetTable(s, t, Int(start_idx + static_cast<int64_t>(i)), m->GetVars()[i]);
    }
}

bool TableEntry(CVar t, uint32_t idx, CVar &k, CVar &v) {
    if (t.type_ != kTable || !t.data_.t) return false;
    VarTable *tbl = t.data_.t;
    const uint32_t spec_cnt = tbl->spec_count;
    if (idx < spec_cnt) {
        k = tbl->spec_keys[idx];
        v = tbl->spec_vals[idx];
        return true;
    }
    const uint32_t hidx = idx - spec_cnt;
    if (tbl->bucket_count_ == 0) {
        if (hidx >= tbl->count_) return false;
        k = tbl->quick_data_[hidx].key;
        v = tbl->quick_data_[hidx].val;
        return true;
    }
    if (!tbl->active_list_ || hidx >= tbl->count_) return false;
    const uint32_t node_idx = tbl->active_list_[hidx];
    k = tbl->nodes_[node_idx].entry.key;
    v = tbl->nodes_[node_idx].entry.val;
    return true;
}

uint32_t TableEntryCount(CVar t) {
    if (t.type_ != kTable || !t.data_.t) {
        ThrowFakeluaException("for in: not a table");
    }
    return t.data_.t->count_ + t.data_.t->spec_count;
}

CVar UnboxMulti(const CVar &v, uint32_t idx) {
    if (v.type_ != kMulti) {
        return idx == 0 ? v : Nil();
    }
    VarMulti *m = v.data_.m;
    return idx < m->GetCount() ? m->GetVars()[idx] : Nil();
}

CVar CombineMulti(State *s, const CVar *prefix, uint32_t prefix_count, CVar last) {
    uint32_t last_count = 1;
    const CVar *last_vars = &last;
    if (last.type_ == kMulti) {
        last_count = last.data_.m->GetCount();
        last_vars = last.data_.m->GetVars();
    }
    const uint32_t total = prefix_count + last_count;
    CVar multi = inter::AllocMultiCVar(s, static_cast<int>(total));
    for (uint32_t i = 0; i < prefix_count; ++i) {
        CVar pv = prefix[i];
        if (pv.type_ == kMulti) {
            pv = pv.data_.m->GetCount() > 0 ? pv.data_.m->GetVars()[0] : Nil();
        }
        inter::SetMultiCVarElement(multi, static_cast<int>(i), pv);
    }
    for (uint32_t i = 0; i < last_count; ++i) {
        inter::SetMultiCVarElement(multi, static_cast<int>(prefix_count + i), last_vars[i]);
    }
    return multi;
}

CVar MakeMulti(State *s, const CVar *vals, uint32_t count) {
    CVar multi = inter::AllocMultiCVar(s, static_cast<int>(count));
    for (uint32_t i = 0; i < count; ++i) {
        CVar pv = vals[i];
        if (pv.type_ == kMulti) {
            pv = pv.data_.m->GetCount() > 0 ? pv.data_.m->GetVars()[0] : Nil();
        }
        inter::SetMultiCVarElement(multi, static_cast<int>(i), pv);
    }
    return multi;
}

CVar MakeClosure(State *s, void *func_ptr, int upvalue_count, int expected_arg_count, bool is_vararg, CVar **upvals) {
    auto *cl = static_cast<VarClosure *>(Alloc(s, sizeof(VarClosure) + static_cast<size_t>(upvalue_count) * sizeof(CVar *)));
    cl->func_ptr = func_ptr;
    cl->upvalue_count = upvalue_count;
    cl->expected_arg_count = expected_arg_count;
    cl->is_vararg = is_vararg;
    cl->code_str = kInterpClosureMagic;
    for (int i = 0; i < upvalue_count; ++i) {
        cl->upvalues[i] = upvals[i];
    }
    CVar r{};
    r.type_ = kClosure;
    r.data_.cl = cl;
    return r;
}

bool ForIntAdvance(int64_t *ctrl, int64_t step) {
    const int64_t cur = *ctrl;
    const int64_t next = static_cast<int64_t>(static_cast<uint64_t>(cur) + static_cast<uint64_t>(step));
    if ((cur ^ step) >= 0 && (next ^ cur) < 0) return false;
    *ctrl = next;
    return true;
}

bool ForStepPositive(const CVar &step) {
    if (step.type_ == kInt) {
        if (step.data_.i == 0) {
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

void *Alloc(State *s, size_t size) {
    return FakeluaAlloc(s, size, s->InterpConstAlloc());
}

}// namespace fakelua::interp_rt
