#pragma once

#include <format>
#include <functional>
#include <memory>
#include <ranges>
#include <string>

namespace fakelua {

// 用于FakeLua与原生代码之间的通信，传输像table这样的复杂类型。
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

// 简单的实现，仅用于测试和调试。实际使用中，用户可以根据需要实现自己的 VarInterface。
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
                for (const auto &[fst, snd]: table_) {
                    ret += std::format("\n{}[{}] = {}", std::string(tab + 1, '\t'), fst->ViToString(tab + 1), snd->ViToString(tab + 1));
                }
                break;
        }

        return ret;
    }

    // 按键对表进行排序，仅用于调试
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

// 基本变量类型，包含类型标识和数据。数据使用 union 存储，根据类型标识来访问。
struct CVar {
protected:
    int type_ = 0;
    int flag_ = 0;
    union cvar_data {
        bool b;
        int64_t i;
        double f;
        VarString *s;
        VarTable *t;
    };
    cvar_data data_{};
};

// 控制编译器的配置项
struct CompileConfig {
    // 跳过 JIT 编译。仅进行词法分析和语法解析。
    bool skip_jit = false;
    // 调试模式。如果为 true，JIT 代码将被转储到文件中。
    bool debug_mode = true;
};

struct StateConfig {
    // 最大栈深，超过该深度将抛出异常。默认为 65536。
    size_t max_stack_size = 65536;
};

class State;

// 创建 FakeLua 状态
State *FakeluaNewState(const StateConfig &cfg = {});

// 释放 FakeLua 状态
void FakeluaFreeState(State *s);

// 编译文件
void CompileFile(State *s, const std::string &filename, const CompileConfig &cfg);

// 编译字符串
void CompileString(State *s, const std::string &str, const CompileConfig &cfg);

// 调用某个Lua里的函数
template<typename... Rets, typename... Args>
void Call(State *s, const std::string &name, std::tuple<Rets &...> &&rets, Args &&...args);

// 设置 VarInterface 构造实例函数
void SetVarInterfaceNewFunc(State *s, const std::function<VarInterface *()> &func);

// 获取 VarInterface 构造实例函数
std::function<VarInterface *()> &GetVarInterfaceNewFunc(State *s);

// 设置全局调试日志级别，注意所有状态都将被设置。
// 0: 关闭, 1: 错误, 2: 信息, 默认为错误。
void SetDebugLogLevel(State *s, int level);

