#include "var.h"
#include "fakelua.h"
#include "state/State.h"
#include "state/var_string_heap.h"
#include "util/common.h"
#include "var_string.h"
#include "var_table.h"

namespace fakelua {

Var const_null_var;

void Var::SetString(const FakeluaStatePtr &s, const std::string_view &val) {
    SetString(s.get(), val);
}

void Var::SetString(FakeluaState *s, const std::string_view &val) {
    type_ = static_cast<int>(VarType::String);
    auto &string_heap = dynamic_cast<State *>(s)->get_var_string_heap();
    data_.s = string_heap.alloc(val);
}

void Var::SetTable(const FakeluaStatePtr &s) {
    SetTable(s.get());
}

void Var::SetTable(FakeluaState *s) {
    type_ = static_cast<int>(VarType::Table);
    auto &table_heap = dynamic_cast<State *>(s)->get_var_table_heap();
    data_.t = table_heap.alloc();
}

std::string Var::ToString(bool has_quote, bool has_postfix) const {
    std::string ret;
    DEBUG_ASSERT(Type() >= VarType::Min && Type() <= VarType::Max);
    switch (Type()) {
        case VarType::Nil:
            ret = "nil";
            break;
        case VarType::Bool:
            ret = GetBool() ? "true" : "false";
            break;
        case VarType::Int:
            ret = std::to_string(data_.i);
            break;
        case VarType::Float: {
            ret = std::format("{}", data_.f);
            break;
        }
        case VarType::String:
            ret = has_quote ? std::format("\"{}\"", data_.s->Str()) : std::format("{}", data_.s->Str());
            break;
        case VarType::Table:
            ret = std::format("table({})", static_cast<void *>(data_.t));
            break;
    }

    return ret;
}

size_t Var::Hash() const {
    switch (Type()) {
        case VarType::Nil:
            return 0;
        case VarType::Bool:
            return GetBool() ? 1 : 0;
        case VarType::Int:
            return std::hash<int64_t>()(data_.i);
        case VarType::Float:
            return std::hash<double>()(data_.f);
        case VarType::String:
            return std::hash<std::string_view>()(data_.s->Str());
        case VarType::Table:
            return std::hash<size_t>()(reinterpret_cast<size_t>(data_.t));
        default:
            return 0;
    }
}

bool Var::Equal(const Var &rhs) const {
    if (Type() != rhs.Type()) {
        return false;
    }

    switch (Type()) {
        case VarType::Nil:
            return true;
        case VarType::Bool:
            return data_.b == rhs.data_.b;
        case VarType::Int:
            return data_.i == rhs.data_.i;
        case VarType::Float:
            if (std::isnan(data_.f) && std::isnan(rhs.data_.f)) {
                return true;
            }
            return data_.f == rhs.data_.f;
        case VarType::String:
            return data_.s == rhs.data_.s;
        case VarType::Table:
            return data_.t == rhs.data_.t;
        default:
            return false;
    }
}

bool Var::IsCalculable() const {
    return Type() == VarType::Int || Type() == VarType::Float || (Type() == VarType::String && IsNumber(data_.s->Str()));
}

bool Var::IsCalculableInteger() const {
    return Type() == VarType::Int || (Type() == VarType::String && IsInteger(data_.s->Str()));
}

int64_t Var::GetCalculableInt() const {
    if (Type() == VarType::Int) {
        return GetInt();
    } else {// if (Type() == VarType::String)
        DEBUG_ASSERT(Type() == VarType::String);
        DEBUG_ASSERT(IsInteger(data_.s->Str()));
        return ToInteger(data_.s->Str());
    }
}

double Var::GetCalculableNumber() const {
    if (type_ == static_cast<int>(VarType::Int)) {
        return static_cast<double>(data_.i);
    } else if (type_ == static_cast<int>(VarType::Float)) {
        return data_.f;
    } else {// if (type_ == VarType::String)
        DEBUG_ASSERT(type_ == static_cast<int>(VarType::String));
        DEBUG_ASSERT(IsNumber(data_.s->Str()));
        return ToFloat(data_.s->Str());
    }
}

void Var::Plus(const Var &rhs, Var &result) const {
    if (!IsCalculable() || !rhs.IsCalculable()) {
        ThrowFakeluaException(std::format("operand of '+' must be number, got {} {}, {} {}", VarTypeToString(Type()), ToString(),
                                          VarTypeToString(rhs.Type()), rhs.ToString()));
    }
    if (IsCalculableInteger() && rhs.IsCalculableInteger()) {
        result.SetInt(GetCalculableInt() + rhs.GetCalculableInt());
    } else {
        result.SetFloat(GetCalculableNumber() + rhs.GetCalculableNumber());
    }
}

void Var::Minus(const Var &rhs, Var &result) const {
    if (!IsCalculable() || !rhs.IsCalculable()) {
        ThrowFakeluaException(std::format("operand of '-' must be number, got {} {}, {} {}", VarTypeToString(Type()), ToString(),
                                          VarTypeToString(rhs.Type()), rhs.ToString()));
    }
    if (IsCalculableInteger() && rhs.IsCalculableInteger()) {
        result.SetInt(GetCalculableInt() - rhs.GetCalculableInt());
    } else {
        result.SetFloat(GetCalculableNumber() - rhs.GetCalculableNumber());
    }
}

void Var::Star(const Var &rhs, Var &result) const {
    if (!IsCalculable() || !rhs.IsCalculable()) {
        ThrowFakeluaException(std::format("operand of '*' must be number, got {} {}, {} {}", VarTypeToString(Type()), ToString(),
                                          VarTypeToString(rhs.Type()), rhs.ToString()));
    }
    if (IsCalculableInteger() && rhs.IsCalculableInteger()) {
        result.SetInt(GetCalculableInt() * rhs.GetCalculableInt());
    } else {
        result.SetFloat(GetCalculableNumber() * rhs.GetCalculableNumber());
    }
}

void Var::Slash(const Var &rhs, Var &result) const {
    if (!IsCalculable() || !rhs.IsCalculable()) {
        ThrowFakeluaException(std::format("operand of '/' must be number, got {} {}, {} {}", VarTypeToString(Type()), ToString(),
                                          VarTypeToString(rhs.Type()), rhs.ToString()));
    }
    result.SetFloat(GetCalculableNumber() / rhs.GetCalculableNumber());
}

void Var::DoubleSlash(const Var &rhs, Var &result) const {
    if (!IsCalculable() || !rhs.IsCalculable()) {
        ThrowFakeluaException(std::format("operand of '//' must be number, got {} {}, {} {}", VarTypeToString(Type()), ToString(),
                                          VarTypeToString(rhs.Type()), rhs.ToString()));
    }
    if (IsCalculableInteger() && rhs.IsCalculableInteger()) {
        result.SetInt(GetCalculableInt() / rhs.GetCalculableInt());
    } else {
        result.SetFloat(std::floor(GetCalculableNumber() / rhs.GetCalculableNumber()));
    }
}

void Var::Pow(const Var &rhs, Var &result) const {
    if (!IsCalculable() || !rhs.IsCalculable()) {
        ThrowFakeluaException(std::format("operand of '^' must be number, got {} {}, {} {}", VarTypeToString(Type()), ToString(),
                                          VarTypeToString(rhs.Type()), rhs.ToString()));
    }
    result.SetFloat(std::pow(GetCalculableNumber(), rhs.GetCalculableNumber()));
}

void Var::Mod(const Var &rhs, Var &result) const {
    if (!IsCalculable() || !rhs.IsCalculable()) {
        ThrowFakeluaException(std::format("operand of '%' must be number, got {} {}, {} {}", VarTypeToString(Type()), ToString(),
                                          VarTypeToString(rhs.Type()), rhs.ToString()));
    }
    if (IsCalculableInteger() && rhs.IsCalculableInteger()) {
        result.SetInt(GetCalculableInt() % rhs.GetCalculableInt());
    } else {
        result.SetFloat(std::fmod(GetCalculableNumber(), rhs.GetCalculableNumber()));
    }
}

void Var::Bitand(const Var &rhs, Var &result) const {
    if (!IsCalculableInteger() || !rhs.IsCalculableInteger()) {
        ThrowFakeluaException(std::format("operand of '&' must be integer, got {} {}, {} {}", VarTypeToString(Type()), ToString(),
                                          VarTypeToString(rhs.Type()), rhs.ToString()));
    }
    result.SetInt(GetCalculableInt() & rhs.GetCalculableInt());
}

void Var::Xor(const Var &rhs, Var &result) const {
    if (!IsCalculableInteger() || !rhs.IsCalculableInteger()) {
        ThrowFakeluaException(std::format("operand of '~' must be integer, got {} {}, {} {}", VarTypeToString(Type()), ToString(),
                                          VarTypeToString(rhs.Type()), rhs.ToString()));
    }
    result.SetInt(GetCalculableInt() ^ rhs.GetCalculableInt());
}

void Var::Bitor(const Var &rhs, Var &result) const {
    if (!IsCalculableInteger() || !rhs.IsCalculableInteger()) {
        ThrowFakeluaException(std::format("operand of '|' must be integer, got {} {}, {} {}", VarTypeToString(Type()), ToString(),
                                          VarTypeToString(rhs.Type()), rhs.ToString()));
    }
    result.SetInt(GetCalculableInt() | rhs.GetCalculableInt());
}

void Var::RightShift(const Var &rhs, Var &result) const {
    if (!IsCalculableInteger() || !rhs.IsCalculableInteger()) {
        ThrowFakeluaException(std::format("operand of '>>' must be integer, got {} {}, {} {}", VarTypeToString(Type()), ToString(),
                                          VarTypeToString(rhs.Type()), rhs.ToString()));
    }
    auto shift = rhs.GetCalculableInt();
    if (shift >= 0) {
        result.SetInt(static_cast<int64_t>(static_cast<uint64_t>(GetCalculableInt()) >> shift));
    } else {
        result.SetInt(static_cast<int64_t>(static_cast<uint64_t>(GetCalculableInt()) << (-shift)));
    }
}

void Var::LeftShift(const Var &rhs, Var &result) const {
    if (!IsCalculableInteger() || !rhs.IsCalculableInteger()) {
        ThrowFakeluaException(std::format("operand of '<<' must be integer, got {} {}, {} {}", VarTypeToString(Type()), ToString(),
                                          VarTypeToString(rhs.Type()), rhs.ToString()));
    }
    auto shift = rhs.GetCalculableInt();
    if (shift >= 0) {
        result.SetInt(static_cast<int64_t>(static_cast<uint64_t>(GetCalculableInt()) << shift));
    } else {
        result.SetInt(static_cast<int64_t>(static_cast<uint64_t>(GetCalculableInt()) >> (-shift)));
    }
}

void Var::Concat(FakeluaState *s, const Var &rhs, Var &result) const {
    result.SetString(s, std::format("{}{}", ToString(false, false), rhs.ToString(false, false)));
}

void Var::Less(const Var &rhs, Var &result) const {
    if (!IsCalculable() || !rhs.IsCalculable()) {
        ThrowFakeluaException(std::format("operand of '<' must be number, got {} {}, {} {}", VarTypeToString(Type()), ToString(),
                                          VarTypeToString(rhs.Type()), rhs.ToString()));
    }

    if (IsCalculableInteger() && rhs.IsCalculableInteger()) {
        result.SetBool(GetCalculableInt() < rhs.GetCalculableInt());
    } else {
        result.SetBool(GetCalculableNumber() < rhs.GetCalculableNumber());
    }
}

void Var::LessEqual(const Var &rhs, Var &result) const {
    if (!IsCalculable() || !rhs.IsCalculable()) {
        ThrowFakeluaException(std::format("operand of '<=' must be number, got {} {}, {} {}", VarTypeToString(Type()), ToString(),
                                          VarTypeToString(rhs.Type()), rhs.ToString()));
    }

    if (IsCalculableInteger() && rhs.IsCalculableInteger()) {
        result.SetBool(GetCalculableInt() <= rhs.GetCalculableInt());
    } else {
        result.SetBool(GetCalculableNumber() <= rhs.GetCalculableNumber());
    }
}

void Var::More(const Var &rhs, Var &result) const {
    if (!IsCalculable() || !rhs.IsCalculable()) {
        ThrowFakeluaException(std::format("operand of '>' must be number, got {} {}, {} {}", VarTypeToString(Type()), ToString(),
                                          VarTypeToString(rhs.Type()), rhs.ToString()));
    }

    if (IsCalculableInteger() && rhs.IsCalculableInteger()) {
        result.SetBool(GetCalculableInt() > rhs.GetCalculableInt());
    } else {
        result.SetBool(GetCalculableNumber() > rhs.GetCalculableNumber());
    }
}

void Var::MoreEqual(const Var &rhs, Var &result) const {
    if (!IsCalculable() || !rhs.IsCalculable()) {
        ThrowFakeluaException(std::format("operand of '>=' must be number, got {} {}, {} {}", VarTypeToString(Type()), ToString(),
                                          VarTypeToString(rhs.Type()), rhs.ToString()));
    }

    if (IsCalculableInteger() && rhs.IsCalculableInteger()) {
        result.SetBool(GetCalculableInt() >= rhs.GetCalculableInt());
    } else {
        result.SetBool(GetCalculableNumber() >= rhs.GetCalculableNumber());
    }
}

void Var::Equal(const Var &rhs, Var &result) const {
    result.SetBool(Equal(rhs));
}

void Var::NotEqual(const Var &rhs, Var &result) const {
    result.SetBool(!Equal(rhs));
}

bool Var::TestTrue() const {
    switch (Type()) {
        case VarType::Nil:
            return false;
        case VarType::Bool:
            return GetBool();
        default:
            return true;
    }
}

void Var::UnopMinus(Var &result) const {
    if (!IsCalculable()) {
        ThrowFakeluaException(std::format("operand of '-' must be number, got {} {}", VarTypeToString(Type()), ToString()));
    }
    if (IsCalculableInteger()) {
        result.SetInt(-GetCalculableInt());
    } else {
        result.SetFloat(-GetCalculableNumber());
    }
}

void Var::UnopNot(Var &result) const {
    result.SetBool(!TestTrue());
}

void Var::UnopNumberSign(Var &result) const {
    if (Type() != VarType::String && Type() != VarType::Table) {
        ThrowFakeluaException(std::format("operand of '#' must be string or table, got {} {}", VarTypeToString(Type()), ToString()));
    }

    if (Type() == VarType::String) {
        result.SetInt(static_cast<int64_t>(data_.s->Size()));
    } else {
        result.SetInt(static_cast<int64_t>(data_.t->Size()));
    }
}

void Var::UnopBitnot(Var &result) const {
    if (Type() != VarType::Int) {
        ThrowFakeluaException(std::format("operand of '~' must be integer, got {} {}", VarTypeToString(Type()), ToString()));
    }

    result.SetInt(~GetInt());
}

void Var::TableSet(const Var &key, const Var &val, bool can_be_nil) const {
    if (Type() != VarType::Table) {
        ThrowFakeluaException(std::format("operand of 'TableSet' must be table, got {} {}", VarTypeToString(Type()), ToString()));
    }

    GetTable()->Set(key, val, can_be_nil);
}

Var Var::TableGet(const Var &key) const {
    if (Type() != VarType::Table) {
        ThrowFakeluaException(std::format("operand of 'TableGet' must be table, got {} {}", VarTypeToString(Type()), ToString()));
    }

    return GetTable()->Get(key);
}

size_t Var::TableSize() const {
    if (Type() != VarType::Table) {
        ThrowFakeluaException(std::format("operand of 'TableSize' must be table, got {} {}", VarTypeToString(Type()), ToString()));
    }

    return GetTable()->Size();
}

Var Var::TableKeyAt(size_t pos) const {
    DEBUG_ASSERT(Type() == VarType::Table);
    DEBUG_ASSERT(pos < GetTable()->Size());
    return GetTable()->KeyAt(pos);
}

Var Var::TableValueAt(size_t pos) const {
    DEBUG_ASSERT(Type() == VarType::Table);
    DEBUG_ASSERT(pos < GetTable()->Size());
    return GetTable()->ValueAt(pos);
}

}// namespace fakelua
