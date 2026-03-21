#pragma once

#include <format>
#include <functional>
#include <memory>
#include <ranges>
#include <string>

namespace fakelua {

// use to describe the complex type of lua.
// communication between fakelua and native.
// so it can transfer complex type like table.
struct VarInterface {
    enum class Type {
        MIN,
        NIL = MIN,
        BOOL,
        INT,
        FLOAT,
        STRING,
        TABLE,
        MAX = TABLE,
    };

    virtual ~VarInterface() = default;

    [[nodiscard]] virtual Type ViGetType() const = 0;

    virtual void ViSetNil() = 0;

    virtual void ViSetBool(bool v) = 0;

    virtual void ViSetInt(int64_t v) = 0;

    virtual void ViSetFloat(double v) = 0;

    virtual void ViSetString(const std::string_view &v) = 0;

    virtual void ViSetTable(const std::vector<std::pair<VarInterface *, VarInterface *>> &kv) = 0;

    [[nodiscard]] virtual bool ViGetBool() const = 0;

    [[nodiscard]] virtual int64_t ViGetInt() const = 0;

    [[nodiscard]] virtual double ViGetFloat() const = 0;

    [[nodiscard]] virtual std::string_view ViGetString() const = 0;

    [[nodiscard]] virtual size_t ViGetTableSize() const = 0;

    [[nodiscard]] virtual std::pair<VarInterface *, VarInterface *> ViGetTableKv(int index) const = 0;

    [[nodiscard]] virtual std::string ViToString(int tab) const = 0;
};

// simple var implement, just for simple use.
struct SimpleVarImpl final : public VarInterface {
    SimpleVarImpl() = default;

    ~SimpleVarImpl() override = default;

    [[nodiscard]] Type ViGetType() const override {
        return type_;
    }

    void ViSetNil() override {
        type_ = Type::NIL;
    }

    void ViSetBool(bool v) override {
        type_ = Type::BOOL;
        bool_ = v;
    }

    void ViSetInt(int64_t v) override {
        type_ = Type::INT;
        int_ = v;
    }

    void ViSetFloat(double v) override {
        type_ = Type::FLOAT;
        float_ = v;
    }

    void ViSetString(const std::string_view &v) override {
        type_ = Type::STRING;
        string_ = v;
    }

    void ViSetTable(const std::vector<std::pair<VarInterface *, VarInterface *>> &kv) override {
        type_ = Type::TABLE;
        table_ = kv;
    }

    [[nodiscard]] bool ViGetBool() const override {
        return bool_;
    }

    [[nodiscard]] int64_t ViGetInt() const override {
        return int_;
    }

    [[nodiscard]] double ViGetFloat() const override {
        return float_;
    }

    [[nodiscard]] std::string_view ViGetString() const override {
        return string_;
    }

    [[nodiscard]] size_t ViGetTableSize() const override {
        return table_.size();
    }

    [[nodiscard]] std::pair<VarInterface *, VarInterface *> ViGetTableKv(int index) const override {
        return table_[index];
    }

    [[nodiscard]] std::string ViToString(int tab) const override {
        std::string ret;
        switch (type_) {
            case Type::NIL:
                ret = "nil";
                break;
            case Type::BOOL:
                ret = bool_ ? "true" : "false";
                break;
            case Type::INT:
                ret = std::to_string(int_);
                break;
            case Type::FLOAT:
                ret = std::to_string(float_);
                break;
            case Type::STRING:
                ret = std::format("\"{}\"", string_);
                break;
            case Type::TABLE:
                ret = "table:";
                for (auto &kv: table_) {
                    ret += std::format("\n{}[{}] = {}", std::string(tab + 1, '\t'), kv.first->ViToString(tab + 1),
                                       kv.second->ViToString(tab + 1));
                }
                break;
        }

        return ret;
    }

    // sort table by key, just for debug
    void ViSortTable() {
        std::sort(table_.begin(), table_.end(), [](const auto &a, const auto &b) {
            if (a.first->ViGetType() != b.first->ViGetType()) {
                return a.first->ViGetType() < b.first->ViGetType();
            }
            switch (a.first->ViGetType()) {
                case Type::NIL:
                    return false;
                case Type::BOOL:
                    return a.first->ViGetBool() < b.first->ViGetBool();
                case Type::INT:
                    return a.first->ViGetInt() < b.first->ViGetInt();
                case Type::FLOAT:
                    return a.first->ViGetFloat() < b.first->ViGetFloat();
                case Type::STRING:
                    return a.first->ViGetString() < b.first->ViGetString();
                case Type::TABLE:
                    return a.first->ViGetTableSize() < b.first->ViGetTableSize();
                default:
                    return false;
            }
        });
        for (const auto &val: table_ | std::views::values) {
            if (val->ViGetType() == Type::TABLE) {
                dynamic_cast<SimpleVarImpl *>(val)->ViSortTable();
            }
        }
    }

