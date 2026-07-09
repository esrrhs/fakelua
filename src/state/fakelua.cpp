#include "compile/compile_common.h"
#include "fakelua.h"
#include "state/state.h"
#include "util/common.h"
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

Var ViToVar(State *state, const VarInterface *src) {
    DEBUG_ASSERT(src->ViGetType() >= VarInterface::Type::MIN && src->ViGetType() <= VarInterface::Type::MAX);
    Var ret;
    switch (src->ViGetType()) {
        case VarInterface::Type::NIL:
            ret.SetNil();
            break;
        case VarInterface::Type::BOOL:
            ret.SetBool(src->ViGetBool());
            break;
        case VarInterface::Type::INT:
            ret.SetInt(src->ViGetInt());
            break;
        case VarInterface::Type::FLOAT:
            ret.SetFloat(src->ViGetFloat());
            break;
        case VarInterface::Type::STRING:
            ret.SetTempString(state, src->ViGetString());
            break;
        case VarInterface::Type::TABLE:
            ret.SetTable(state);
            for (int i = 0; i < static_cast<int>(src->ViGetTableSize()); ++i) {
                const auto [fst, snd] = src->ViGetTableKv(i);
                auto key = ViToVar(state, fst);
                auto val = ViToVar(state, snd);
                ret.GetTable()->Set(state, key, val, true);
            }
            break;
        default:
            ThrowFakeluaException(std::format("ViToVar failed, unknown type {}", static_cast<int>(src->ViGetType())));
    }
    return ret;
}

CVar NativeToFakeluaObj(State *state, const VarInterface *val) {
    return ViToVar(state, val);
}

bool FakeluaToNativeBool(State *state, CVar val) {
    const auto var_val = reinterpret_cast<Var &>(val);
    if (LIKELY(var_val.Type() == VarType::Bool)) {
        return var_val.GetBool();
    }
    ThrowFakeluaException(std::format("FakeluaToNativeBool failed, type is {}", VarTypeToString(var_val.Type())));
}

template<typename T>
T FakeluaToNativeIntHelper(CVar val, const char *func_name) {
    const auto &var_val = reinterpret_cast<const Var &>(val);
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
    const auto &var_val = reinterpret_cast<const Var &>(val);
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
    const auto var_val = reinterpret_cast<Var &>(val);
    if (LIKELY(var_val.Type() == VarType::String || var_val.Type() == VarType::StringId)) {
        return std::string(var_val.GetString()->Str());
    }
    ThrowFakeluaException(std::format("FakeluaToNativeString failed, type is {}", VarTypeToString(var_val.Type())));
}

std::string_view FakeluaToNativeStringView(State *state, CVar val) {
    const auto var_val = reinterpret_cast<Var &>(val);
    if (LIKELY(var_val.Type() == VarType::String || var_val.Type() == VarType::StringId)) {
        return var_val.GetString()->Str();
    }
    ThrowFakeluaException(std::format("FakeluaToNativeStringview failed, type is {}", VarTypeToString(var_val.Type())));
}

void VarToVi(State *state, CVar src, VarInterface *dst) {
    const auto var_val = reinterpret_cast<Var &>(src);
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
            std::vector<std::pair<VarInterface *, VarInterface *>> kvs;
            const auto table = var_val.GetTable();
            // 先遍历 spec 字段
            if (table->GetSpecCount() > 0) {
                const auto *spec_keys = table->GetSpecKeys();
                const auto *spec_vals = table->GetSpecVals();
                for (uint32_t i = 0; i < table->GetSpecCount(); ++i) {
                    auto key_item = GetVarInterfaceNewFunc(state)();
                    auto val_item = GetVarInterfaceNewFunc(state)();
                    VarToVi(state, spec_keys[i], key_item);
                    VarToVi(state, spec_vals[i], val_item);
                    kvs.emplace_back(key_item, val_item);
                }
            }
            // 再遍历 hash 字段
            const uint32_t count = table->GetHashCount();
            const VarTable::VarEntry *quick_data = table->GetQuickData();
            const auto *nodes = table->GetNodes();
            if (const uint32_t *active_list = table->GetActiveList(); active_list == nullptr) {
                for (uint32_t i = 0; i < count; ++i) {
                    const auto &entry = quick_data[i];
                    const auto key = entry.key;
                    const auto val = entry.val;
                    auto key_item = GetVarInterfaceNewFunc(state)();
                    auto val_item = GetVarInterfaceNewFunc(state)();
                    VarToVi(state, key, key_item);
                    VarToVi(state, val, val_item);
                    kvs.emplace_back(key_item, val_item);
                }
            } else {
                for (uint32_t i = 0; i < count; ++i) {
                    const auto &entry = nodes[active_list[i]].entry;
                    const auto key = entry.key;
                    const auto val = entry.val;
                    auto key_item = GetVarInterfaceNewFunc(state)();
                    auto val_item = GetVarInterfaceNewFunc(state)();
                    VarToVi(state, key, key_item);
                    VarToVi(state, val, val_item);
                    kvs.emplace_back(key_item, val_item);
                }
            }
            dst->ViSetTable(kvs);
            break;
        }
        case VarType::Multi: {
            ThrowFakeluaException("VarToVi failed: Multi-return type is not supported in host VarInterface");
            break;
        }
    }
}