namespace inter {

// 原生转 FakeLua
CVar NativeToFakeluaNil(State *s);
CVar NativeToFakeluaBool(State *s, bool v);
CVar NativeToFakeluaChar(State *s, char v);
CVar NativeToFakeluaUchar(State *s, unsigned char v);
CVar NativeToFakeluaShort(State *s, short v);
CVar NativeToFakeluaUshort(State *s, unsigned short v);
CVar NativeToFakeluaInt(State *s, int v);
CVar NativeToFakeluaUint(State *s, unsigned int v);
CVar NativeToFakeluaLong(State *s, long v);
CVar NativeToFakeluaUlong(State *s, unsigned long v);
CVar NativeToFakeluaLonglong(State *s, long long v);
CVar NativeToFakeluaUlonglong(State *s, unsigned long long v);
CVar NativeToFakeluaFloat(State *s, float v);
CVar NativeToFakeluaDouble(State *s, double v);
CVar NativeToFakeluaCstr(State *s, const char *v);
CVar NativeToFakeluaStr(State *s, char *v);
CVar NativeToFakeluaString(State *s, const std::string &v);
CVar NativeToFakeluaStringview(State *s, const std::string_view &v);
CVar NativeToFakeluaObj(State *s, const VarInterface *v);

template<typename T>
CVar NativeToFakelua(State *s, T v) {
    // 检查 T 是否为 nil
    if constexpr (std::is_same_v<T, std::nullptr_t>) {
        return NativeToFakeluaNil(s);
    }
    // 检查 T 是否为 bool
    else if constexpr (std::is_same_v<T, bool>) {
        return NativeToFakeluaBool(s, v);
    }
    // 检查 T 是否为 char
    else if constexpr (std::is_same_v<T, char>) {
        return NativeToFakeluaChar(s, v);
    }
    // 检查 T 是否为 unsigned char
    else if constexpr (std::is_same_v<T, unsigned char>) {
        return NativeToFakeluaUchar(s, v);
    }
    // 检查 T 是否为 short
    else if constexpr (std::is_same_v<T, short>) {
        return NativeToFakeluaShort(s, v);
    }
    // 检查 T 是否为 unsigned short
    else if constexpr (std::is_same_v<T, unsigned short>) {
        return NativeToFakeluaUshort(s, v);
    }
    // 检查 T 是否为 int
    else if constexpr (std::is_same_v<T, int>) {
        return NativeToFakeluaInt(s, v);
    }
    // 检查 T 是否为 unsigned int
    else if constexpr (std::is_same_v<T, unsigned int>) {
        return NativeToFakeluaUint(s, v);
    }
    // 检查 T 是否为 long
    else if constexpr (std::is_same_v<T, long>) {
        return NativeToFakeluaLong(s, v);
    }
    // 检查 T 是否为 unsigned long
    else if constexpr (std::is_same_v<T, unsigned long>) {
        return NativeToFakeluaUlong(s, v);
    }
    // 检查 T 是否为 long long
    else if constexpr (std::is_same_v<T, long long>) {
        return NativeToFakeluaLonglong(s, v);
    }
    // 检查 T 是否为 unsigned long long
    else if constexpr (std::is_same_v<T, unsigned long long>) {
        return NativeToFakeluaUlonglong(s, v);
    }
    // 检查 T 是否为 float
    else if constexpr (std::is_same_v<T, float>) {
        return NativeToFakeluaFloat(s, v);
    }
    // 检查 T 是否为 double
    else if constexpr (std::is_same_v<T, double>) {
        return NativeToFakeluaDouble(s, v);
    }
    // 检查 T 是否为 const char *
    else if constexpr (std::is_same_v<T, const char *>) {
        return NativeToFakeluaCstr(s, v);
    }
    // 检查 T 是否为 char *
    else if constexpr (std::is_same_v<T, char *>) {
        return NativeToFakeluaStr(s, v);
    }
    // 检查 T 是否为 std::string
    else if constexpr (std::is_same_v<T, std::string>) {
        return NativeToFakeluaString(s, v);
    }
    // 检查 T 是否为 std::string_view
    else if constexpr (std::is_same_v<T, std::string_view>) {
        return NativeToFakeluaStringview(s, v);
    }
    // 检查 T 是否为 cvar
    else if constexpr (std::is_same_v<T, CVar>) {
        return v;
    } else {
        // 静态断言 T 应该是 VarInterface* 或实现 VarInterface
        static_assert(std::is_pointer_v<T>, "T should be pointer");
        static_assert(std::is_base_of_v<VarInterface, std::remove_pointer_t<T>>, "T should be VarInterface");
        return NativeToFakeluaObj(s, v);
    }
}

// FakeLua 转原生
bool FakeluaToNativeBool(State *s, CVar v);
char FakeluaToNativeChar(State *s, CVar v);
unsigned char FakeluaToNativeUchar(State *s, CVar v);
short FakeluaToNativeShort(State *s, CVar v);
unsigned short FakeluaToNativeUshort(State *s, CVar v);
int FakeluaToNativeInt(State *s, CVar v);
unsigned int FakeluaToNativeUint(State *s, CVar v);
long FakeluaToNativeLong(State *s, CVar v);
unsigned long FakeluaToNativeUlong(State *s, CVar v);
long long FakeluaToNativeLonglong(State *s, CVar v);
unsigned long long FakeluaToNativeUlonglong(State *s, CVar v);
float FakeluaToNativeFloat(State *s, CVar v);
double FakeluaToNativeDouble(State *s, CVar v);
const char *FakeluaToNativeCstr(State *s, CVar v);
const char *FakeluaToNativeStr(State *s, CVar v);
std::string FakeluaToNativeString(State *s, CVar v);
std::string_view FakeluaToNativeStringview(State *s, CVar v);
VarInterface *FakeluaToNativeObj(State *s, CVar v);

template<typename T>
T FakeluaToNative(State *s, const CVar v) {
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
        // 静态断言 T 应该是 VarInterface* 或实现 VarInterface
        static_assert(std::is_pointer_v<T>, "T should be pointer");
        static_assert(std::is_base_of_v<VarInterface, std::remove_pointer_t<T>>, "T should be VarInterface");
        return FakeluaToNativeObj(s, v);
    }
}

template<size_t I = 0, typename... Rets>
inline std::enable_if_t<I == sizeof...(Rets), void> FakeluaFuncRetHelper(State *s, CVar *ret, std::tuple<Rets &...> &rets) {
}

template<size_t I = 0, typename... Rets>
        inline std::enable_if_t < I<sizeof...(Rets), void> FakeluaFuncRetHelper(State *s, CVar *ret, std::tuple<Rets &...> &rets) {
    typedef std::remove_reference_t<std::tuple_element_t<I, std::tuple<Rets &...>>> t;
    std::get<I>(rets) = FakeluaToNative<t>(s, ret[I]);
    FakeluaFuncRetHelper<I + 1, Rets...>(s, ret, rets);
}

void Call(State *s, const std::string &name, CVar *args, size_t arg_size, CVar *rets, size_t ret_size);

}// namespace inter

// 调用函数
template<typename... Rets, typename... Args>
void Call(State *s, const std::string &name, std::tuple<Rets &...> &&rets, Args &&...args) {
    // 将参数传输到变量数组
    CVar args_array[sizeof...(Args)] = {inter::NativeToFakelua(s, std::forward<Args>(args))...};

    // 在栈中分配变量
    CVar rets_array[sizeof...(Rets)] = {};

    // 调用函数
    inter::Call(s, name, args_array, sizeof...(Args), rets_array, sizeof...(Rets));

    // 将变量传输到返回值
    inter::FakeluaFuncRetHelper(s, rets_array, rets);
}

}// namespace fakelua
