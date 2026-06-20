#pragma once

#include <algorithm>
#include <format>
#include <functional>
#include <memory>
#include <ranges>
#include <string>
#include <type_traits>

namespace fakelua {

constexpr size_t kMaxFunctionInputParams = 32;
constexpr size_t kMaxMathSpecializedParams = 8;

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
    std::string string_;
    std::vector<std::pair<VarInterface *, VarInterface *>> table_;
};

class VarTable;
class VarMulti;
class VarString;
class Var;

// 基本变量类型，包含类型标识和数据。数据使用 union 存储，根据类型标识来访问。
// 必须保持为 POD 类型，以确保 C JIT 代码 and C++ 宿主代码之间的 ABI 兼容性。
// 特别是 arm64 平台的非 POD 结构体返回值调用约定与 POD 不同。
struct CVar {
    int type_ = 0;
    int flag_ = 0;

    union cvar_data {
        bool b;
        int64_t i;
        double f;
        VarString *s;
        VarTable *t;
        VarMulti *m;
    };

    cvar_data data_{};
};

// 确保 CVar 是标准布局类型（POD），以匹配 C 代码中的定义
static_assert(std::is_standard_layout_v<CVar>, "CVar must be standard-layout for ABI compatibility");
static_assert(std::is_trivially_copyable_v<CVar>, "CVar must be trivially copyable for ABI compatibility");

// JIT类型
enum JITType {
    // TinyCC 是一个小型的 C 语言编译器，支持即时编译（JIT）。它的特点是编译速度快，适合于需要快速生成和执行代码的场景
    JIT_TCC = 0,
    // GCC 后端：将生成 C 代码通过系统 gcc 编译为动态库并加载执行
    JIT_GCC,
    JIT_MAX,
};

// 控制编译器的配置项
struct CompileConfig {
    // 跳过 JIT 编译。仅进行词法分析和语法解析。
    bool skip_jit = false;
    // 调试模式。如果为 true，JIT 代码将被转储到文件中。
    bool debug_mode = true;
    // 是否使用 JIT 编译，默认都开启
    bool disable_jit[JIT_MAX] = {false};
    // 记录生成的 C 代码（全局变量、函数声明、函数实现，不含公共头部）。
    // 开启后可通过 GetLastRecordedCCode(State*) 获取最近一次编译产生的代码片段。
    bool record_c_code = false;
};

struct StateTCCConfig {
    std::vector<std::string> include_paths = {"./include"};
    std::vector<std::string> library_paths = {"./lib"};
    std::vector<std::string> libraries;
};

struct StateGCCConfig {
    std::vector<std::string> include_paths;
    std::vector<std::string> library_paths;
    std::vector<std::string> libraries;
};

struct StateConfig {
    // tcc编译配置
    StateTCCConfig tcc_config;
    // gcc编译配置
    StateGCCConfig gcc_config;
};

class State;

// 创建 FakeLua 状态
State *FakeluaNewState(const StateConfig &cfg = {});

// 释放 FakeLua 状态
void FakeluaDeleteState(State *s);

// RAII 守卫：在作用域内自动管理 State 的创建与销毁
class FakeluaStateGuard {
public:
    explicit FakeluaStateGuard(const StateConfig &cfg = {}) : state_(FakeluaNewState(cfg)) {
    }

    ~FakeluaStateGuard() {
        if (state_ != nullptr) {
            FakeluaDeleteState(state_);
        }
    }

    // 独占所有权语义：不允许拷贝，避免同一个 State 被 double-free。
    FakeluaStateGuard(const FakeluaStateGuard &) = delete;
    FakeluaStateGuard &operator=(const FakeluaStateGuard &) = delete;

    // 允许移动：移动后源对象不再持有 state，析构时不会重复释放。
    FakeluaStateGuard(FakeluaStateGuard &&other) noexcept : state_(other.state_) {
        other.state_ = nullptr;
    }

    FakeluaStateGuard &operator=(FakeluaStateGuard &&other) noexcept {
        if (this != &other) {
            if (state_ != nullptr) {
                FakeluaDeleteState(state_);
            }
            state_ = other.state_;
            other.state_ = nullptr;
        }
        return *this;
    }

    [[nodiscard]] State *GetState() const {
        return state_;
    }

private:
    State *state_;
};

