#include "fakelua.h"
#include "state/State.h"
#include "util/common.h"
#include "var/var_table.h"

namespace fakelua {

namespace inter {

CVar NativeToFakeluaNil(const FakeluaStatePtr &s) {
    return Var{};
}

CVar NativeToFakeluaBool(const FakeluaStatePtr &s, bool v) {
    Var ret;
    ret.SetBool(v);
    return ret;
}

CVar NativeToFakeluaChar(const FakeluaStatePtr &s, char v) {
    Var ret;
    ret.SetInt(v);
    return ret;
}

CVar NativeToFakeluaUchar(const FakeluaStatePtr &s, unsigned char v) {
    Var ret;
    ret.SetInt(v);
    return ret;
}

CVar NativeToFakeluaShort(const FakeluaStatePtr &s, short v) {
    Var ret;
    ret.SetInt(v);
    return ret;
}

CVar NativeToFakeluaUshort(const FakeluaStatePtr &s, unsigned short v) {
    Var ret;
    ret.SetInt(v);
    return ret;
}

CVar NativeToFakeluaInt(const FakeluaStatePtr &s, int v) {
    Var ret;
    ret.SetInt(v);
    return ret;
}

CVar NativeToFakeluaUint(const FakeluaStatePtr &s, unsigned int v) {
    Var ret;
    ret.SetInt(v);
    return ret;
}

CVar NativeToFakeluaLong(const FakeluaStatePtr &s, long v) {
    Var ret;
    ret.SetInt(v);
    return ret;
}

CVar NativeToFakeluaUlong(const FakeluaStatePtr &s, unsigned long v) {
    Var ret;
    ret.SetInt(v);
    return ret;
}

CVar NativeToFakeluaLonglong(const FakeluaStatePtr &s, long long v) {
    Var ret;
    ret.SetInt(v);
    return ret;
}

CVar NativeToFakeluaUlonglong(const FakeluaStatePtr &s, unsigned long long v) {
    Var ret;
    ret.SetInt(static_cast<int64_t>(v));
    return ret;
}

CVar NativeToFakeluaFloat(const FakeluaStatePtr &s, float v) {
    Var ret;
    ret.SetFloat(v);
    return ret;
}

CVar NativeToFakeluaDouble(const FakeluaStatePtr &s, double v) {
    Var ret;
    ret.SetFloat(v);
    return ret;
}

CVar NativeToFakeluaCstr(const FakeluaStatePtr &s, const char *v) {
    Var ret;
    ret.SetString(s, v);
    return ret;
}

CVar NativeToFakeluaStr(const FakeluaStatePtr &s, char *v) {
    Var ret;
    ret.SetString(s, v);
    return ret;
}

CVar NativeToFakeluaString(const FakeluaStatePtr &s, const std::string &v) {
    Var ret;
    ret.SetString(s, v);
    return ret;
}

CVar NativeToFakeluaStringview(const FakeluaStatePtr &s, const std::string_view &v) {
    Var ret;
    ret.SetString(s, v);
    return ret;
}

Var ViToVar(const FakeluaStatePtr &s, const VarInterface *src) {
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

CVar NativeToFakeluaObj(const FakeluaStatePtr &s, const VarInterface *v) {
    return ViToVar(s, v);
}

bool FakeluaToNativeBool(const FakeluaStatePtr &s, CVar v) {
    const auto vv = static_cast<Var &>(v);
    if (vv.Type() == VarType::Bool) {
        return vv.GetBool();
    }
    ThrowFakeluaException(std::format("FakeluaToNativeBool failed, type is {}", VarTypeToString(vv.Type())));
}

char FakeluaToNativeChar(const FakeluaStatePtr &s, CVar v) {
    const auto vv = static_cast<Var &>(v);
    if (vv.Type() == VarType::Int) {
        return static_cast<char>(vv.GetInt());
    }
    ThrowFakeluaException(std::format("FakeluaToNativeChar failed, type is {}", VarTypeToString(vv.Type())));
}

unsigned char FakeluaToNativeUchar(const FakeluaStatePtr &s, CVar v) {
    const auto vv = static_cast<Var &>(v);
    if (vv.Type() == VarType::Int) {
        return static_cast<unsigned char>(vv.GetInt());
    }
    ThrowFakeluaException(std::format("FakeluaToNativeUchar failed, type is {}", VarTypeToString(vv.Type())));
}

short FakeluaToNativeShort(const FakeluaStatePtr &s, CVar v) {
    const auto vv = static_cast<Var &>(v);
    if (vv.Type() == VarType::Int) {
        return static_cast<short>(vv.GetInt());
    }
    ThrowFakeluaException(std::format("FakeluaToNativeShort failed, type is {}", VarTypeToString(vv.Type())));
}

unsigned short FakeluaToNativeUshort(const FakeluaStatePtr &s, CVar v) {
    const auto vv = static_cast<Var &>(v);
    if (vv.Type() == VarType::Int) {
        return static_cast<unsigned short>(vv.GetInt());
    }
    ThrowFakeluaException(std::format("FakeluaToNativeUshort failed, type is {}", VarTypeToString(vv.Type())));
}

int FakeluaToNativeInt(const FakeluaStatePtr &s, CVar v) {
    const auto vv = static_cast<Var &>(v);
    if (vv.Type() == VarType::Int) {
        return static_cast<int>(vv.GetInt());
    }
    ThrowFakeluaException(std::format("FakeluaToNativeInt failed, type is {}", VarTypeToString(vv.Type())));
}

unsigned int FakeluaToNativeUint(const FakeluaStatePtr &s, CVar v) {
    const auto vv = static_cast<Var &>(v);
    if (vv.Type() == VarType::Int) {
        return static_cast<unsigned int>(vv.GetInt());
    }
    ThrowFakeluaException(std::format("FakeluaToNativeUint failed, type is {}", VarTypeToString(vv.Type())));
}

long FakeluaToNativeLong(const FakeluaStatePtr &s, CVar v) {
    const auto vv = static_cast<Var &>(v);
    if (vv.Type() == VarType::Int) {
        return vv.GetInt();
    }
    ThrowFakeluaException(std::format("FakeluaToNativeLong failed, type is {}", VarTypeToString(vv.Type())));
}

unsigned long FakeluaToNativeUlong(const FakeluaStatePtr &s, CVar v) {
    const auto vv = static_cast<Var &>(v);
    if (vv.Type() == VarType::Int) {
        return vv.GetInt();
    }
    ThrowFakeluaException(std::format("FakeluaToNativeUlong failed, type is {}", VarTypeToString(vv.Type())));
}

long long FakeluaToNativeLonglong(const FakeluaStatePtr &s, CVar v) {
    const auto vv = static_cast<Var &>(v);
    if (vv.Type() == VarType::Int) {
        return vv.GetInt();
    }
    ThrowFakeluaException(std::format("FakeluaToNativeLonglong failed, type is {}", VarTypeToString(vv.Type())));
}

unsigned long long FakeluaToNativeUlonglong(const FakeluaStatePtr &s, CVar v) {
    const auto vv = static_cast<Var &>(v);
    if (vv.Type() == VarType::Int) {
        return static_cast<unsigned long long>(vv.GetInt());
    }
    ThrowFakeluaException(std::format("FakeluaToNativeUlonglong failed, type is {}", VarTypeToString(vv.Type())));
}

float FakeluaToNativeFloat(const FakeluaStatePtr &s, CVar v) {
    const auto vv = static_cast<Var &>(v);
    if (vv.Type() == VarType::Float) {
        return static_cast<float>(vv.GetFloat());
    }
    if (vv.Type() == VarType::Int) {
        return static_cast<float>(vv.GetInt());
    }
    ThrowFakeluaException(std::format("FakeluaToNativeFloat failed, type is {}", VarTypeToString(vv.Type())));
}

double FakeluaToNativeDouble(const FakeluaStatePtr &s, CVar v) {
    const auto vv = static_cast<Var &>(v);
    if (vv.Type() == VarType::Float) {
        return vv.GetFloat();
    }
    if (vv.Type() == VarType::Int) {
        return static_cast<double>(vv.GetInt());
    }
    ThrowFakeluaException(std::format("FakeluaToNativeDouble failed, type is {}", VarTypeToString(vv.Type())));
}

const char *FakeluaToNativeCstr(const FakeluaStatePtr &s, CVar v) {
    const auto vv = static_cast<Var &>(v);
    if (vv.Type() == VarType::String) {
        return vv.GetString()->Str().data();
    }
    ThrowFakeluaException(std::format("FakeluaToNativeCstr failed, type is {}", VarTypeToString(vv.Type())));
}

const char *FakeluaToNativeStr(const FakeluaStatePtr &s, CVar v) {
    const auto vv = static_cast<Var &>(v);
    if (vv.Type() == VarType::String) {
        return vv.GetString()->Str().data();
    }
    ThrowFakeluaException(std::format("FakeluaToNativeStr failed, type is {}", VarTypeToString(vv.Type())));
}

std::string FakeluaToNativeString(const FakeluaStatePtr &s, CVar v) {
    const auto vv = static_cast<Var &>(v);
    if (vv.Type() == VarType::String) {
        return std::string(vv.GetString()->Str());
    }
    ThrowFakeluaException(std::format("FakeluaToNativeString failed, type is {}", VarTypeToString(vv.Type())));
}

std::string_view FakeluaToNativeStringview(const FakeluaStatePtr &s, CVar v) {
    const auto vv = static_cast<Var &>(v);
    if (vv.Type() == VarType::String) {
        return vv.GetString()->Str();
    }
    ThrowFakeluaException(std::format("FakeluaToNativeStringview failed, type is {}", VarTypeToString(vv.Type())));
}

void VarToVi(const FakeluaStatePtr &s, CVar src, VarInterface *dst) {
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
                auto ki = std::dynamic_pointer_cast<State>(s)->GetVarInterfaceNewFunc()();
                auto vi = std::dynamic_pointer_cast<State>(s)->GetVarInterfaceNewFunc()();
                VarToVi(s, k, ki);
                VarToVi(s, v, vi);
                kvs.emplace_back(ki, vi);
            }
            dst->ViSetTable(kvs);
            break;
        }
    }
}

