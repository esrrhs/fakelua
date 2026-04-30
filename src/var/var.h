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

public:
    // 转换为字符串显示
    [[nodiscard]] std::string ToString(bool has_quote = true, bool has_postfix = true) const;

    // 获取哈希值
    [[nodiscard]] size_t Hash() const;

    // 比较两个变量是否相等
    [[nodiscard]] bool Equal(const Var &rhs) const;

    // 检查变量是否可以参与算术运算（数字或数字格式的字符串）
    [[nodiscard]] bool IsCalculable() const;

    // 检查变量是否可以作为整数参与运算
    [[nodiscard]] bool IsCalculableInteger() const;

    // 获取计算用的整数值，仅在 IsCalculableInteger() 为真时调用
    [[nodiscard]] int64_t GetCalculableInt() const;

    // 获取计算用的浮点数值（支持数字、字符串转换）
    [[nodiscard]] double GetCalculableNumber() const;

public:
    // 加法运算
    void Plus(const Var &rhs, Var &result) const;

    // 减法运算
    void Minus(const Var &rhs, Var &result) const;

    // 乘法运算
    void Star(const Var &rhs, Var &result) const;

    // 除法运算
    void Slash(const Var &rhs, Var &result) const;

    // 整除运算
    void DoubleSlash(const Var &rhs, Var &result) const;

    // 幂运算
    void Pow(const Var &rhs, Var &result) const;

    // 取模运算
    void Mod(const Var &rhs, Var &result) const;

    // 位与运算
    void Bitand(const Var &rhs, Var &result) const;

    // 位异或运算
    void Xor(const Var &rhs, Var &result) const;

    // 位或运算
    void Bitor(const Var &rhs, Var &result) const;

    // 右移运算
    void RightShift(const Var &rhs, Var &result) const;

    // 左移运算
    void LeftShift(const Var &rhs, Var &result) const;

    // 字符串连接运算
    void Concat(State *state, const Var &rhs, Var &result) const;

    // 小于判断
    void Less(const Var &rhs, Var &result) const;

    // 小于等于判断
    void LessEqual(const Var &rhs, Var &result) const;

    // 大于判断
    void More(const Var &rhs, Var &result) const;

    // 大于等于判断
    void MoreEqual(const Var &rhs, Var &result) const;

    // 等于判断
    void Equal(const Var &rhs, Var &result) const;

    // 不等于判断
    void NotEqual(const Var &rhs, Var &result) const;

    // 检查变量在逻辑判断中是否为真（Lua 规则：除了 nil 和 false 之外都为真）
    [[nodiscard]] bool TestTrue() const;

    // 一元减（取负）
    void UnopMinus(Var &result) const;

    // 一元非（取反）
    void UnopNot(Var &result) const;

    // 长度运算符 #
    void UnopNumberSign(Var &result) const;

    // 位取反运算
    void UnopBitnot(Var &result) const;

    // 表元素设置
    void TableSet(State *state, const Var &key, const Var &val, bool can_be_nil) const;

    // 获取表元素
    [[nodiscard]] Var TableGet(const Var &key) const;

    // 获取表大小
    [[nodiscard]] size_t TableSize() const;

private:
    // 尝试将数值类型变量转换为整数；仅对 Int 和整数值的 Float 成功。
    bool TryConvertNumberToInteger(int64_t &out) const;
};

// 确保 Var 的大小为 16 字节，与 gccjit 中定义的 CVar 一致
static_assert(sizeof(Var) == sizeof(CVar));
static_assert(sizeof(Var) == 16);

extern Var const_null_var;

}// namespace fakelua
