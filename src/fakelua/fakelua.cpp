#include "fakelua.h"
#include "state/State.h"
#include "util/common.h"
#include "var/var_table.h"

namespace fakelua {

namespace inter {

CVar NativeToFakeluaNil(State *s) {
    return Var{};
}

CVar NativeToFakeluaBool(State *s, bool v) {
    Var ret;
    ret.SetBool(v);
    return ret;
}

CVar NativeToFakeluaChar(State *s, char v) {
    Var ret;
    ret.SetInt(v);
    return ret;
}

CVar NativeToFakeluaUchar(State *s, unsigned char v) {
    Var ret;
    ret.SetInt(v);
    return ret;
}

CVar NativeToFakeluaShort(State *s, short v) {
    Var ret;
    ret.SetInt(v);
    return ret;
}

CVar NativeToFakeluaUshort(State *s, unsigned short v) {
    Var ret;
    ret.SetInt(v);
    return ret;
}

CVar NativeToFakeluaInt(State *s, int v) {
    Var ret;
    ret.SetInt(v);
    return ret;
}

CVar NativeToFakeluaUint(State *s, unsigned int v) {
    Var ret;
    ret.SetInt(v);
    return ret;
}

CVar NativeToFakeluaLong(State *s, long v) {
    Var ret;
    ret.SetInt(v);
    return ret;
}

CVar NativeToFakeluaUlong(State *s, unsigned long v) {
    Var ret;
    ret.SetInt(v);
    return ret;
}

CVar NativeToFakeluaLonglong(State *s, long long v) {
    Var ret;
    ret.SetInt(v);
    return ret;
}

CVar NativeToFakeluaUlonglong(State *s, unsigned long long v) {
    Var ret;
    ret.SetInt(static_cast<int64_t>(v));
    return ret;
}

CVar NativeToFakeluaFloat(State *s, float v) {
    Var ret;
    ret.SetFloat(v);
    return ret;
}

CVar NativeToFakeluaDouble(State *s, double v) {
    Var ret;
    ret.SetFloat(v);
    return ret;
}

CVar NativeToFakeluaCstr(State *s, const char *v) {
    Var ret;
    ret.SetString(s, v);
    return ret;
}

CVar NativeToFakeluaStr(State *s, char *v) {
    Var ret;
    ret.SetString(s, v);
    return ret;
}

CVar NativeToFakeluaString(State *s, const std::string &v) {
    Var ret;
    ret.SetString(s, v);
    return ret;
}

CVar NativeToFakeluaStringView(State *s, const std::string_view &v) {
    Var ret;
    ret.SetString(s, v);
    return ret;
}

Var ViToVar(State *s, const VarInterface *src) {
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
            ret.SetString(s, src->ViGetString());
            break;
        case VarInterface::Type::TABLE:
            ret.SetTable(s);
            for (int i = 0; i < static_cast<int>(src->ViGetTableSize()); ++i) {
                const auto [fst, snd] = src->ViGetTableKv(i);
                auto k = ViToVar(s, fst);
                auto v = ViToVar(s, snd);
                ret.GetTable()->Set(k, v, true);
            }
            break;
        default:
            ThrowFakeluaException(std::format("ViToVar failed, unknown type {}", static_cast<int>(src->ViGetType())));
    }
    return ret;
}

CVar NativeToFakeluaObj(State *s, const VarInterface *v) {
    return ViToVar(s, v);
}

bool FakeluaToNativeBool(State *s, CVar v) {
    const auto vv = static_cast<Var &>(v);
    if (vv.Type() == VarType::Bool) {
        return vv.GetBool();
    }
    ThrowFakeluaException(std::format("FakeluaToNativeBool failed, type is {}", VarTypeToString(vv.Type())));
}

char FakeluaToNativeChar(State *s, CVar v) {
    const auto vv = static_cast<Var &>(v);
    if (vv.Type() == VarType::Int) {
        return static_cast<char>(vv.GetInt());
    }
    ThrowFakeluaException(std::format("FakeluaToNativeChar failed, type is {}", VarTypeToString(vv.Type())));
}

unsigned char FakeluaToNativeUchar(State *s, CVar v) {
    const auto vv = static_cast<Var &>(v);
    if (vv.Type() == VarType::Int) {
        return static_cast<unsigned char>(vv.GetInt());
    }
    ThrowFakeluaException(std::format("FakeluaToNativeUchar failed, type is {}", VarTypeToString(vv.Type())));
}

short FakeluaToNativeShort(State *s, CVar v) {
    const auto vv = static_cast<Var &>(v);
    if (vv.Type() == VarType::Int) {
        return static_cast<short>(vv.GetInt());
    }
    ThrowFakeluaException(std::format("FakeluaToNativeShort failed, type is {}", VarTypeToString(vv.Type())));
}

unsigned short FakeluaToNativeUshort(State *s, CVar v) {
    const auto vv = static_cast<Var &>(v);
    if (vv.Type() == VarType::Int) {
        return static_cast<unsigned short>(vv.GetInt());
    }
    ThrowFakeluaException(std::format("FakeluaToNativeUshort failed, type is {}", VarTypeToString(vv.Type())));
}

int FakeluaToNativeInt(State *s, CVar v) {
    const auto vv = static_cast<Var &>(v);
    if (vv.Type() == VarType::Int) {
        return static_cast<int>(vv.GetInt());
    }
    ThrowFakeluaException(std::format("FakeluaToNativeInt failed, type is {}", VarTypeToString(vv.Type())));
}

unsigned int FakeluaToNativeUint(State *s, CVar v) {
    const auto vv = static_cast<Var &>(v);
    if (vv.Type() == VarType::Int) {
        return static_cast<unsigned int>(vv.GetInt());
    }
    ThrowFakeluaException(std::format("FakeluaToNativeUint failed, type is {}", VarTypeToString(vv.Type())));
}

long FakeluaToNativeLong(State *s, CVar v) {
    const auto vv = static_cast<Var &>(v);
    if (vv.Type() == VarType::Int) {
        return vv.GetInt();
    }
    ThrowFakeluaException(std::format("FakeluaToNativeLong failed, type is {}", VarTypeToString(vv.Type())));
}

unsigned long FakeluaToNativeUlong(State *s, CVar v) {
    const auto vv = static_cast<Var &>(v);
    if (vv.Type() == VarType::Int) {
        return vv.GetInt();
    }
    ThrowFakeluaException(std::format("FakeluaToNativeUlong failed, type is {}", VarTypeToString(vv.Type())));
}

long long FakeluaToNativeLonglong(State *s, CVar v) {
    const auto vv = static_cast<Var &>(v);
    if (vv.Type() == VarType::Int) {
        return vv.GetInt();
    }
    ThrowFakeluaException(std::format("FakeluaToNativeLonglong failed, type is {}", VarTypeToString(vv.Type())));
}

unsigned long long FakeluaToNativeUlonglong(State *s, CVar v) {
    const auto vv = static_cast<Var &>(v);
    if (vv.Type() == VarType::Int) {
        return static_cast<unsigned long long>(vv.GetInt());
    }
    ThrowFakeluaException(std::format("FakeluaToNativeUlonglong failed, type is {}", VarTypeToString(vv.Type())));
}

float FakeluaToNativeFloat(State *s, CVar v) {
    const auto vv = static_cast<Var &>(v);
    if (vv.Type() == VarType::Float) {
        return static_cast<float>(vv.GetFloat());
    }
    if (vv.Type() == VarType::Int) {
        return static_cast<float>(vv.GetInt());
    }
    ThrowFakeluaException(std::format("FakeluaToNativeFloat failed, type is {}", VarTypeToString(vv.Type())));
}

double FakeluaToNativeDouble(State *s, CVar v) {
    const auto vv = static_cast<Var &>(v);
    if (vv.Type() == VarType::Float) {
        return vv.GetFloat();
    }
    if (vv.Type() == VarType::Int) {
        return static_cast<double>(vv.GetInt());
    }
    ThrowFakeluaException(std::format("FakeluaToNativeDouble failed, type is {}", VarTypeToString(vv.Type())));
}

const char *FakeluaToNativeCstr(State *s, CVar v) {
    const auto vv = static_cast<Var &>(v);
    if (vv.Type() == VarType::String) {
        return vv.GetString()->Str().data();
    }
    ThrowFakeluaException(std::format("FakeluaToNativeCstr failed, type is {}", VarTypeToString(vv.Type())));
}

const char *FakeluaToNativeStr(State *s, CVar v) {
    const auto vv = static_cast<Var &>(v);
    if (vv.Type() == VarType::String) {
        return vv.GetString()->Str().data();
    }
    ThrowFakeluaException(std::format("FakeluaToNativeStr failed, type is {}", VarTypeToString(vv.Type())));
}

std::string FakeluaToNativeString(State *s, CVar v) {
    const auto vv = static_cast<Var &>(v);
    if (vv.Type() == VarType::String) {
        return std::string(vv.GetString()->Str());
    }
    ThrowFakeluaException(std::format("FakeluaToNativeString failed, type is {}", VarTypeToString(vv.Type())));
}

std::string_view FakeluaToNativeStringView(State *s, CVar v) {
    const auto vv = static_cast<Var &>(v);
    if (vv.Type() == VarType::String) {
        return vv.GetString()->Str();
    }
    ThrowFakeluaException(std::format("FakeluaToNativeStringview failed, type is {}", VarTypeToString(vv.Type())));
}

void VarToVi(State *s, CVar src, VarInterface *dst) {
    const auto vv = static_cast<Var &>(src);
    DEBUG_ASSERT(vv.Type() >= VarType::Min && vv.Type() <= VarType::Max);
    switch (vv.Type()) {
        case VarType::Nil:
            dst->ViSetNil();
            break;
        case VarType::Bool:
            dst->ViSetBool(vv.GetBool());
            break;
        case VarType::Int:
            dst->ViSetInt(vv.GetInt());
            break;
        case VarType::Float:
            dst->ViSetFloat(vv.GetFloat());
            break;
        case VarType::String:
            dst->ViSetString(vv.GetString()->Str());
            break;
        case VarType::Table: {
            std::vector<std::pair<VarInterface *, VarInterface *>> kvs;
            for (size_t i = 0; i < vv.GetTable()->Size(); ++i) {
                const auto k = vv.GetTable()->KeyAt(i);
                const auto v = vv.GetTable()->ValueAt(i);
                auto ki = GetVarInterfaceNewFunc(s)();
                auto vi = GetVarInterfaceNewFunc(s)();
                VarToVi(s, k, ki);
                VarToVi(s, v, vi);
                kvs.emplace_back(ki, vi);
            }
            dst->ViSetTable(kvs);
            break;
        }
    }
}

VarInterface *FakeluaToNativeObj(State *s, CVar v) {
    const auto ret = GetVarInterfaceNewFunc(s)();
    VarToVi(s, v, ret);
    return ret;
}

void *GetFuncAddr(State *s, const std::string_view &name, int &arg_count) {
    // TODO
}

[[noreturn]] void ThrowInterFakeluaException(const std::string &msg) {
    ThrowFakeluaException(msg);
}

int GetReentrantCount(State *s) {
    return s->GetReentrantCount();
}

void AddReentrantCount(State *s) {
    s->AddReentrantCount();
}

void SubReentrantCount(State *s) {
    s->SubReentrantCount();
}

void Reset(State *s) {
    s->Reset();
}

}// namespace inter

State *FakeluaNewState(const StateConfig &cfg) {
    return new State(cfg);
}

void FakeluaFreeState(State *s) {
    delete s;
}

void CompileFile(State *s, const std::string &filename, const CompileConfig &cfg) {
    s->CompileFile(filename, cfg);
}

void CompileString(State *s, const std::string &str, const CompileConfig &cfg) {
    s->CompileString(str, cfg);
}

void SetVarInterfaceNewFunc(State *s, const std::function<VarInterface *()> &func) {
    s->SetVarInterfaceNewFunc(func);
}

std::function<VarInterface *()> &GetVarInterfaceNewFunc(State *s) {
    return s->GetVarInterfaceNewFunc();
}

void SetDebugLogLevel(int level) {
    SetLogLevel(static_cast<LogLevel>(level));
}

}// namespace fakelua
