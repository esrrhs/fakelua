#include "var.h"
#include "fakelua.h"
#include "state/const_string.h"
#include "state/state.h"
#include "util/common.h"
#include "var_string.h"
#include "var_table.h"

namespace fakelua {

Var const_null_var;

VarString *Var::GetString() const {
    DEBUG_ASSERT(type_ == static_cast<int>(VarType::String) || type_ == static_cast<int>(VarType::StringId));
    if (type_ == static_cast<int>(VarType::String)) {
        return data_.s;
    } else /*if (type_ == static_cast<int>(VarType::StringId))*/ {
        return ConstString::GetVarString(data_.i);
    }
}

void Var::SetTempString(State *s, const std::string_view &val) {
    type_ = static_cast<int>(VarType::String);
    data_.s = VarString::AllocTemp(s, val);
}

void Var::SetConstString(State *s, const std::string_view &val) {
    type_ = static_cast<int>(VarType::StringId);
    data_.i = s->GetConstString().Alloc(val);
}

void Var::SetTable(State *s) {
    type_ = static_cast<int>(VarType::Table);
    data_.t = s->GetHeap().GetTempAllocator().New<VarTable>();
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
        case VarType::StringId:
            ret = has_quote ? std::format("\"{}\"", ConstString::GetString(data_.i)) : std::format("{}", ConstString::GetString(data_.i));
            break;
        case VarType::Table:
            ret = std::format("table({})", static_cast<void *>(data_.t));
            break;
    }

    return ret;
}

size_t Var::Hash() const {
    switch (Type()) {
        case VarType::Nil: {
            return 0;
        }
        case VarType::Bool: {
            return data_.b ? 1 : 0;
        }
        case VarType::Int: {
            return static_cast<uint32_t>(data_.i ^ (data_.i >> 32));
        }
        case VarType::Float: {
            union {
                double d;
                uint64_t u;
            } u;
            u.d = data_.f;
            return static_cast<uint32_t>(u.u ^ (u.u >> 32));
        }
        case VarType::String:
        case VarType::StringId: {
            return GetString()->Hash();
        }
        case VarType::Table: {
            return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(data_.t) ^ (reinterpret_cast<uintptr_t>(data_.t) >> 32));
        }
        default: {
            return 0;
        }
    }
}

bool Var::Equal(const Var &rhs) const {
    if (Type() != rhs.Type()) {
        if (Type() == VarType::String && rhs.Type() == VarType::StringId) {
            return GetString()->Str() == rhs.GetString()->Str();
        }
        if (Type() == VarType::StringId && rhs.Type() == VarType::String) {
            return GetString()->Str() == rhs.GetString()->Str();
        }
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
            return data_.s->Str() == rhs.data_.s->Str();
        case VarType::StringId:
            return data_.i == rhs.data_.i;
        case VarType::Table:
            return data_.t == rhs.data_.t;
        default:
            return false;
    }
}

bool Var::IsCalculable() const {
    return Type() == VarType::Int || Type() == VarType::Float;
}

bool Var::IsCalculableInteger() const {
    return Type() == VarType::Int;
}

int64_t Var::GetCalculableInt() const {
    return GetInt();
}

double Var::GetCalculableNumber() const {
    if (type_ == static_cast<int>(VarType::Int)) {
        return static_cast<double>(data_.i);
    } else /*if (type_ == static_cast<int>(VarType::Float))*/ {
        return data_.f;
    }
}

