#pragma once

#include "fakelua.h"
#include "util/common.h"
#include "var_type.h"

namespace fakelua {

class State;
class VarString;
class VarTable;

// Var is the class that holds the multiple types of data.
class Var final : public CVar {
public:
    Var() = default;

    explicit Var(bool val) {
        SetBool(val);
    }

    explicit Var(int64_t val) {
        SetInt(val);
    }

    explicit Var(double val) {
        SetFloat(val);
    }

    explicit Var(const FakeluaStatePtr &s, const std::string_view &val) {
        SetString(s, val);
    }

    bool operator==(const Var &r) const {
        return Equal(r);
    }

    // get the var type
    [[nodiscard]] VarType Type() const {
        return static_cast<VarType>(type_);
    }

    // get bool value
    [[nodiscard]] bool GetBool() const {
        DEBUG_ASSERT(type_ == static_cast<int>(VarType::Bool));
        return data_.b;
    }

    // get int value
    [[nodiscard]] int64_t GetInt() const {
        DEBUG_ASSERT(type_ == static_cast<int>(VarType::Int));
        return data_.i;
    }

    // get float value
    [[nodiscard]] double GetFloat() const {
        DEBUG_ASSERT(type_ == static_cast<int>(VarType::Float));
        return data_.f;
    }

    // get string_view value
    [[nodiscard]] VarString *GetString() const {
        DEBUG_ASSERT(type_ == static_cast<int>(VarType::String));
        return data_.s;
    }

    // get table value
    [[nodiscard]] VarTable *GetTable() const {
        DEBUG_ASSERT(type_ == static_cast<int>(VarType::Table));
        return data_.t;
    }

public:
    // set nullptr
    void SetNil() {
        type_ = static_cast<int>(VarType::Nil);
    }

    // set bool value
    void SetBool(bool val) {
        type_ = static_cast<int>(VarType::Bool);
        data_.b = val;
    }

    // set int value
    void SetInt(int64_t val) {
        type_ = static_cast<int>(VarType::Int);
        data_.i = val;
    }

    // set float value
    void SetFloat(double val) {
        type_ = static_cast<int>(VarType::Float);
        data_.f = val;
    }

    // set string value
    void SetString(const FakeluaStatePtr &s, const std::string_view &val);

    // set string value
    void SetString(FakeluaState *s, const std::string_view &val);

    // set table value
    void SetTable(const FakeluaStatePtr &s);

    // set table value
    void SetTable(FakeluaState *s);

public:
    // to string
    [[nodiscard]] std::string ToString(bool has_quote = true, bool has_postfix = true) const;

    // get hash value
    [[nodiscard]] size_t Hash() const;

    // equal
    [[nodiscard]] bool Equal(const Var &rhs) const;

    // is calculable
    [[nodiscard]] bool IsCalculable() const;

    // is calculable integer
    [[nodiscard]] bool IsCalculableInteger() const;

    // get calculable integer, only call this when IsCalculableInteger() is true
    [[nodiscard]] int64_t GetCalculableInt() const;

    // get calculable number value, maybe integer or float or string
    [[nodiscard]] double GetCalculableNumber() const;

public:
    void Plus(const Var &rhs, Var &result) const;

    void Minus(const Var &rhs, Var &result) const;

    void Star(const Var &rhs, Var &result) const;

    void Slash(const Var &rhs, Var &result) const;

    void DoubleSlash(const Var &rhs, Var &result) const;

    void Pow(const Var &rhs, Var &result) const;

    void Mod(const Var &rhs, Var &result) const;

    void Bitand(const Var &rhs, Var &result) const;

    void Xor(const Var &rhs, Var &result) const;

    void Bitor(const Var &rhs, Var &result) const;

    void RightShift(const Var &rhs, Var &result) const;

    void LeftShift(const Var &rhs, Var &result) const;

    void Concat(FakeluaState *s, const Var &rhs, Var &result) const;

    void Less(const Var &rhs, Var &result) const;

    void LessEqual(const Var &rhs, Var &result) const;

    void More(const Var &rhs, Var &result) const;

    void MoreEqual(const Var &rhs, Var &result) const;

    void Equal(const Var &rhs, Var &result) const;

    void NotEqual(const Var &rhs, Var &result) const;

    [[nodiscard]] bool TestTrue() const;

    void UnopMinus(Var &result) const;

    void UnopNot(Var &result) const;

    void UnopNumberSign(Var &result) const;

    void UnopBitnot(Var &result) const;

    void TableSet(const Var &key, const Var &val, bool can_be_nil) const;

    [[nodiscard]] Var TableGet(const Var &key) const;

    [[nodiscard]] size_t TableSize() const;

    [[nodiscard]] Var TableKeyAt(size_t pos) const;

    [[nodiscard]] Var TableValueAt(size_t pos) const;
};

// assert var size is 16 bytes, the same as we defined in gccjit
static_assert(sizeof(Var) == sizeof(CVar));
static_assert(sizeof(Var) == 16);

extern Var const_null_var;

}// namespace fakelua