    Type type_ = Type::NIL;
    bool bool_ = false;
    int64_t int_ = 0;
    double float_ = 0;
    std::string_view string_;
    std::vector<std::pair<VarInterface *, VarInterface *>> table_;
};

class VarTable;
class VarString;
class Var;

struct CVar {
protected:
    int type_ = 0;
    union cvar_data {
        bool b;
        int64_t i;
        double f;
        VarString *s;
        VarTable *t;
    };
    cvar_data data_{};
};

// control the Compiler behavior
struct CompileConfig {
    // skip jit compile. just lex and parse.
    bool skip_jit = false;
    // debug mode. if true, the jit code will be dumped to file.
    bool debug_mode = true;
};

struct StateConfig {
    // max var stack count
    size_t max_stack_size = 65536;
};

// fake_lua state interface, every state can only run in one thread, just like Lua.
// every state has its own running environment. there could be many states in one process.
class FakeluaState : public std::enable_shared_from_this<FakeluaState> {
public:
    FakeluaState(StateConfig config = {}) : config_(config) {
    }

    virtual ~FakeluaState() = default;

    // compile file, the file is a lua file.
    virtual void CompileFile(const std::string &filename, const CompileConfig &cfg) = 0;

    // compile string, the string is the content of a file.
    virtual void CompileString(const std::string &str, const CompileConfig &cfg) = 0;

    // call function by name
    template<typename... Rets, typename... Args>
    void call(const std::string &name, std::tuple<Rets &...> &&rets, Args &&...args);

    // set VarInterface new instance function
    void SetVarInterfaceNewFunc(const std::function<VarInterface *()> &func) {
        var_interface_new_func_ = func;
    }

    // get VarInterface new instance function
    std::function<VarInterface *()> &GetVarInterfaceNewFunc() {
        return var_interface_new_func_;
    }