void Var::Plus(const Var &rhs, Var &result) const {
    if (!IsCalculable() || !rhs.IsCalculable()) {
        ThrowFakeluaException(std::format("Var op failed, operand of '+' must be number, got {} {}, {} {}", VarTypeToString(Type()), ToString(),
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
        ThrowFakeluaException(std::format("Var op failed, operand of '-' must be number, got {} {}, {} {}", VarTypeToString(Type()), ToString(),
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
        ThrowFakeluaException(std::format("Var op failed, operand of '*' must be number, got {} {}, {} {}", VarTypeToString(Type()), ToString(),
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
        ThrowFakeluaException(std::format("Var op failed, operand of '/' must be number, got {} {}, {} {}", VarTypeToString(Type()), ToString(),
                                          VarTypeToString(rhs.Type()), rhs.ToString()));
    }
    result.SetFloat(GetCalculableNumber() / rhs.GetCalculableNumber());
}

void Var::DoubleSlash(const Var &rhs, Var &result) const {
    if (!IsCalculable() || !rhs.IsCalculable()) {
        ThrowFakeluaException(std::format("Var op failed, operand of '//' must be number, got {} {}, {} {}", VarTypeToString(Type()), ToString(),
                                          VarTypeToString(rhs.Type()), rhs.ToString()));
    }
    if (IsCalculableInteger() && rhs.IsCalculableInteger()) {
        if (rhs.GetCalculableInt() == 0) {
            ThrowFakeluaException("Var op failed, division by zero in '//'");
        }
        result.SetInt(GetCalculableInt() / rhs.GetCalculableInt());
    } else {
        result.SetFloat(std::floor(GetCalculableNumber() / rhs.GetCalculableNumber()));
    }
}

void Var::Pow(const Var &rhs, Var &result) const {
    if (!IsCalculable() || !rhs.IsCalculable()) {
        ThrowFakeluaException(std::format("Var op failed, operand of '^' must be number, got {} {}, {} {}", VarTypeToString(Type()), ToString(),
                                          VarTypeToString(rhs.Type()), rhs.ToString()));
    }
    result.SetFloat(std::pow(GetCalculableNumber(), rhs.GetCalculableNumber()));
}

void Var::Mod(const Var &rhs, Var &result) const {
    if (!IsCalculable() || !rhs.IsCalculable()) {
        ThrowFakeluaException(std::format("Var op failed, operand of '%' must be number, got {} {}, {} {}", VarTypeToString(Type()), ToString(),
                                          VarTypeToString(rhs.Type()), rhs.ToString()));
    }
    if (IsCalculableInteger() && rhs.IsCalculableInteger()) {
        if (rhs.GetCalculableInt() == 0) {
            ThrowFakeluaException("Var op failed, division by zero in '%'");
        }
        result.SetInt(GetCalculableInt() % rhs.GetCalculableInt());
    } else {
        result.SetFloat(std::fmod(GetCalculableNumber(), rhs.GetCalculableNumber()));
    }
}

void Var::Bitand(const Var &rhs, Var &result) const {
    if (!IsCalculableInteger() || !rhs.IsCalculableInteger()) {
        ThrowFakeluaException(std::format("Var op failed, operand of '&' must be integer, got {} {}, {} {}", VarTypeToString(Type()), ToString(),
                                          VarTypeToString(rhs.Type()), rhs.ToString()));
    }
    result.SetInt(GetCalculableInt() & rhs.GetCalculableInt());
}

void Var::Xor(const Var &rhs, Var &result) const {
    if (!IsCalculableInteger() || !rhs.IsCalculableInteger()) {
        ThrowFakeluaException(std::format("Var op failed, operand of '~' must be integer, got {} {}, {} {}", VarTypeToString(Type()), ToString(),
                                          VarTypeToString(rhs.Type()), rhs.ToString()));
    }
    result.SetInt(GetCalculableInt() ^ rhs.GetCalculableInt());
}

void Var::Bitor(const Var &rhs, Var &result) const {
    if (!IsCalculableInteger() || !rhs.IsCalculableInteger()) {
        ThrowFakeluaException(std::format("Var op failed, operand of '|' must be integer, got {} {}, {} {}", VarTypeToString(Type()), ToString(),
                                          VarTypeToString(rhs.Type()), rhs.ToString()));
    }
    result.SetInt(GetCalculableInt() | rhs.GetCalculableInt());
}

void Var::RightShift(const Var &rhs, Var &result) const {
    if (!IsCalculableInteger() || !rhs.IsCalculableInteger()) {
        ThrowFakeluaException(std::format("Var op failed, operand of '>>' must be integer, got {} {}, {} {}", VarTypeToString(Type()), ToString(),
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
        ThrowFakeluaException(std::format("Var op failed, operand of '<<' must be integer, got {} {}, {} {}", VarTypeToString(Type()), ToString(),
                                          VarTypeToString(rhs.Type()), rhs.ToString()));
    }
    auto shift = rhs.GetCalculableInt();
    if (shift >= 0) {
        result.SetInt(static_cast<int64_t>(static_cast<uint64_t>(GetCalculableInt()) << shift));
    } else {
        result.SetInt(static_cast<int64_t>(static_cast<uint64_t>(GetCalculableInt()) >> (-shift)));
    }
}

void Var::Concat(State *s, const Var &rhs, Var &result) const {
    result.SetTempString(s, std::format("{}{}", ToString(false, false), rhs.ToString(false, false)));
}

void Var::Less(const Var &rhs, Var &result) const {
    if (!IsCalculable() || !rhs.IsCalculable()) {
        ThrowFakeluaException(std::format("Var op failed, operand of '<' must be number, got {} {}, {} {}", VarTypeToString(Type()), ToString(),
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
        ThrowFakeluaException(std::format("Var op failed, operand of '<=' must be number, got {} {}, {} {}", VarTypeToString(Type()), ToString(),
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
        ThrowFakeluaException(std::format("Var op failed, operand of '>' must be number, got {} {}, {} {}", VarTypeToString(Type()), ToString(),
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
        ThrowFakeluaException(std::format("Var op failed, operand of '>=' must be number, got {} {}, {} {}", VarTypeToString(Type()), ToString(),
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
        ThrowFakeluaException(std::format("Var op failed, operand of '-' must be number, got {} {}", VarTypeToString(Type()), ToString()));
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
        ThrowFakeluaException(std::format("Var op failed, operand of '#' must be string or table, got {} {}", VarTypeToString(Type()), ToString()));
    }

    if (Type() == VarType::String) {
        result.SetInt(static_cast<int64_t>(data_.s->Size()));
    } else {
        result.SetInt(static_cast<int64_t>(data_.t->Size()));
    }
}

void Var::UnopBitnot(Var &result) const {
    if (Type() != VarType::Int) {
        ThrowFakeluaException(std::format("Var op failed, operand of '~' must be integer, got {} {}", VarTypeToString(Type()), ToString()));
    }

    result.SetInt(~GetInt());
}

void Var::TableSet(State *s, const Var &key, const Var &val, bool can_be_nil) const {
    if (Type() != VarType::Table) {
        ThrowFakeluaException(std::format("Var op failed, operand of 'TableSet' must be table, got {} {}", VarTypeToString(Type()), ToString()));
    }

    GetTable()->Set(s, key, val, can_be_nil);
}

Var Var::TableGet(const Var &key) const {
    if (Type() != VarType::Table) {
        ThrowFakeluaException(std::format("Var op failed, operand of 'TableGet' must be table, got {} {}", VarTypeToString(Type()), ToString()));
    }

    return GetTable()->Get(key);
}

size_t Var::TableSize() const {
    if (Type() != VarType::Table) {
        ThrowFakeluaException(std::format("Var op failed, operand of 'TableSize' must be table, got {} {}", VarTypeToString(Type()), ToString()));
    }

    return GetTable()->Size();
}

}// namespace fakelua