// 编译文件
void CompileFile(State *s, const std::string &filename, const CompileConfig &cfg);

// 编译字符串
void CompileString(State *s, const std::string &str, const CompileConfig &cfg);

// 获取最近一次编译时记录的 C 代码（仅在 CompileConfig::record_c_code 为 true 时有效）。
// 返回全局变量、函数声明和函数实现部分，不含公共头部。
std::string GetLastRecordedCCode(State *s);

// 调用某个脚本函数（定义在文件末尾）
template<typename Ret, typename... Args>
void Call(State *s, JITType type, const std::string_view &name, Ret &&ret, Args &&...args);

// 设置 VarInterface 构造实例函数
void SetVarInterfaceNewFunc(State *s, const std::function<VarInterface *()> &func);

// 获取 VarInterface 构造实例函数
std::function<VarInterface *()> &GetVarInterfaceNewFunc(State *s);

// 设置全局调试日志级别，注意所有状态都将被设置。
// 0: 关闭, 1: 错误, 2: 信息, 默认为错误。
void SetDebugLogLevel(int level);

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
CVar NativeToFakeluaStringView(State *s, const std::string_view &v);
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
        return NativeToFakeluaStringView(s, v);
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
std::string FakeluaToNativeString(State *s, CVar v);
std::string_view FakeluaToNativeStringView(State *s, CVar v);
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
    } else if constexpr (std::is_same_v<T, std::string>) {
        return FakeluaToNativeString(s, v);
    } else if constexpr (std::is_same_v<T, std::string_view>) {
        return FakeluaToNativeStringView(s, v);
    } else if constexpr (std::is_same_v<T, CVar>) {
        return v;
    } else {
        // 静态断言 T 应该是 VarInterface* 或实现 VarInterface
        static_assert(std::is_pointer_v<T>, "T should be pointer");
        static_assert(std::is_base_of_v<VarInterface, std::remove_pointer_t<T>>, "T should be VarInterface");
        return FakeluaToNativeObj(s, v);
    }
}

void *GetFuncAddr(State *s, JITType type, const std::string_view &name, int &arg_count, bool &is_vararg);

[[noreturn]] void ThrowInterFakeluaException(const std::string &msg);

int GetReentrantCount(State *s);

void AddReentrantCount(State *s);

void SubReentrantCount(State *s);

void Reset(State *s);

class ReentryCounter {
public:
    explicit ReentryCounter(State *s) : s_(s) {
        AddReentrantCount(s_);
    }

    ~ReentryCounter() {
        SubReentrantCount(s_);
    }

private:
    State *s_;
};

// ---------------------------------------------------------------------------
// Multi CVar 底层操作（供 Call() 内部使用）
// ---------------------------------------------------------------------------

// 分配一个含 count 个槽位的空 Multi CVar（各槽初始为 Nil）
CVar AllocMultiCVar(State *s, int count);

// 设置 Multi CVar 中第 idx 个槽的值（idx 必须在 [0, count) 内）
void SetMultiCVarElement(CVar &multi, int idx, CVar val);

// 读取 Multi CVar 中第 idx 个槽（越界返回 Nil）
CVar GetMultiCVarElement(const CVar &multi, int idx);

// 返回 Multi CVar 的元素个数（若不是 Multi 则返回 0）
int GetMultiCVarCount(const CVar &multi);

}// namespace inter

// ---------------------------------------------------------------------------
// std::tuple 支持：自动解包 Multi 返回值
// ---------------------------------------------------------------------------

template<typename T>
struct is_std_tuple : std::false_type {};
template<typename... Ts>
struct is_std_tuple<std::tuple<Ts...>> : std::true_type {};
template<typename T>
inline constexpr bool is_std_tuple_v = is_std_tuple<T>::value;

namespace inter {

template<typename Tuple, std::size_t... I>
void UnpackMultiToTuple(State *s, const CVar &ret_var, Tuple &tuple, std::index_sequence<I...>) {
    ((std::get<I>(tuple) = FakeluaToNative<std::remove_cvref_t<std::tuple_element_t<I, std::remove_cvref_t<Tuple>>>>(s, GetMultiCVarElement(ret_var, I))), ...);
}

}// namespace inter