VarInterface *FakeluaToNativeObj(const FakeluaStatePtr &s, CVar v) {
    const auto ret = std::dynamic_pointer_cast<State>(s)->GetVarInterfaceNewFunc()();
    VarToVi(s, v, ret);
    return ret;
}

class ReentryCounter {
public:
    explicit ReentryCounter(int &counter) : counter_(counter) {
        ++counter_;
    }

    ~ReentryCounter() {
        --counter_;
    }

private:
    int &counter_;
};

void call(const FakeluaStatePtr &s, const std::string &name, CVar *args, size_t arg_size, CVar *rets, size_t ret_size) {
    const auto st = std::dynamic_pointer_cast<State>(s);
    const auto func = st->get_vm().GetFunction(name);
    if (!func) {
        ThrowFakeluaException(std::format("function {} not found", name));
    }
    const auto addr = func->get_addr();

    auto &reentrant_count = st->get_reentrant_count();
    if (!reentrant_count) {
        st->reset();
    }
    ReentryCounter rc(reentrant_count);

    // every time before call function, we save the stack top position, after call we can pop to this position
    auto &stack = st->get_stack();
    auto stack_start = stack.top();
    auto stack_max = stack.max();

    // push args
    for (size_t i = 0; i < arg_size; ++i) {
        stack.push(args[i]);
    }

    auto stack_rets = stack.top();
    auto stack_cur = stack.top();
    // call
    auto return_count = reinterpret_cast<size_t (*)(CVar *, size_t, CVar *, CVar *, CVar *)>(addr)(stack_start,// the args start here
                                                                                                   arg_size,   // number of args
                                                                                                   stack_rets, // rets start here
                                                                                                   stack_cur,  // current stack position
                                                                                                   stack_max   // max stack position
    );

    // get rets
    for (size_t i = 0; i < ret_size && i < return_count; ++i) {
        rets[i] = stack_rets[i];
    }
    // set nil for extra rets
    for (size_t i = return_count; i < ret_size; ++i) {
        rets[i] = CVar{};
    }

    // pop stack to the start position
    stack.PopTo(stack_start);
}

}// namespace inter

FakeluaStatePtr FakeluaNewstate() {
    LOG_INFO("FakeluaNewstate");
    return std::make_shared<State>();
}

void FakeluaState::SetDebugLogLevel(int level) {
    SetLogLevel(static_cast<LogLevel>(level));
}

}// namespace fakelua
