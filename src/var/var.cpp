#include "var.h"
#include "fakelua.h"
#include "state/const_string.h"
#include "state/state.h"
#include "util/common.h"
#include "var_string.h"
#include "var_table.h"
#include <limits>

namespace fakelua {

void Var::CheckCalculable(const Var &rhs, const char *op) const {
    if (!IsCalculable() || !rhs.IsCalculable()) {
        ThrowFakeluaException(std::format("Var op failed, operand of '{}' must be number, got {} {}, {} {}", op, VarTypeToString(Type()),
                                          ToString(), VarTypeToString(rhs.Type()), rhs.ToString()));
    }
}

void Var::CheckCalculable(const char *op) const {
    if (!IsCalculable()) {
        ThrowFakeluaException(std::format("Var op failed, operand of '{}' must be number, got {} {}", op, VarTypeToString(Type()),
                                          ToString()));
    }
}

void Var::CheckInteger(const Var &rhs, const char *op, int64_t &lhs_int, int64_t &rhs_int) const {
    if (!TryConvertNumberToInteger(lhs_int) || !rhs.TryConvertNumberToInteger(rhs_int)) {
        ThrowFakeluaException(std::format("Var op failed, operand of '{}' must be integer, got {} {}, {} {}", op, VarTypeToString(Type()),
                                          ToString(), VarTypeToString(rhs.Type()), rhs.ToString()));
    }
}

void Var::CheckInteger(const char *op, int64_t &val) const {
    if (!TryConvertNumberToInteger(val)) {
        ThrowFakeluaException(std::format("Var op failed, operand of '{}' must be integer, got {} {}", op, VarTypeToString(Type()),
                                          ToString()));
    }
}

Var const_null_var;

bool Var::TryConvertNumberToInteger(int64_t &out) const {
    if (Type() == VarType::Int) {
        out = GetInt();
        return true;
    }
    if (Type() != VarType::Float) {
        return false;
    }

    const double double_val = GetFloat();
    if (!std::isfinite(double_val)) {
        return false;
    }
    double int_part = 0;
    if (std::modf(double_val, &int_part) != 0.0) {
        return false;
    }
    // 上界必须按“< 2^63”检查，而不能用 static_cast<double>(INT64_MAX)。
    // 因为 double 无法精确表示 INT64_MAX，转换后会变成 2^63，若仍按 INT64_MAX 比较，
    // 会把 2^63 误判为可转换值，随后 static_cast<int64_t>(2^63) 触发 UB。
    constexpr double kInt64UpperBoundExclusive = 9223372036854775808.0;// 2^63
    if (int_part < static_cast<double>(std::numeric_limits<int64_t>::min()) || int_part >= kInt64UpperBoundExclusive) {
        return false;
    }
    out = static_cast<int64_t>(int_part);
    return true;
}

VarString *Var::GetString() const {
    DEBUG_ASSERT(type_ == static_cast<int>(VarType::String) || type_ == static_cast<int>(VarType::StringId));
    if (type_ == static_cast<int>(VarType::String)) {
        return data_.s;
    } else /*if (type_ == static_cast<int>(VarType::StringId))*/ {
        return ConstString::GetVarString(data_.i);
    }
}

void Var::SetTempString(State *state, const std::string_view &val) {
    type_ = static_cast<int>(VarType::String);
    data_.s = VarString::AllocTemp(state, val);
}

void Var::SetConstString(State *state, const std::string_view &val) {
    type_ = static_cast<int>(VarType::StringId);
    data_.i = state->GetConstString().Alloc(val);
}

void Var::SetTable(State *state) {
    type_ = static_cast<int>(VarType::Table);
    data_.t = state->GetHeap().GetTempAllocator().New<VarTable>();
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
        case VarType::StringId:
            ret = has_quote ? std::format("\"{}\"", GetString()->Str()) : std::string(GetString()->Str());
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
            } bits;
            bits.d = data_.f;
            return static_cast<uint32_t>(bits.u ^ (bits.u >> 32));
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
        // Int and Float with the same mathematical value are equal (Lua semantics).
        if (Type() == VarType::Int && rhs.Type() == VarType::Float) {
            return static_cast<double>(data_.i) == rhs.data_.f;
        }
        if (Type() == VarType::Float && rhs.Type() == VarType::Int) {
            return data_.f == static_cast<double>(rhs.data_.i);
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
    ArithBinop(rhs, result, "+", std::plus<int64_t>{}, std::plus<double>{});
}

void Var::Minus(const Var &rhs, Var &result) const {
    ArithBinop(rhs, result, "-", std::minus<int64_t>{}, std::minus<double>{});
}

void Var::Star(const Var &rhs, Var &result) const {
    ArithBinop(rhs, result, "*", std::multiplies<int64_t>{}, std::multiplies<double>{});
}

void Var::Slash(const Var &rhs, Var &result) const {
    CheckCalculable(rhs, "/");
    result.SetFloat(GetCalculableNumber() / rhs.GetCalculableNumber());
}

int64_t Var::FloorDiv(int64_t lhs_val, int64_t rhs_val) const {
    int64_t quotient = lhs_val / rhs_val;
    if ((lhs_val ^ rhs_val) < 0 && lhs_val % rhs_val != 0) {
        quotient -= 1;
    }
    return quotient;
}

void Var::DoubleSlash(const Var &rhs, Var &result) const {
    CheckCalculable(rhs, "//");
    if (IsCalculableInteger() && rhs.IsCalculableInteger()) {
        if (rhs.GetCalculableInt() == 0) {
            ThrowFakeluaException("Var op failed, division by zero in '//'");
        }
        result.SetInt(FloorDiv(GetCalculableInt(), rhs.GetCalculableInt()));
    } else {
        result.SetFloat(std::floor(GetCalculableNumber() / rhs.GetCalculableNumber()));
    }
}

void Var::Pow(const Var &rhs, Var &result) const {
    CheckCalculable(rhs, "^");
    result.SetFloat(std::pow(GetCalculableNumber(), rhs.GetCalculableNumber()));
}

void Var::Mod(const Var &rhs, Var &result) const {
    CheckCalculable(rhs, "%");
    if (IsCalculableInteger() && rhs.IsCalculableInteger()) {
        if (rhs.GetCalculableInt() == 0) {
            ThrowFakeluaException("Var op failed, division by zero in '%'");
        }
        int64_t lhs_val = GetCalculableInt();
        int64_t rhs_val = rhs.GetCalculableInt();
        result.SetInt(lhs_val - rhs_val * FloorDiv(lhs_val, rhs_val));
    } else {
        double lhs_val = GetCalculableNumber();
        double rhs_val = rhs.GetCalculableNumber();
        result.SetFloat(lhs_val - rhs_val * std::floor(lhs_val / rhs_val));
    }
}

void Var::Bitand(const Var &rhs, Var &result) const {
    BitBinop(rhs, result, "&", std::bit_and<int64_t>{});
}

void Var::Xor(const Var &rhs, Var &result) const {
    BitBinop(rhs, result, "~", std::bit_xor<int64_t>{});
}

void Var::Bitor(const Var &rhs, Var &result) const {
    BitBinop(rhs, result, "|", std::bit_or<int64_t>{});
}

void Var::Shift(const Var &rhs, Var &result, const char *op_str, bool right) const {
    int64_t lhs_int = 0, rhs_int = 0;
    CheckInteger(rhs, op_str, lhs_int, rhs_int);
    if (rhs_int >= 64 || rhs_int <= -64) {
        result.SetInt(0);
        return;
    }
    const uint64_t u_lhs = static_cast<uint64_t>(lhs_int);
    if (rhs_int >= 0) {
        result.SetInt(static_cast<int64_t>(right ? (u_lhs >> rhs_int) : (u_lhs << rhs_int)));
    } else {
        result.SetInt(static_cast<int64_t>(right ? (u_lhs << -rhs_int) : (u_lhs >> -rhs_int)));
    }
}

void Var::RightShift(const Var &rhs, Var &result) const { Shift(rhs, result, ">>", true); }
void Var::LeftShift(const Var &rhs, Var &result) const { Shift(rhs, result, "<<", false); }

void Var::Concat(State *state, const Var &rhs, Var &result) const {
    // fakelua 有意扩展了 Lua 标准：允许任意类型通过 .. 运算符拼接。
    // nil/bool/int/float/string/table 均会被 ToString 转换为字符串后拼接，不抛出错误。
    // 标准 Lua 5.4 对非字符串/数字操作数会报错，但 fakelua 选择更宽松的语义以方便使用。
    result.SetTempString(state, std::format("{}{}", ToString(false, false), rhs.ToString(false, false)));
}

void Var::Less(const Var &rhs, Var &result) const {
    CompBinop(rhs, result, "<", std::less<int64_t>{}, std::less<double>{});
}

void Var::LessEqual(const Var &rhs, Var &result) const {
    CompBinop(rhs, result, "<=", std::less_equal<int64_t>{}, std::less_equal<double>{});
}

void Var::More(const Var &rhs, Var &result) const {
    CompBinop(rhs, result, ">", std::greater<int64_t>{}, std::greater<double>{});
}

void Var::MoreEqual(const Var &rhs, Var &result) const {
    CompBinop(rhs, result, ">=", std::greater_equal<int64_t>{}, std::greater_equal<double>{});
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
    CheckCalculable("-");
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
    if (Type() == VarType::String || Type() == VarType::StringId) {
        result.SetInt(static_cast<int64_t>(GetString()->Size()));
    } else if (Type() == VarType::Table) {
        result.SetInt(static_cast<int64_t>(data_.t->Size()));
    } else {
        ThrowFakeluaException(
                std::format("Var op failed, operand of '#' must be string or table, got {} {}", VarTypeToString(Type()), ToString()));
    }
}

void Var::UnopBitnot(Var &result) const {
    int64_t int_value = 0;
    CheckInteger("~", int_value);
    result.SetInt(~int_value);
}

void Var::CheckTable(const char *op) const {
    if (Type() != VarType::Table) {
        ThrowFakeluaException(std::format("Var op failed, operand of '{}' must be table, got {} {}", op, VarTypeToString(Type()), ToString()));
    }
}

void Var::TableSet(State *state, const Var &key, const Var &val, bool can_be_nil) const {
    CheckTable("TableSet");
    GetTable()->Set(state, key, val, can_be_nil);
}

Var Var::TableGet(const Var &key) const {
    CheckTable("TableGet");
    return GetTable()->Get(key);
}

size_t Var::TableSize() const {
    CheckTable("TableSize");
    return GetTable()->Size();
}

}// namespace fakelua