// ---------------------------------------------------------------------------
// Call() — 统一调用入口
//
// 支持：
//   1. 普通调用：Call(s, type, "fn", ret, arg1, arg2)
//   2. 自动 vararg：Call(s, type, "sum", ret, 1, 2, 3)  -- 多余参数自动打包成 Multi
//   3. 多返回值：Call(s, type, "fn", std::tie(a, b, c))  -- 自动解包 Multi 到 tuple
// ---------------------------------------------------------------------------

#define CALLCVAR_0
#define CALLCVAR_1 CVar
#define CALLCVAR_2 CALLCVAR_1, CVar
#define CALLCVAR_3 CALLCVAR_2, CVar
#define CALLCVAR_4 CALLCVAR_3, CVar
#define CALLCVAR_5 CALLCVAR_4, CVar
#define CALLCVAR_6 CALLCVAR_5, CVar
#define CALLCVAR_7 CALLCVAR_6, CVar
#define CALLCVAR_8 CALLCVAR_7, CVar
#define CALLCVAR_9 CALLCVAR_8, CVar
#define CALLCVAR_10 CALLCVAR_9, CVar
#define CALLCVAR_11 CALLCVAR_10, CVar
#define CALLCVAR_12 CALLCVAR_11, CVar
#define CALLCVAR_13 CALLCVAR_12, CVar
#define CALLCVAR_14 CALLCVAR_13, CVar
#define CALLCVAR_15 CALLCVAR_14, CVar
#define CALLCVAR_16 CALLCVAR_15, CVar
#define CALLCVAR_17 CALLCVAR_16, CVar
#define CALLCVAR_18 CALLCVAR_17, CVar
#define CALLCVAR_19 CALLCVAR_18, CVar
#define CALLCVAR_20 CALLCVAR_19, CVar
#define CALLCVAR_21 CALLCVAR_20, CVar
#define CALLCVAR_22 CALLCVAR_21, CVar
#define CALLCVAR_23 CALLCVAR_22, CVar
#define CALLCVAR_24 CALLCVAR_23, CVar
#define CALLCVAR_25 CALLCVAR_24, CVar
#define CALLCVAR_26 CALLCVAR_25, CVar
#define CALLCVAR_27 CALLCVAR_26, CVar
#define CALLCVAR_28 CALLCVAR_27, CVar
#define CALLCVAR_29 CALLCVAR_28, CVar
#define CALLCVAR_30 CALLCVAR_29, CVar
#define CALLCVAR_31 CALLCVAR_30, CVar
#define CALLCVAR_32 CALLCVAR_31, CVar

#define CALLARG_0
#define CALLARG_1 call_cvars[0]
#define CALLARG_2 CALLARG_1, call_cvars[1]
#define CALLARG_3 CALLARG_2, call_cvars[2]
#define CALLARG_4 CALLARG_3, call_cvars[3]
#define CALLARG_5 CALLARG_4, call_cvars[4]
#define CALLARG_6 CALLARG_5, call_cvars[5]
#define CALLARG_7 CALLARG_6, call_cvars[6]
#define CALLARG_8 CALLARG_7, call_cvars[7]
#define CALLARG_9 CALLARG_8, call_cvars[8]
#define CALLARG_10 CALLARG_9, call_cvars[9]
#define CALLARG_11 CALLARG_10, call_cvars[10]
#define CALLARG_12 CALLARG_11, call_cvars[11]
#define CALLARG_13 CALLARG_12, call_cvars[12]
#define CALLARG_14 CALLARG_13, call_cvars[13]
#define CALLARG_15 CALLARG_14, call_cvars[14]
#define CALLARG_16 CALLARG_15, call_cvars[15]
#define CALLARG_17 CALLARG_16, call_cvars[16]
#define CALLARG_18 CALLARG_17, call_cvars[17]
#define CALLARG_19 CALLARG_18, call_cvars[18]
#define CALLARG_20 CALLARG_19, call_cvars[19]
#define CALLARG_21 CALLARG_20, call_cvars[20]
#define CALLARG_22 CALLARG_21, call_cvars[21]
#define CALLARG_23 CALLARG_22, call_cvars[22]
#define CALLARG_24 CALLARG_23, call_cvars[23]
#define CALLARG_25 CALLARG_24, call_cvars[24]
#define CALLARG_26 CALLARG_25, call_cvars[25]
#define CALLARG_27 CALLARG_26, call_cvars[26]
#define CALLARG_28 CALLARG_27, call_cvars[27]
#define CALLARG_29 CALLARG_28, call_cvars[28]
#define CALLARG_30 CALLARG_29, call_cvars[29]
#define CALLARG_31 CALLARG_30, call_cvars[30]
#define CALLARG_32 CALLARG_31, call_cvars[31]

