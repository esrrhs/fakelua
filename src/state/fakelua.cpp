#include "fakelua.h"
#include "compile/c_runtime_header.h"
#include "state/state.h"
#include "util/common.h"
#include "util/dispatch_macro.h"
#include "var/var_multi.h"
#include "var/var_string.h"
#include "var/var_table.h"

namespace fakelua {

namespace inter {

template<typename T>
CVar NativeToFakeluaIntHelper(T val) {
    Var ret;
    ret.SetInt(static_cast<int64_t>(val));
    return ret;
}

CVar NativeToFakeluaNil(State *state) {
    return Var{};
}

CVar NativeToFakeluaBool(State *state, bool val) {
    Var ret;
    ret.SetBool(val);
    return ret;
}

CVar NativeToFakeluaChar(State *state, char val) {
    return NativeToFakeluaIntHelper(val);
}

CVar NativeToFakeluaUchar(State *state, unsigned char val) {
    return NativeToFakeluaIntHelper(val);
}

CVar NativeToFakeluaShort(State *state, short val) {
    return NativeToFakeluaIntHelper(val);
}

CVar NativeToFakeluaUshort(State *state, unsigned short val) {
    return NativeToFakeluaIntHelper(val);
}

CVar NativeToFakeluaInt(State *state, int val) {
    return NativeToFakeluaIntHelper(val);
}

CVar NativeToFakeluaUint(State *state, unsigned int val) {
    return NativeToFakeluaIntHelper(val);
}

CVar NativeToFakeluaLong(State *state, long val) {
    return NativeToFakeluaIntHelper(val);
}

CVar NativeToFakeluaUlong(State *state, unsigned long val) {
    return NativeToFakeluaIntHelper(val);
}

CVar NativeToFakeluaLonglong(State *state, long long val) {
    return NativeToFakeluaIntHelper(val);
}

CVar NativeToFakeluaUlonglong(State *state, unsigned long long val) {
    return NativeToFakeluaIntHelper(val);
}

CVar NativeToFakeluaFloat(State *state, float val) {
    Var ret;
    ret.SetFloat(val);
    return ret;
}

CVar NativeToFakeluaDouble(State *state, double val) {
    Var ret;
    ret.SetFloat(val);
    return ret;
}

CVar NativeToFakeluaCstr(State *state, const char *val) {
    Var ret;
    // 防御性处理：外部传入 nullptr 时视为空字符串，避免 string_view 构造崩溃
    ret.SetTempString(state, val ? val : "");
    return ret;
}

CVar NativeToFakeluaStr(State *state, char *val) {
    return NativeToFakeluaCstr(state, val);
}

CVar NativeToFakeluaString(State *state, const std::string &val) {
    Var ret;
    ret.SetTempString(state, val);
    return ret;
}

CVar NativeToFakeluaStringView(State *state, const std::string_view &val) {
    Var ret;
    ret.SetTempString(state, val);
    return ret;
}

bool FakeluaToNativeBool(State *state, CVar val) {
    const auto &var_val = AsVar(val);
    if (LIKELY(var_val.Type() == VarType::Bool)) {
        return var_val.GetBool();
    }
    ThrowFakeluaException(std::format("FakeluaToNativeBool failed, type is {}", VarTypeToString(var_val.Type())));
}

template<typename T>
T FakeluaToNativeIntHelper(CVar val, const char *func_name) {
    const auto &var_val = AsVar(val);
    if (LIKELY(var_val.Type() == VarType::Int)) {
        return static_cast<T>(var_val.GetInt());
    }
    ThrowFakeluaException(std::format("{} failed, type is {}", func_name, VarTypeToString(var_val.Type())));
}

char FakeluaToNativeChar(State *state, CVar val) {
    return FakeluaToNativeIntHelper<char>(val, "FakeluaToNativeChar");
}

unsigned char FakeluaToNativeUchar(State *state, CVar val) {
    return FakeluaToNativeIntHelper<unsigned char>(val, "FakeluaToNativeUchar");
}

short FakeluaToNativeShort(State *state, CVar val) {
    return FakeluaToNativeIntHelper<short>(val, "FakeluaToNativeShort");
}

unsigned short FakeluaToNativeUshort(State *state, CVar val) {
    return FakeluaToNativeIntHelper<unsigned short>(val, "FakeluaToNativeUshort");
}

int FakeluaToNativeInt(State *state, CVar val) {
    return FakeluaToNativeIntHelper<int>(val, "FakeluaToNativeInt");
}

unsigned int FakeluaToNativeUint(State *state, CVar val) {
    return FakeluaToNativeIntHelper<unsigned int>(val, "FakeluaToNativeUint");
}

long FakeluaToNativeLong(State *state, CVar val) {
    return FakeluaToNativeIntHelper<long>(val, "FakeluaToNativeLong");
}

unsigned long FakeluaToNativeUlong(State *state, CVar val) {
    return FakeluaToNativeIntHelper<unsigned long>(val, "FakeluaToNativeUlong");
}

long long FakeluaToNativeLonglong(State *state, CVar val) {
    return FakeluaToNativeIntHelper<long long>(val, "FakeluaToNativeLonglong");
}

unsigned long long FakeluaToNativeUlonglong(State *state, CVar val) {
    return FakeluaToNativeIntHelper<unsigned long long>(val, "FakeluaToNativeUlonglong");
}

template<typename T>
T FakeluaToNativeFloatHelper(CVar val, const char *func_name) {
    const auto &var_val = AsVar(val);
    if (LIKELY(var_val.Type() == VarType::Float)) {
        return static_cast<T>(var_val.GetFloat());
    }
    if (UNLIKELY(var_val.Type() == VarType::Int)) {
        return static_cast<T>(var_val.GetInt());
    }
    ThrowFakeluaException(std::format("{} failed, type is {}", func_name, VarTypeToString(var_val.Type())));
}

float FakeluaToNativeFloat(State *state, CVar val) {
    return FakeluaToNativeFloatHelper<float>(val, "FakeluaToNativeFloat");
}

double FakeluaToNativeDouble(State *state, CVar val) {
    return FakeluaToNativeFloatHelper<double>(val, "FakeluaToNativeDouble");
}

std::string FakeluaToNativeString(State *state, CVar val) {
    const auto &var_val = AsVar(val);
    if (LIKELY(var_val.Type() == VarType::String || var_val.Type() == VarType::StringId)) {
        return std::string(var_val.GetString()->Str());
    }
    ThrowFakeluaException(std::format("FakeluaToNativeString failed, type is {}", VarTypeToString(var_val.Type())));
}

std::string_view FakeluaToNativeStringView(State *state, CVar val) {
    const auto &var_val = AsVar(val);
    if (LIKELY(var_val.Type() == VarType::String || var_val.Type() == VarType::StringId)) {
        return var_val.GetString()->Str();
    }
    ThrowFakeluaException(std::format("FakeluaToNativeStringview failed, type is {}", VarTypeToString(var_val.Type())));
}

// VarToVi: read CVar into VarInterface* bridge type. Tables are read-only —
// no VarTable methods needed, just direct field access on the C struct.
static void VarToVi(State *state, const CVar &src, VarInterface *dst) {
    const auto &var_val = AsVar(src);
    DEBUG_ASSERT(var_val.Type() >= VarType::Min && var_val.Type() <= VarType::Max);
    switch (var_val.Type()) {
        case VarType::Nil:
            dst->ViSetNil();
            break;
        case VarType::Bool:
            dst->ViSetBool(var_val.GetBool());
            break;
        case VarType::Int:
            dst->ViSetInt(var_val.GetInt());
            break;
        case VarType::Float:
            dst->ViSetFloat(var_val.GetFloat());
            break;
        case VarType::String:
        case VarType::StringId:
            dst->ViSetString(var_val.GetString()->Str());
            break;
        case VarType::Table: {
            const auto table = var_val.GetTable();
            const uint32_t count = table->count_;
            std::vector<std::pair<VarInterface *, VarInterface *>> kvs;
            kvs.reserve(table->spec_count + count);
            const auto &new_func = GetVarInterfaceNewFunc(state);
            auto alloc_vi = [&new_func]() -> VarInterface * { return new_func ? new_func() : new SimpleVarImpl(); };

            if (table->spec_count > 0) {
                const auto *sk = reinterpret_cast<const Var *>(table->spec_keys);
                const auto *sv = reinterpret_cast<const Var *>(table->spec_vals);
                for (uint32_t i = 0; i < table->spec_count; ++i) {
                    auto key_item = alloc_vi();
                    auto val_item = alloc_vi();
                    VarToVi(state, sk[i], key_item);
                    VarToVi(state, sv[i], val_item);
                    kvs.emplace_back(key_item, val_item);
                }
            }
            if (const uint32_t *al = table->active_list_; al == nullptr) {
                for (uint32_t i = 0; i < count; ++i) {
                    const auto &e = table->quick_data_[i];
                    auto key_item = alloc_vi();
                    auto val_item = alloc_vi();
                    VarToVi(state, e.key, key_item);
                    VarToVi(state, e.val, val_item);
                    kvs.emplace_back(key_item, val_item);
                }
            } else {
                for (uint32_t i = 0; i < count; ++i) {
                    const auto &e = table->nodes_[al[i]].entry;
                    auto key_item = alloc_vi();
                    auto val_item = alloc_vi();
                    VarToVi(state, e.key, key_item);
                    VarToVi(state, e.val, val_item);
                    kvs.emplace_back(key_item, val_item);
                }
            }
            dst->ViSetTable(kvs);
            break;
        }
        case VarType::Closure:
            ThrowFakeluaException("VarToVi failed: Closure type is not supported");
            break;
        case VarType::Multi:
            ThrowFakeluaException("VarToVi failed: Multi-return type is not supported");
            break;
    }
}

VarInterface *FakeluaToNativeObj(State *state, CVar val) {
    const auto &new_func = GetVarInterfaceNewFunc(state);
    const auto ret = new_func ? new_func() : new SimpleVarImpl();
    VarToVi(state, val, ret);
    return ret;
}

CVar NativeToFakeluaNativeObject(State *state, const NativeObject *obj) {
    if (!obj) {
        return NativeToFakeluaNil(state);
    }
    return obj->Wrap(state);
}

void *GetFuncAddr(State *state, JITType type, const std::string_view &name, int &arg_count, bool &is_vararg) {
    const auto ret = state->GetVM().GetFunction({name.data(), name.size()});
    if (UNLIKELY(ret.Empty())) {
        return nullptr;
    }
    arg_count = ret.GetArgCount();
    is_vararg = ret.IsVararg();
    return ret.GetAddr(type);
}

[[noreturn]] void ThrowInterFakeluaException(const std::string &msg) {
    ThrowFakeluaException(msg);
}

int GetReentrantCount(State *state) {
    return state->GetReentrantCount();
}

void AddReentrantCount(State *state) {
    state->AddReentrantCount();
}

void SubReentrantCount(State *state) {
    state->SubReentrantCount();
}

void Reset(State *state) {
    state->Reset();
}

}// namespace inter

State *FakeluaNewState(const StateConfig &cfg) {
    return new State(cfg);
}

void FakeluaDeleteState(State *state) {
    delete state;
}

void CompileFile(State *state, const std::string &filename, const CompileConfig &cfg) {
    state->GetCompiler().CompileFile(filename, cfg);
}

void CompileString(State *state, const std::string &str, const CompileConfig &cfg) {
    state->GetCompiler().CompileString(str, cfg);
}

std::string GetLastRecordedCCode(State *state) {
    return state->GetCompiler().GetLastRecordedCCode();
}

void SetVarInterfaceNewFunc(State *state, const std::function<VarInterface *()> &func) {
    state->SetVarInterfaceNewFunc(func);
}

std::function<VarInterface *()> &GetVarInterfaceNewFunc(State *state) {
    return state->GetVarInterfaceNewFunc();
}

void SetDebugLogLevel(int level) {
    SetLogLevel(static_cast<LogLevel>(level));
}

// ─────────────────────────────────────────────────────────────────────────────
// RegisterNativeFunction — 注册 C++ 函数供 lua 侧通过名字调用
// ─────────────────────────────────────────────────────────────────────────────
void RegisterNativeFunction(State *s, const std::string &name, int arg_count, bool is_vararg, NativeFuncCallback callback) {
    s->GetVM().RegisterNativeFunction(name, arg_count, is_vararg, std::move(callback));
}

void RegisterNativeVarFunction(State *s, const std::string &name, int arg_count, bool is_vararg, NativeVarFuncCallback callback) {
    auto safe_cb = std::move(callback);
    RegisterNativeFunction(s, name, arg_count, is_vararg, [safe_cb](State *state, CVar *args, int n) -> CVar {
        if (!safe_cb) {
            return inter::NativeToFakeluaNil(state);
        }
        std::vector<VarInterface *> vi_args;
        vi_args.reserve(n);
        for (int i = 0; i < n; ++i) {
            CVar arg_i = inter::GetNativeArg(state, args, n, i);
            vi_args.push_back(inter::FakeluaToNativeObj(state, arg_i));
        }
        VarInterface *res_vi = safe_cb(state, vi_args);
        return inter::NativeToFakeluaVarInterface(state, res_vi);
    });
}

namespace inter {

static CVar ViToVar(State *state, VarInterface *src) {
    if (!src || src->ViGetType() == VarInterface::Type::NIL) {
        return NativeToFakeluaNil(state);
    }
    switch (src->ViGetType()) {
        case VarInterface::Type::BOOL:
            return NativeToFakeluaBool(state, src->ViGetBool());
        case VarInterface::Type::INT:
            return NativeToFakeluaInt(state, src->ViGetInt());
        case VarInterface::Type::FLOAT:
            return NativeToFakeluaDouble(state, src->ViGetFloat());
        case VarInterface::Type::STRING:
            return NativeToFakeluaStringView(state, src->ViGetString());
        case VarInterface::Type::TABLE: {
            size_t count = src->ViGetTableSize();
            auto &alloc = state->GetHeap().GetAllocator(false /* temp */);
            auto *vtbl = static_cast<VarTable *>(alloc.Alloc(sizeof(VarTable)));
            *vtbl = VarTable{};
            for (auto &qd: vtbl->quick_data_) {
                qd.key.type_ = static_cast<int>(VarType::Nil);
                qd.val.type_ = static_cast<int>(VarType::Nil);
            }
            vtbl->free_list_idx_ = VarTable::INVALID_INDEX;

            if (count > 0) {
                uint32_t bucket_count = 1;
                while (bucket_count < count) bucket_count <<= 1;
                vtbl->bucket_count_ = bucket_count;
                vtbl->nodes_ = static_cast<VarTable::TableNode *>(alloc.Alloc(sizeof(VarTable::TableNode) * bucket_count));
                vtbl->active_list_ = static_cast<uint32_t *>(alloc.Alloc(sizeof(uint32_t) * bucket_count));
                for (uint32_t i = 0; i < bucket_count; ++i) {
                    vtbl->nodes_[i].entry.key.type_ = static_cast<int>(VarType::Nil);
                    vtbl->nodes_[i].entry.val.type_ = static_cast<int>(VarType::Nil);
                    vtbl->nodes_[i].next = VarTable::INVALID_INDEX;
                }
                for (size_t i = 0; i < count; ++i) {
                    auto kv = src->ViGetTableKv(static_cast<int>(i));
                    CVar k = ViToVar(state, kv.first);
                    CVar v = ViToVar(state, kv.second);
                    static_cast<CVar &>(vtbl->nodes_[i].entry.key) = k;
                    static_cast<CVar &>(vtbl->nodes_[i].entry.val) = v;
                    vtbl->active_list_[i] = static_cast<uint32_t>(i);
                }
                vtbl->count_ = static_cast<uint32_t>(count);
            }
            CVar tbl_cvar{};
            tbl_cvar.type_ = static_cast<int>(VarType::Table);
            tbl_cvar.data_.t = vtbl;
            return tbl_cvar;
        }
        default:
            break;
    }
    return NativeToFakeluaNil(state);
}

CVar NativeToFakeluaVarInterface(State *state, VarInterface *v) {
    return ViToVar(state, v);
}

void ThrowIfMultiCVar(const CVar &v) {
    if (UNLIKELY(v.type_ == static_cast<int>(VarType::Multi))) {
        ThrowFakeluaException("NativeToFakelua: CVar with Multi type is not allowed, use raw values instead");
    }
}

CVar DispatchCall(void *addr, const CVar *arg_arr, int arg_count) {
    switch (arg_count) {
#define DCASE(N)                                                                                                                                                                                       \
    case N:                                                                                                                                                                                            \
        return reinterpret_cast<CVar (*)(VarClosure * DISPATCH_CVAR_##N)>(addr)(nullptr DISPATCH_ARG_##N);

        DCASE(0)
        DCASE(1)
        DCASE(2)
        DCASE(3)
        DCASE(4) DCASE(5) DCASE(6) DCASE(7) DCASE(8) DCASE(9) DCASE(10) DCASE(11) DCASE(12) DCASE(13) DCASE(14) DCASE(15) DCASE(16) DCASE(17) DCASE(18) DCASE(19) DCASE(20) DCASE(21) DCASE(22)
                DCASE(23) DCASE(24) DCASE(25) DCASE(26) DCASE(27) DCASE(28) DCASE(29) DCASE(30) DCASE(31) DCASE(32)

#undef DCASE
#include "util/dispatch_macro_undef.h"
                        default : ThrowFakeluaException(std::format("DispatchCall: arg_count {} out of range", arg_count));
    }
    __builtin_unreachable();
}

CVar AllocMultiCVar(State *s, int count) {
    VarMulti *m = VarMulti::AllocTemp(s, count);
    for (int i = 0; i < count; ++i) {
        m->GetVars()[i] = CVar{static_cast<int>(VarType::Nil)};
    }
    CVar result;
    result.type_ = static_cast<int>(VarType::Multi);
    result.flag_ = 0;
    result.data_.m = m;
    return result;
}

void SetMultiCVarElement(CVar &multi, int idx, CVar val) {
    if (UNLIKELY(multi.type_ != static_cast<int>(VarType::Multi))) {
        ThrowFakeluaException("SetMultiCVarElement: CVar is not Multi");
    }
    VarMulti *m = multi.data_.m;
    if (UNLIKELY(idx < 0 || idx >= static_cast<int>(m->GetCount()))) {
        ThrowFakeluaException(std::format("SetMultiCVarElement: idx {} out of range", idx));
    }
    m->GetVars()[idx] = val;
}

CVar GetMultiCVarElement(const CVar &multi, int idx) {
    if (UNLIKELY(multi.type_ != static_cast<int>(VarType::Multi))) {
        ThrowFakeluaException("GetMultiCVarElement: CVar is not Multi");
    }
    VarMulti *m = multi.data_.m;
    if (UNLIKELY(idx < 0 || idx >= static_cast<int>(m->GetCount()))) {
        ThrowFakeluaException(std::format("GetMultiCVarElement: idx {} out of range", idx));
    }
    return m->GetVars()[idx];
}

int GetMultiCVarCount(const CVar &multi) {
    if (UNLIKELY(multi.type_ != static_cast<int>(VarType::Multi))) {
        ThrowFakeluaException("GetMultiCVarCount: CVar is not Multi");
    }
    return static_cast<int>(multi.data_.m->GetCount());
}

CVar GetNativeArg(State *s, CVar *args, int n, int idx) {
    (void) s;
    CVar res{static_cast<int>(VarType::Nil)};
    if (n > 0 && args[0].type_ == static_cast<int>(VarType::Multi)) {
        VarMulti *m = args[0].data_.m;
        if (m && idx < static_cast<int>(m->GetCount())) {
            res = m->GetVars()[idx];
        }
    } else if (idx < n) {
        res = args[idx];
    }
    while (res.type_ == static_cast<int>(VarType::Multi)) {
        VarMulti *m = res.data_.m;
        if (m && m->GetCount() > 0) {
            res = m->GetVars()[0];
        } else {
            res = CVar{static_cast<int>(VarType::Nil)};
            break;
        }
    }
    return res;
}

}// namespace inter

}// namespace fakelua