    // set global debug log level, note all state will be set.
    // 0: off, 1: error, 2: info, default is error.
    void SetDebugLogLevel(int level);

private:
    std::function<VarInterface *()> var_interface_new_func_;
    StateConfig config_;
};

using FakeluaStatePtr = std::shared_ptr<FakeluaState>;

namespace inter {

// native to fakelua
CVar NativeToFakeluaNil(const FakeluaStatePtr &s);
CVar NativeToFakeluaBool(const FakeluaStatePtr &s, bool v);
CVar NativeToFakeluaChar(const FakeluaStatePtr &s, char v);
CVar NativeToFakeluaUchar(const FakeluaStatePtr &s, unsigned char v);
CVar NativeToFakeluaShort(const FakeluaStatePtr &s, short v);
CVar NativeToFakeluaUshort(const FakeluaStatePtr &s, unsigned short v);
CVar NativeToFakeluaInt(const FakeluaStatePtr &s, int v);
CVar NativeToFakeluaUint(const FakeluaStatePtr &s, unsigned int v);
CVar NativeToFakeluaLong(const FakeluaStatePtr &s, long v);
CVar NativeToFakeluaUlong(const FakeluaStatePtr &s, unsigned long v);
CVar NativeToFakeluaLonglong(const FakeluaStatePtr &s, long long v);
CVar NativeToFakeluaUlonglong(const FakeluaStatePtr &s, unsigned long long v);
CVar NativeToFakeluaFloat(const FakeluaStatePtr &s, float v);
CVar NativeToFakeluaDouble(const FakeluaStatePtr &s, double v);
CVar NativeToFakeluaCstr(const FakeluaStatePtr &s, const char *v);
CVar NativeToFakeluaStr(const FakeluaStatePtr &s, char *v);
CVar NativeToFakeluaString(const FakeluaStatePtr &s, const std::string &v);
CVar NativeToFakeluaStringview(const FakeluaStatePtr &s, const std::string_view &v);
CVar NativeToFakeluaObj(const FakeluaStatePtr &s, const VarInterface *v);

template<typename T>
CVar NativeToFakelua(const FakeluaStatePtr &s, T v) {
    // check if T is nil
    if constexpr (std::is_same_v<T, std::nullptr_t>) {
        return NativeToFakeluaNil(s);
    }
    // check if T is bool
    else if constexpr (std::is_same_v<T, bool>) {
        return NativeToFakeluaBool(s, v);
    }
    // check if T is char
    else if constexpr (std::is_same_v<T, char>) {
        return NativeToFakeluaChar(s, v);
    }
    // check if T is unsigned char
    else if constexpr (std::is_same_v<T, unsigned char>) {
        return NativeToFakeluaUchar(s, v);
    }
    // check if T is short
    else if constexpr (std::is_same_v<T, short>) {
        return NativeToFakeluaShort(s, v);
    }
    // check if T is unsigned short
    else if constexpr (std::is_same_v<T, unsigned short>) {
        return NativeToFakeluaUshort(s, v);
    }
    // check if T is int
    else if constexpr (std::is_same_v<T, int>) {
        return NativeToFakeluaInt(s, v);
    }
    // check if T is unsigned int
    else if constexpr (std::is_same_v<T, unsigned int>) {
        return NativeToFakeluaUint(s, v);
    }
    // check if T is long
    else if constexpr (std::is_same_v<T, long>) {
        return NativeToFakeluaLong(s, v);
    }
    // check if T is unsigned long
    else if constexpr (std::is_same_v<T, unsigned long>) {
        return NativeToFakeluaUlong(s, v);
    }
    // check if T is long long
    else if constexpr (std::is_same_v<T, long long>) {
        return NativeToFakeluaLonglong(s, v);
    }
    // check if T is unsigned long long
    else if constexpr (std::is_same_v<T, unsigned long long>) {
        return NativeToFakeluaUlonglong(s, v);
    }
    // check if T is float
    else if constexpr (std::is_same_v<T, float>) {
        return NativeToFakeluaFloat(s, v);
    }
    // check if T is double
    else if constexpr (std::is_same_v<T, double>) {
        return NativeToFakeluaDouble(s, v);
    }
    // check if T is const char *
    else if constexpr (std::is_same_v<T, const char *>) {
        return NativeToFakeluaCstr(s, v);
    }
    // check if T is char *
    else if constexpr (std::is_same_v<T, char *>) {
        return NativeToFakeluaStr(s, v);
    }
    // check if T is std::string
    else if constexpr (std::is_same_v<T, std::string>) {
        return NativeToFakeluaString(s, v);
    }
    // check if T is std::string_view
    else if constexpr (std::is_same_v<T, std::string_view>) {
        return NativeToFakeluaStringview(s, v);
    }
    // check if T is a cvar
    else if constexpr (std::is_same_v<T, CVar>) {
        return v;
    } else {
        // static_assert T should be VarInterface* or implement VarInterface
        static_assert(std::is_pointer_v<T>, "T should be pointer");
        static_assert(std::is_base_of_v<VarInterface, std::remove_pointer_t<T>>, "T should be VarInterface");
        return NativeToFakeluaObj(s, v);
    }
}

// fakelua to native
bool FakeluaToNativeBool(const FakeluaStatePtr &s, CVar v);
char FakeluaToNativeChar(const FakeluaStatePtr &s, CVar v);
unsigned char FakeluaToNativeUchar(const FakeluaStatePtr &s, CVar v);
short FakeluaToNativeShort(const FakeluaStatePtr &s, CVar v);
unsigned short FakeluaToNativeUshort(const FakeluaStatePtr &s, CVar v);
int FakeluaToNativeInt(const FakeluaStatePtr &s, CVar v);
unsigned int FakeluaToNativeUint(const FakeluaStatePtr &s, CVar v);
long FakeluaToNativeLong(const FakeluaStatePtr &s, CVar v);
unsigned long FakeluaToNativeUlong(const FakeluaStatePtr &s, CVar v);
long long FakeluaToNativeLonglong(const FakeluaStatePtr &s, CVar v);
unsigned long long FakeluaToNativeUlonglong(const FakeluaStatePtr &s, CVar v);
float FakeluaToNativeFloat(const FakeluaStatePtr &s, CVar v);
double FakeluaToNativeDouble(const FakeluaStatePtr &s, CVar v);
const char *FakeluaToNativeCstr(const FakeluaStatePtr &s, CVar v);
const char *FakeluaToNativeStr(const FakeluaStatePtr &s, CVar v);
std::string FakeluaToNativeString(const FakeluaStatePtr &s, CVar v);
std::string_view FakeluaToNativeStringview(const FakeluaStatePtr &s, CVar v);
VarInterface *FakeluaToNativeObj(const FakeluaStatePtr &s, CVar v);

template<typename T>
T FakeluaToNative(const FakeluaStatePtr &s, const CVar v) {
    if constexpr (std::is_same_v<T, bool>) {
        return FakeluaToNativeBool(s, v);
    } else if constexpr (std::is_same_v<T, char>) {
        return FakeluaToNativeChar(s, v);
    } else if constexpr (std::is_same_v<T, unsigned char>) {
        return FakeluaToNativeUchar(s, v);
    } else if constexpr (std::is_same_v<T, short>) {
        return FakeluaToNativeShort(s, v);
    } else if constexpr (std::is_same_v<T, unsigned short>) {
        return FakeluaToNativeUshort(s, v);
    } else if constexpr (std::is_same_v<T, int>) {
        return FakeluaToNativeInt(s, v);
    } else if constexpr (std::is_same_v<T, unsigned int>) {
        return FakeluaToNativeUint(s, v);
    } else if constexpr (std::is_same_v<T, long>) {
        return FakeluaToNativeLong(s, v);
    } else if constexpr (std::is_same_v<T, unsigned long>) {
        return FakeluaToNativeUlong(s, v);
    } else if constexpr (std::is_same_v<T, long long>) {
        return FakeluaToNativeLonglong(s, v);
    } else if constexpr (std::is_same_v<T, unsigned long long>) {
        return FakeluaToNativeUlonglong(s, v);
    } else if constexpr (std::is_same_v<T, float>) {
        return FakeluaToNativeFloat(s, v);
    } else if constexpr (std::is_same_v<T, double>) {
        return FakeluaToNativeDouble(s, v);
    } else if constexpr (std::is_same_v<T, const char *>) {
        return FakeluaToNativeCstr(s, v);
    } else if constexpr (std::is_same_v<T, char *>) {
        return (char *) FakeluaToNativeStr(s, v);
    } else if constexpr (std::is_same_v<T, std::string>) {
        return FakeluaToNativeString(s, v);
    } else if constexpr (std::is_same_v<T, std::string_view>) {
        return FakeluaToNativeStringview(s, v);
    } else if constexpr (std::is_same_v<T, CVar>) {
        return v;
    } else {
        // static_assert T should be VarInterface* or implement VarInterface
        static_assert(std::is_pointer_v<T>, "T should be pointer");
        static_assert(std::is_base_of_v<VarInterface, std::remove_pointer_t<T>>, "T should be VarInterface");
        return FakeluaToNativeObj(s, v);
    }
}

template<size_t I = 0, typename... Rets>
inline std::enable_if_t<I == sizeof...(Rets), void> FakeluaFuncRetHelper(const FakeluaStatePtr &s, CVar *ret, std::tuple<Rets &...> &rets) {
}

template<size_t I = 0, typename... Rets>
        inline std::enable_if_t <
        I<sizeof...(Rets), void> FakeluaFuncRetHelper(const FakeluaStatePtr &s, CVar *ret, std::tuple<Rets &...> &rets) {
    typedef std::remove_reference_t<std::tuple_element_t<I, std::tuple<Rets &...>>> t;
    std::get<I>(rets) = FakeluaToNative<t>(s, ret[I]);
    FakeluaFuncRetHelper<I + 1, Rets...>(s, ret, rets);
}

void call(const FakeluaStatePtr &s, const std::string &name, CVar *args, size_t arg_size, CVar *rets, size_t ret_size);

}// namespace inter

// call funtion by name
template<typename... Rets, typename... Args>
void FakeluaState::call(const std::string &name, std::tuple<Rets &...> &&rets, Args &&...args) {
    // transfer args to vars array
    CVar args_array[sizeof...(Args)] = {inter::NativeToFakelua(shared_from_this(), std::forward<Args>(args))...};

    // alloc vars in stack
    CVar rets_array[sizeof...(Rets)] = {};

    // call function
    inter::call(shared_from_this(), name, args_array, sizeof...(Args), rets_array, sizeof...(Rets));

    // transfer vars to rets
    inter::FakeluaFuncRetHelper(shared_from_this(), rets_array, rets);
}

// create fake_lua state
FakeluaStatePtr FakeluaNewstate();

}// namespace fakelua