template<typename Ret, typename... Args>
void Call(State *s, JITType type, const std::string_view &name, Ret &&ret, Args &&...args) {
    using RetType = std::remove_cvref_t<Ret>;
    int arg_count = 0;
    bool is_vararg = false;
    const auto addr = inter::GetFuncAddr(s, type, name, arg_count, is_vararg);
    if (!addr) {
        inter::ThrowInterFakeluaException(std::format("Call failed, function {} not found", name));
    }

    const int user_arg_count = static_cast<int>(sizeof...(Args));
    const int fixed_count = is_vararg ? arg_count - 1 : arg_count;

    if (__builtin_expect(!is_vararg && user_arg_count != arg_count, 0)) {
        inter::ThrowInterFakeluaException(std::format("Call failed, function {} arg count not match, need {} get {}", name, arg_count, user_arg_count));
    }

    if (const auto reentrant_count = inter::GetReentrantCount(s); !reentrant_count) {
        bool has_multi = false;
        if constexpr (sizeof...(Args) > 0) {
            auto check_multi = [](auto &&arg) -> bool {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, CVar>) {
                    return arg.type_ == 7;
                } else {
                    return false;
                }
            };
            has_multi = (check_multi(std::forward<Args>(args)) || ...);
        }
        if (!has_multi) {
            inter::Reset(s);
        }
    }
    inter::ReentryCounter rc(s);

    CVar ret_var;

    if (is_vararg) {
        // 自动 vararg 路径：将多余参数打包为 Multi
        CVar raw_cvars[kMaxFunctionInputParams] = {};
        if constexpr (sizeof...(Args) > 0) {
            int idx = 0;
            ((raw_cvars[idx++] = inter::NativeToFakelua(s, std::forward<Args>(args))), ...);
        }

        CVar call_cvars[kMaxFunctionInputParams];
        for (int i = 0; i < fixed_count; ++i) {
            call_cvars[i] = raw_cvars[i];
        }
        const int vararg_count = user_arg_count - fixed_count;
        CVar multi = inter::AllocMultiCVar(s, vararg_count > 0 ? vararg_count : 0);
        for (int i = 0; i < vararg_count; ++i) {
            inter::SetMultiCVarElement(multi, i, raw_cvars[fixed_count + i]);
        }
        call_cvars[fixed_count] = multi;

        switch (arg_count) {
#define CALL_DISPATCH(N) case N: ret_var = reinterpret_cast<CVar (*)(CALLCVAR_##N)>(addr)(CALLARG_##N); break;
            CALL_DISPATCH(0)
            CALL_DISPATCH(1)
            CALL_DISPATCH(2)
            CALL_DISPATCH(3)
            CALL_DISPATCH(4)
            CALL_DISPATCH(5)
            CALL_DISPATCH(6)
            CALL_DISPATCH(7)
            CALL_DISPATCH(8)
            CALL_DISPATCH(9)
            CALL_DISPATCH(10)
            CALL_DISPATCH(11)
            CALL_DISPATCH(12)
            CALL_DISPATCH(13)
            CALL_DISPATCH(14)
            CALL_DISPATCH(15)
            CALL_DISPATCH(16)
            CALL_DISPATCH(17)
            CALL_DISPATCH(18)
            CALL_DISPATCH(19)
            CALL_DISPATCH(20)
            CALL_DISPATCH(21)
            CALL_DISPATCH(22)
            CALL_DISPATCH(23)
            CALL_DISPATCH(24)
            CALL_DISPATCH(25)
            CALL_DISPATCH(26)
            CALL_DISPATCH(27)
            CALL_DISPATCH(28)
            CALL_DISPATCH(29)
            CALL_DISPATCH(30)
            CALL_DISPATCH(31)
            CALL_DISPATCH(32)
#undef CALL_DISPATCH
            default:
                inter::ThrowInterFakeluaException(std::format("Call failed, function {} has too many compiled params {}", name, arg_count));
        }
    } else {
        // 非 vararg 编译期快速路径
        CVar call_cvars[kMaxFunctionInputParams] = {};
        if constexpr (sizeof...(Args) > 0) {
            int idx = 0;
            ((call_cvars[idx++] = inter::NativeToFakelua(s, std::forward<Args>(args))), ...);
        }

        switch (arg_count) {
#define CALL_DISPATCH(N) case N: ret_var = reinterpret_cast<CVar (*)(CALLCVAR_##N)>(addr)(CALLARG_##N); break;
            CALL_DISPATCH(0)
            CALL_DISPATCH(1)
            CALL_DISPATCH(2)
            CALL_DISPATCH(3)
            CALL_DISPATCH(4)
            CALL_DISPATCH(5)
            CALL_DISPATCH(6)
            CALL_DISPATCH(7)
            CALL_DISPATCH(8)
            CALL_DISPATCH(9)
            CALL_DISPATCH(10)
            CALL_DISPATCH(11)
            CALL_DISPATCH(12)
            CALL_DISPATCH(13)
            CALL_DISPATCH(14)
            CALL_DISPATCH(15)
            CALL_DISPATCH(16)
            CALL_DISPATCH(17)
            CALL_DISPATCH(18)
            CALL_DISPATCH(19)
            CALL_DISPATCH(20)
            CALL_DISPATCH(21)
            CALL_DISPATCH(22)
            CALL_DISPATCH(23)
            CALL_DISPATCH(24)
            CALL_DISPATCH(25)
            CALL_DISPATCH(26)
            CALL_DISPATCH(27)
            CALL_DISPATCH(28)
            CALL_DISPATCH(29)
            CALL_DISPATCH(30)
            CALL_DISPATCH(31)
            CALL_DISPATCH(32)
#undef CALL_DISPATCH
            default:
                static_assert(sizeof...(Args) <= kMaxFunctionInputParams, "Too many arguments for Call()");
        }
    }

    // 返回值处理：自动解包 tuple / 单值
    if constexpr (is_std_tuple_v<RetType>) {
        constexpr std::size_t N = std::tuple_size_v<RetType>;
        inter::UnpackMultiToTuple(s, ret_var, ret, std::make_index_sequence<N>{});
    } else {
        ret = inter::FakeluaToNative<RetType>(s, ret_var);
    }
}

