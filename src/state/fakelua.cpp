#include "fakelua.h"
#include "state/state.h"
#include "util/common.h"
#include "var/var_string.h"
#include "var/var_table.h"

namespace fakelua {

namespace inter {

CVar NativeToFakeluaNil(State *state) {
    return Var{};
}

CVar NativeToFakeluaBool(State *state, bool val) {
    Var ret;
    ret.SetBool(val);
    return ret;
}

CVar NativeToFakeluaChar(State *state, char val) {
    Var ret;
    ret.SetInt(val);
    return ret;
}

CVar NativeToFakeluaUchar(State *state, unsigned char val) {
    Var ret;
    ret.SetInt(val);
    return ret;
}

CVar NativeToFakeluaShort(State *state, short val) {
    Var ret;
    ret.SetInt(val);
    return ret;
}

CVar NativeToFakeluaUshort(State *state, unsigned short val) {
    Var ret;
    ret.SetInt(val);
    return ret;
}

CVar NativeToFakeluaInt(State *state, int val) {
    Var ret;
    ret.SetInt(val);
    return ret;
}

CVar NativeToFakeluaUint(State *state, unsigned int val) {
    Var ret;
    ret.SetInt(val);
    return ret;
}

CVar NativeToFakeluaLong(State *state, long val) {
    Var ret;
    ret.SetInt(val);
    return ret;
}

CVar NativeToFakeluaUlong(State *state, unsigned long val) {
    Var ret;
    ret.SetInt(static_cast<int64_t>(val));
    return ret;
}

CVar NativeToFakeluaLonglong(State *state, long long val) {
    Var ret;
    ret.SetInt(val);
    return ret;
}

CVar NativeToFakeluaUlonglong(State *state, unsigned long long val) {
    Var ret;
    ret.SetInt(static_cast<int64_t>(val));
    return ret;
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
    Var ret;
    // 防御性处理：外部传入 nullptr 时视为空字符串，避免 string_view 构造崩溃
    ret.SetTempString(state, val ? val : "");
    return ret;
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
    if (var_val.Type() == VarType::Bool) {
        return var_val.GetBool();
    }
    ThrowFakeluaException(std::format("FakeluaToNativeBool failed, type is {}", VarTypeToString(var_val.Type())));
}

char FakeluaToNativeChar(State *state, CVar val) {
    const auto var_val = reinterpret_cast<Var &>(val);
    if (var_val.Type() == VarType::Int) {
        return static_cast<char>(var_val.GetInt());
    }
    ThrowFakeluaException(std::format("FakeluaToNativeChar failed, type is {}", VarTypeToString(var_val.Type())));
}

unsigned char FakeluaToNativeUchar(State *state, CVar val) {
    const auto var_val = reinterpret_cast<Var &>(val);
    if (var_val.Type() == VarType::Int) {
        return static_cast<unsigned char>(var_val.GetInt());
    }
    ThrowFakeluaException(std::format("FakeluaToNativeUchar failed, type is {}", VarTypeToString(var_val.Type())));
}

short FakeluaToNativeShort(State *state, CVar val) {
    const auto var_val = reinterpret_cast<Var &>(val);
    if (var_val.Type() == VarType::Int) {
        return static_cast<short>(var_val.GetInt());
    }
    ThrowFakeluaException(std::format("FakeluaToNativeShort failed, type is {}", VarTypeToString(var_val.Type())));
}

unsigned short FakeluaToNativeUshort(State *state, CVar val) {
    const auto var_val = reinterpret_cast<Var &>(val);
    if (var_val.Type() == VarType::Int) {
        return static_cast<unsigned short>(var_val.GetInt());
    }
    ThrowFakeluaException(std::format("FakeluaToNativeUshort failed, type is {}", VarTypeToString(var_val.Type())));
}

int FakeluaToNativeInt(State *state, CVar val) {
    const auto var_val = reinterpret_cast<Var &>(val);
    if (var_val.Type() == VarType::Int) {
        return static_cast<int>(var_val.GetInt());
    }
    ThrowFakeluaException(std::format("FakeluaToNativeInt failed, type is {}", VarTypeToString(var_val.Type())));
}

unsigned int FakeluaToNativeUint(State *state, CVar val) {
    const auto var_val = reinterpret_cast<Var &>(val);
    if (var_val.Type() == VarType::Int) {
        return static_cast<unsigned int>(var_val.GetInt());
    }
    ThrowFakeluaException(std::format("FakeluaToNativeUint failed, type is {}", VarTypeToString(var_val.Type())));
}

long FakeluaToNativeLong(State *state, CVar val) {
    const auto var_val = reinterpret_cast<Var &>(val);
    if (var_val.Type() == VarType::Int) {
        return var_val.GetInt();
    }
    ThrowFakeluaException(std::format("FakeluaToNativeLong failed, type is {}", VarTypeToString(var_val.Type())));
}

unsigned long FakeluaToNativeUlong(State *state, CVar val) {
    const auto var_val = reinterpret_cast<Var &>(val);
    if (var_val.Type() == VarType::Int) {
        return var_val.GetInt();
    }
    ThrowFakeluaException(std::format("FakeluaToNativeUlong failed, type is {}", VarTypeToString(var_val.Type())));
}

long long FakeluaToNativeLonglong(State *state, CVar val) {
    const auto var_val = reinterpret_cast<Var &>(val);
    if (var_val.Type() == VarType::Int) {
        return var_val.GetInt();
    }
    ThrowFakeluaException(std::format("FakeluaToNativeLonglong failed, type is {}", VarTypeToString(var_val.Type())));
}

unsigned long long FakeluaToNativeUlonglong(State *state, CVar val) {
    const auto var_val = reinterpret_cast<Var &>(val);
    if (var_val.Type() == VarType::Int) {
        return static_cast<unsigned long long>(var_val.GetInt());
    }
    ThrowFakeluaException(std::format("FakeluaToNativeUlonglong failed, type is {}", VarTypeToString(var_val.Type())));
}

float FakeluaToNativeFloat(State *state, CVar val) {
    const auto var_val = reinterpret_cast<Var &>(val);
    if (var_val.Type() == VarType::Float) {
        return static_cast<float>(var_val.GetFloat());
    }
    if (var_val.Type() == VarType::Int) {
        return static_cast<float>(var_val.GetInt());
    }
    ThrowFakeluaException(std::format("FakeluaToNativeFloat failed, type is {}", VarTypeToString(var_val.Type())));
}

double FakeluaToNativeDouble(State *state, CVar val) {
    const auto var_val = reinterpret_cast<Var &>(val);
    if (var_val.Type() == VarType::Float) {
        return var_val.GetFloat();
    }
    if (var_val.Type() == VarType::Int) {
        return static_cast<double>(var_val.GetInt());
    }
    ThrowFakeluaException(std::format("FakeluaToNativeDouble failed, type is {}", VarTypeToString(var_val.Type())));
}

std::string FakeluaToNativeString(State *state, CVar val) {
    const auto var_val = reinterpret_cast<Var &>(val);
    if (var_val.Type() == VarType::String || var_val.Type() == VarType::StringId) {
        return std::string(var_val.GetString()->Str());
    }
    ThrowFakeluaException(std::format("FakeluaToNativeString failed, type is {}", VarTypeToString(var_val.Type())));
}

std::string_view FakeluaToNativeStringView(State *state, CVar val) {
    const auto var_val = reinterpret_cast<Var &>(val);
    if (var_val.Type() == VarType::String || var_val.Type() == VarType::StringId) {
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
            dst->ViSetString(var_val.GetString()->Str());
            break;
        case VarType::StringId:
            dst->ViSetString(var_val.GetString()->Str());
            break;
        case VarType::Table: {
            std::vector<std::pair<VarInterface *, VarInterface *>> kvs;
            const auto table = var_val.GetTable();
            const uint32_t count = static_cast<uint32_t>(table->Size());
            const VarTable::VarEntry *quick_data = table->GetQuickData();
            const auto *nodes = table->GetNodes();
            if (const uint32_t *active_list = table->GetActiveList(); active_list == nullptr) {// 快速路径
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
            } else {// 哈希表路径
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
    }
}

VarInterface *FakeluaToNativeObj(State *state, CVar val) {
    const auto ret = GetVarInterfaceNewFunc(state)();
    VarToVi(state, val, ret);
    return ret;
}

void *GetFuncAddr(State *state, JITType type, const std::string_view &name, int &arg_count) {
    const auto ret = state->GetVM().GetFunction({name.data(), name.size()});
    if (ret.Empty()) {
        return nullptr;
    }
    arg_count = ret.GetArgCount();
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

}// namespace fakelua