VarInterface *FakeluaToNativeObj(State *state, CVar val) {
    const auto ret = GetVarInterfaceNewFunc(state)();
    VarToVi(state, val, ret);
    return ret;
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

// ── 编译管线接口 ──────────────────────────────────────────────

// CompileResult pimpl 方法实现——定义在 fakelua.cpp 中以避免在公共头文件中 include 内部类型。
static const std::string kEmptyString;

CompileResult::CompileResult() : impl_(std::make_shared<CompileResultImpl>()) {
}

CompileResult::CompileResult(CompileResult &&other) noexcept = default;
CompileResult &CompileResult::operator=(CompileResult &&other) noexcept = default;
CompileResult::~CompileResult() = default;

const std::string &CompileResult::GetCCode() const {
    if (!impl_) return kEmptyString;
    return GetCompileResultImpl(*this).GetCCode();
}

const std::string &CompileResult::GetRecordedCCode() const {
    if (!impl_) return kEmptyString;
    return GetCompileResultImpl(*this).GetRecordedCCode();
}

CompileResult CompileFile(State *state, const std::string &filename, const CompileConfig &cfg) {
    return state->GetCompiler().CompileFile(filename, cfg);
}

CompileResult CompileString(State *state, const std::string &str, const CompileConfig &cfg) {
    return state->GetCompiler().CompileString(str, cfg);
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

namespace inter {

void ThrowIfMultiCVar(const CVar &v) {
    if (UNLIKELY(v.type_ == static_cast<int>(VarType::Multi))) {
        ThrowFakeluaException("NativeToFakelua: CVar with Multi type is not allowed, use raw values instead");
    }
}

CVar DispatchCall(void *addr, const CVar *arg_arr, int arg_count) {
#define DCVAR_0
#define DCVAR_1 CVar
#define DCVAR_2 DCVAR_1, CVar
#define DCVAR_3 DCVAR_2, CVar
#define DCVAR_4 DCVAR_3, CVar
#define DCVAR_5 DCVAR_4, CVar
#define DCVAR_6 DCVAR_5, CVar
#define DCVAR_7 DCVAR_6, CVar
#define DCVAR_8 DCVAR_7, CVar
#define DCVAR_9 DCVAR_8, CVar
#define DCVAR_10 DCVAR_9, CVar
#define DCVAR_11 DCVAR_10, CVar
#define DCVAR_12 DCVAR_11, CVar
#define DCVAR_13 DCVAR_12, CVar
#define DCVAR_14 DCVAR_13, CVar
#define DCVAR_15 DCVAR_14, CVar
#define DCVAR_16 DCVAR_15, CVar
#define DCVAR_17 DCVAR_16, CVar
#define DCVAR_18 DCVAR_17, CVar
#define DCVAR_19 DCVAR_18, CVar
#define DCVAR_20 DCVAR_19, CVar
#define DCVAR_21 DCVAR_20, CVar
#define DCVAR_22 DCVAR_21, CVar
#define DCVAR_23 DCVAR_22, CVar
#define DCVAR_24 DCVAR_23, CVar
#define DCVAR_25 DCVAR_24, CVar
#define DCVAR_26 DCVAR_25, CVar
#define DCVAR_27 DCVAR_26, CVar
#define DCVAR_28 DCVAR_27, CVar
#define DCVAR_29 DCVAR_28, CVar
#define DCVAR_30 DCVAR_29, CVar
#define DCVAR_31 DCVAR_30, CVar
#define DCVAR_32 DCVAR_31, CVar

#define DARG_0
#define DARG_1 arg_arr[0]
#define DARG_2 DARG_1, arg_arr[1]
#define DARG_3 DARG_2, arg_arr[2]
#define DARG_4 DARG_3, arg_arr[3]
#define DARG_5 DARG_4, arg_arr[4]
#define DARG_6 DARG_5, arg_arr[5]
#define DARG_7 DARG_6, arg_arr[6]
#define DARG_8 DARG_7, arg_arr[7]
#define DARG_9 DARG_8, arg_arr[8]
#define DARG_10 DARG_9, arg_arr[9]
#define DARG_11 DARG_10, arg_arr[10]
#define DARG_12 DARG_11, arg_arr[11]
#define DARG_13 DARG_12, arg_arr[12]
#define DARG_14 DARG_13, arg_arr[13]
#define DARG_15 DARG_14, arg_arr[14]
#define DARG_16 DARG_15, arg_arr[15]
#define DARG_17 DARG_16, arg_arr[16]
#define DARG_18 DARG_17, arg_arr[17]
#define DARG_19 DARG_18, arg_arr[18]
#define DARG_20 DARG_19, arg_arr[19]
#define DARG_21 DARG_20, arg_arr[20]
#define DARG_22 DARG_21, arg_arr[21]
#define DARG_23 DARG_22, arg_arr[22]
#define DARG_24 DARG_23, arg_arr[23]
#define DARG_25 DARG_24, arg_arr[24]
#define DARG_26 DARG_25, arg_arr[25]
#define DARG_27 DARG_26, arg_arr[26]
#define DARG_28 DARG_27, arg_arr[27]
#define DARG_29 DARG_28, arg_arr[28]
#define DARG_30 DARG_29, arg_arr[29]
#define DARG_31 DARG_30, arg_arr[30]
#define DARG_32 DARG_31, arg_arr[31]

#define DCASE(N)                                                                                                                                                                                       \
    case N:                                                                                                                                                                                            \
        return reinterpret_cast<CVar (*)(DCVAR_##N)>(addr)(DARG_##N);

    switch (arg_count) {
        DCASE(0)
        DCASE(1) DCASE(2) DCASE(3) DCASE(4) DCASE(5) DCASE(6) DCASE(7) DCASE(8) DCASE(9) DCASE(10) DCASE(11) DCASE(12) DCASE(13) DCASE(14) DCASE(15) DCASE(16) DCASE(17) DCASE(18) DCASE(19) DCASE(20)
                DCASE(21) DCASE(22) DCASE(23) DCASE(24) DCASE(25) DCASE(26) DCASE(27) DCASE(28) DCASE(29) DCASE(30) DCASE(31) DCASE(32) default
            : ThrowFakeluaException(std::format("DispatchCall: arg_count {} out of range", arg_count));
    }
#undef DCASE
#undef DCVAR_0
#undef DCVAR_1
#undef DCVAR_2
#undef DCVAR_3
#undef DCVAR_4
#undef DCVAR_5
#undef DCVAR_6
#undef DCVAR_7
#undef DCVAR_8
#undef DCVAR_9
#undef DCVAR_10
#undef DCVAR_11
#undef DCVAR_12
#undef DCVAR_13
#undef DCVAR_14
#undef DCVAR_15
#undef DCVAR_16
#undef DCVAR_17
#undef DCVAR_18
#undef DCVAR_19
#undef DCVAR_20
#undef DCVAR_21
#undef DCVAR_22
#undef DCVAR_23
#undef DCVAR_24
#undef DCVAR_25
#undef DCVAR_26
#undef DCVAR_27
#undef DCVAR_28
#undef DCVAR_29
#undef DCVAR_30
#undef DCVAR_31
#undef DCVAR_32
#undef DARG_0
#undef DARG_1
#undef DARG_2
#undef DARG_3
#undef DARG_4
#undef DARG_5
#undef DARG_6
#undef DARG_7
#undef DARG_8
#undef DARG_9
#undef DARG_10
#undef DARG_11
#undef DARG_12
#undef DARG_13
#undef DARG_14
#undef DARG_15
#undef DARG_16
#undef DARG_17
#undef DARG_18
#undef DARG_19
#undef DARG_20
#undef DARG_21
#undef DARG_22
#undef DARG_23
#undef DARG_24
#undef DARG_25
#undef DARG_26
#undef DARG_27
#undef DARG_28
#undef DARG_29
#undef DARG_30
#undef DARG_31
#undef DARG_32
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

}// namespace inter

}// namespace fakelua