#undef CALLCVAR_0
#undef CALLCVAR_1
#undef CALLCVAR_2
#undef CALLCVAR_3
#undef CALLCVAR_4
#undef CALLCVAR_5
#undef CALLCVAR_6
#undef CALLCVAR_7
#undef CALLCVAR_8
#undef CALLCVAR_9
#undef CALLCVAR_10
#undef CALLCVAR_11
#undef CALLCVAR_12
#undef CALLCVAR_13
#undef CALLCVAR_14
#undef CALLCVAR_15
#undef CALLCVAR_16
#undef CALLCVAR_17
#undef CALLCVAR_18
#undef CALLCVAR_19
#undef CALLCVAR_20
#undef CALLCVAR_21
#undef CALLCVAR_22
#undef CALLCVAR_23
#undef CALLCVAR_24
#undef CALLCVAR_25
#undef CALLCVAR_26
#undef CALLCVAR_27
#undef CALLCVAR_28
#undef CALLCVAR_29
#undef CALLCVAR_30
#undef CALLCVAR_31
#undef CALLCVAR_32
#undef CALLARG_0
#undef CALLARG_1
#undef CALLARG_2
#undef CALLARG_3
#undef CALLARG_4
#undef CALLARG_5
#undef CALLARG_6
#undef CALLARG_7
#undef CALLARG_8
#undef CALLARG_9
#undef CALLARG_10
#undef CALLARG_11
#undef CALLARG_12
#undef CALLARG_13
#undef CALLARG_14
#undef CALLARG_15
#undef CALLARG_16
#undef CALLARG_17
#undef CALLARG_18
#undef CALLARG_19
#undef CALLARG_20
#undef CALLARG_21
#undef CALLARG_22
#undef CALLARG_23
#undef CALLARG_24
#undef CALLARG_25
#undef CALLARG_26
#undef CALLARG_27
#undef CALLARG_28
#undef CALLARG_29
#undef CALLARG_30
#undef CALLARG_31
#undef CALLARG_32

}// namespace fakelua
