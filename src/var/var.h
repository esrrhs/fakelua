#pragma once

#include "fakelua.h"
#include "util/common.h"
#include "var_type.h"

namespace fakelua {

class State;
class VarString;
class VarTable;

// Var 是 FakeLua 中用于存储各种数据类型的通用变量类。
// 在JIT中使用的是CVar，Var是同等内存布局的结构，只在JIT之外使用。
class Var final : public CVar {
public:
    Var() = default;

    // 构造布尔型变量
    explicit Var(bool val) {
        SetBool(val);
    }

    // 构造整型变量
    explicit Var(int64_t val) {
        SetInt(val);
    }

    // 构造浮点型变量
    explicit Var(double val) {
        SetFloat(val);
    }

    // 重载等于操作符
    bool operator==(const Var &rhs_var) const {
        return Equal(rhs_var);
    }

    // 获取变量类型
    [[nodiscard]] VarType Type() const {
        return static_cast<VarType>(type_);
    }

    // 获取布尔值（仅在类型匹配时有效）
    [[nodiscard]] bool GetBool() const {
        DEBUG_ASSERT(type_ == static_cast<int>(VarType::Bool));
        return data_.b;
    }

    // 获取整数值（仅在类型匹配时有效）
    [[nodiscard]] int64_t GetInt() const {
        DEBUG_ASSERT(type_ == static_cast<int>(VarType::Int));
        return data_.i;
    }

    // 获取浮点数值（仅在类型匹配时有效）
    [[nodiscard]] double GetFloat() const {
        DEBUG_ASSERT(type_ == static_cast<int>(VarType::Float));
        return data_.f;
    }

    // 获取字符串对象（仅在类型匹配时有效）
    [[nodiscard]] VarString *GetString() const;

    // 获取表对象（仅在类型匹配时有效）
    [[nodiscard]] VarTable *GetTable() const {
        DEBUG_ASSERT(type_ == static_cast<int>(VarType::Table));
        return data_.t;
    }

    // 获取闭包对象（仅在类型匹配时有效）
    [[nodiscard]] VarClosure *GetClosure() const {
        DEBUG_ASSERT(type_ == static_cast<int>(VarType::Closure));
        return data_.cl;
    }

public:
    // 设置为 nil
    void SetNil() {
        type_ = static_cast<int>(VarType::Nil);
    }

    // 设置布尔值
    void SetBool(bool val) {
        type_ = static_cast<int>(VarType::Bool);
        data_.b = val;
    }

    // 设置整数值
    void SetInt(int64_t val) {
        type_ = static_cast<int>(VarType::Int);
        data_.i = val;
    }

    // 设置浮点数值，始终按浮点存储
    void SetFloat(double val) {
        type_ = static_cast<int>(VarType::Float);
        data_.f = val;
    }

    // 设置临时字符串值
    void SetTempString(State *state, const std::string_view &val);

    // 设置常量字符串值
    void SetConstString(State *state, const std::string_view &val);

    // 创建并设置新表
    void SetTable(State *state);

    // 检查变量在逻辑判断中是否为真（Lua 规则：除了 nil 和 false 之外都为真）
    [[nodiscard]] bool TestTrue() const;

public:
    // 转换为字符串显示
    [[nodiscard]] std::string ToString(bool has_quote = true, bool has_postfix = true) const;

    // 获取哈希值
    [[nodiscard]] size_t Hash() const;

    // 比较两个变量是否相等
    [[nodiscard]] bool Equal(const Var &rhs) const;

    // 表元素设置
    void TableSet(State *state, const Var &key, const Var &val, bool can_be_nil) const;

    // 获取表元素
    [[nodiscard]] Var TableGet(const Var &key) const;

    // 获取表大小
    [[nodiscard]] size_t TableSize() const;

private:
    void CheckTable(const char *op) const;
};

// 确保 Var 的大小为 16 字节，与 gccjit 中定义的 CVar 一致
static_assert(sizeof(Var) == sizeof(CVar));
static_assert(sizeof(Var) == 16);

extern Var const_null_var;

}// namespace fakelua
