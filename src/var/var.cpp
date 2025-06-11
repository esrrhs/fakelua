#include "var.h"
#include "fakelua.h"
#include "state/state.h"
#include "state/var_string_heap.h"
#include "util/common.h"

namespace fakelua {

var const_null_var;

void var::set_string(const fakelua_state_ptr &s, const std::string_view &val) {
    set_string(s.get(), val);
}

void var::set_string(fakelua_state *s, const std::string_view &val) {
    type_ = var_type::VAR_STRING;
    auto &string_heap = dynamic_cast<state *>(s)->get_var_string_heap();
    data_.s = string_heap.alloc(val, is_const());
}

void var::set_table(const fakelua_state_ptr &s) {
    set_table(s.get());
}

void var::set_table(fakelua_state *s) {
    type_ = var_type::VAR_TABLE;
    auto &table_heap = dynamic_cast<state *>(s)->get_var_table_heap();
    data_.t = table_heap.alloc(is_const());
}

std::string var::to_string(bool has_quote, bool has_postfix) const {
    std::string ret;
    DEBUG_ASSERT(type() >= var_type::VAR_MIN && type() <= var_type::VAR_MAX);
    switch (type()) {
        case var_type::VAR_NIL:
            ret = "nil";
            break;
        case var_type::VAR_BOOL:
            ret = get_bool() ? "true" : "false";
            break;
        case var_type::VAR_INT:
            ret = std::to_string(data_.i);
            break;
        case var_type::VAR_FLOAT: {
            ret = std::format("{}", data_.f);
            break;
        }
        case var_type::VAR_STRING:
            ret = has_quote ? std::format("\"{}\"", data_.s->str()) : std::format("{}", data_.s->str());
            break;
        case var_type::VAR_TABLE:
            ret = std::format("table({})", static_cast<void *>(data_.t));
            break;
    }

    if (has_postfix && is_const()) {
        ret += "(const)";
    }

    if (has_postfix && is_variadic()) {
        ret += "(variadic)";
    }

    return ret;
}

size_t var::hash() const {
    switch (type()) {
        case var_type::VAR_NIL:
            return 0;
        case var_type::VAR_BOOL:
            return get_bool() ? 1 : 0;
        case var_type::VAR_INT:
            return std::hash<int64_t>()(data_.i);
        case var_type::VAR_FLOAT:
            return std::hash<double>()(data_.f);
        case var_type::VAR_STRING:
            return std::hash<std::string_view>()(data_.s->str());
        case var_type::VAR_TABLE:
            return std::hash<size_t>()(reinterpret_cast<size_t>(data_.t));
        default:
            return 0;
    }
}

bool var::equal(const var &rhs) const {
    if (type() != rhs.type()) {
        return false;
    }

    switch (type()) {
        case var_type::VAR_NIL:
            return true;
        case var_type::VAR_BOOL:
            return data_.b == rhs.data_.b;
        case var_type::VAR_INT:
            return data_.i == rhs.data_.i;
        case var_type::VAR_FLOAT:
            if (std::isnan(data_.f) && std::isnan(rhs.data_.f)) {
                return true;
            }
            return data_.f == rhs.data_.f;
        case var_type::VAR_STRING:
            return data_.s == rhs.data_.s;
        case var_type::VAR_TABLE:
            return data_.t == rhs.data_.t;
        default:
            return false;
    }
}

bool var::is_calculable() const {
    return type() == var_type::VAR_INT || type() == var_type::VAR_FLOAT || (type() == var_type::VAR_STRING && is_number(data_.s->str()));
}

bool var::is_calculable_integer() const {
    return type() == var_type::VAR_INT || (type() == var_type::VAR_STRING && is_integer(data_.s->str()));
}

int64_t var::get_calculable_int() const {
    if (type() == var_type::VAR_INT) {
        return get_int();
    } else {// if (type() == var_type::VAR_STRING)
        DEBUG_ASSERT(type() == var_type::VAR_STRING);
        DEBUG_ASSERT(is_integer(data_.s->str()));
        return to_integer(data_.s->str());
    }
}

double var::get_calculable_number() const {
    if (type_ == var_type::VAR_INT) {
        return (double) data_.i;
    } else if (type_ == var_type::VAR_FLOAT) {
        return data_.f;
    } else {// if (type_ == var_type::VAR_STRING)
        DEBUG_ASSERT(type_ == var_type::VAR_STRING);
        DEBUG_ASSERT(is_number(data_.s->str()));
        return to_float(data_.s->str());
    }
}

void var::plus(const var &rhs, var &result) const {
    if (!is_calculable() || !rhs.is_calculable()) {
        throw_fakelua_exception(std::format("operand of '+' must be number, got {} {}, {} {}", magic_enum::enum_name(type()), to_string(),
                                            magic_enum::enum_name(rhs.type()), rhs.to_string()));
    }
    if (is_calculable_integer() && rhs.is_calculable_integer()) {
        result.set_int(get_calculable_int() + rhs.get_calculable_int());
    } else {
        result.set_float(get_calculable_number() + rhs.get_calculable_number());
    }
}

void var::minus(const var &rhs, var &result) const {
    if (!is_calculable() || !rhs.is_calculable()) {
        throw_fakelua_exception(std::format("operand of '-' must be number, got {} {}, {} {}", magic_enum::enum_name(type()), to_string(),
                                            magic_enum::enum_name(rhs.type()), rhs.to_string()));
    }
    if (is_calculable_integer() && rhs.is_calculable_integer()) {
        result.set_int(get_calculable_int() - rhs.get_calculable_int());
    } else {
        result.set_float(get_calculable_number() - rhs.get_calculable_number());
    }
}

void var::star(const var &rhs, var &result) const {
    if (!is_calculable() || !rhs.is_calculable()) {
        throw_fakelua_exception(std::format("operand of '*' must be number, got {} {}, {} {}", magic_enum::enum_name(type()), to_string(),
                                            magic_enum::enum_name(rhs.type()), rhs.to_string()));
    }
    if (is_calculable_integer() && rhs.is_calculable_integer()) {
        result.set_int(get_calculable_int() * rhs.get_calculable_int());
    } else {
        result.set_float(get_calculable_number() * rhs.get_calculable_number());
    }
}

void var::slash(const var &rhs, var &result) const {
    if (!is_calculable() || !rhs.is_calculable()) {
        throw_fakelua_exception(std::format("operand of '/' must be number, got {} {}, {} {}", magic_enum::enum_name(type()), to_string(),
                                            magic_enum::enum_name(rhs.type()), rhs.to_string()));
    }
    result.set_float(get_calculable_number() / rhs.get_calculable_number());
}

void var::double_slash(const var &rhs, var &result) const {
    if (!is_calculable() || !rhs.is_calculable()) {
        throw_fakelua_exception(std::format("operand of '//' must be number, got {} {}, {} {}", magic_enum::enum_name(type()), to_string(),
                                            magic_enum::enum_name(rhs.type()), rhs.to_string()));
    }
    if (is_calculable_integer() && rhs.is_calculable_integer()) {
        result.set_int(get_calculable_int() / rhs.get_calculable_int());
    } else {
        result.set_float(std::floor(get_calculable_number() / rhs.get_calculable_number()));
    }
}

void var::pow(const var &rhs, var &result) const {
    if (!is_calculable() || !rhs.is_calculable()) {
        throw_fakelua_exception(std::format("operand of '^' must be number, got {} {}, {} {}", magic_enum::enum_name(type()), to_string(),
                                            magic_enum::enum_name(rhs.type()), rhs.to_string()));
    }
    result.set_float(std::pow(get_calculable_number(), rhs.get_calculable_number()));
}

void var::mod(const var &rhs, var &result) const {
    if (!is_calculable() || !rhs.is_calculable()) {
        throw_fakelua_exception(std::format("operand of '%' must be number, got {} {}, {} {}", magic_enum::enum_name(type()), to_string(),
                                            magic_enum::enum_name(rhs.type()), rhs.to_string()));
    }
    if (is_calculable_integer() && rhs.is_calculable_integer()) {
        result.set_int(get_calculable_int() % rhs.get_calculable_int());
    } else {
        result.set_float(std::fmod(get_calculable_number(), rhs.get_calculable_number()));
    }
}

void var::bitand_(const var &rhs, var &result) const {
    if (!is_calculable_integer() || !rhs.is_calculable_integer()) {
        throw_fakelua_exception(std::format("operand of '&' must be integer, got {} {}, {} {}", magic_enum::enum_name(type()), to_string(),
                                            magic_enum::enum_name(rhs.type()), rhs.to_string()));
    }
    result.set_int(get_calculable_int() & rhs.get_calculable_int());
}

void var::xor_(const var &rhs, var &result) const {
    if (!is_calculable_integer() || !rhs.is_calculable_integer()) {
        throw_fakelua_exception(std::format("operand of '~' must be integer, got {} {}, {} {}", magic_enum::enum_name(type()), to_string(),
                                            magic_enum::enum_name(rhs.type()), rhs.to_string()));
    }
    result.set_int(get_calculable_int() ^ rhs.get_calculable_int());
}

void var::bitor_(const var &rhs, var &result) const {
    if (!is_calculable_integer() || !rhs.is_calculable_integer()) {
        throw_fakelua_exception(std::format("operand of '|' must be integer, got {} {}, {} {}", magic_enum::enum_name(type()), to_string(),
                                            magic_enum::enum_name(rhs.type()), rhs.to_string()));
    }
    result.set_int(get_calculable_int() | rhs.get_calculable_int());
}

void var::right_shift(const var &rhs, var &result) const {
    if (!is_calculable_integer() || !rhs.is_calculable_integer()) {
        throw_fakelua_exception(std::format("operand of '>>' must be integer, got {} {}, {} {}", magic_enum::enum_name(type()), to_string(),
                                            magic_enum::enum_name(rhs.type()), rhs.to_string()));
    }
    auto shift = rhs.get_calculable_int();
    if (shift >= 0) {
        result.set_int(static_cast<int64_t>(static_cast<uint64_t>(get_calculable_int()) >> shift));
    } else {
        result.set_int(static_cast<int64_t>(static_cast<uint64_t>(get_calculable_int()) << (-shift)));
    }
}

void var::left_shift(const var &rhs, var &result) const {
    if (!is_calculable_integer() || !rhs.is_calculable_integer()) {
        throw_fakelua_exception(std::format("operand of '<<' must be integer, got {} {}, {} {}", magic_enum::enum_name(type()), to_string(),
                                            magic_enum::enum_name(rhs.type()), rhs.to_string()));
    }
    auto shift = rhs.get_calculable_int();
    if (shift >= 0) {
        result.set_int(static_cast<int64_t>(static_cast<uint64_t>(get_calculable_int()) << shift));
    } else {
        result.set_int(static_cast<int64_t>(static_cast<uint64_t>(get_calculable_int()) >> (-shift)));
    }
}

void var::concat(fakelua_state *s, const var &rhs, var &result) const {
    result.set_string(s, std::format("{}{}", to_string(false, false), rhs.to_string(false, false)));
}

void var::less(const var &rhs, var &result) const {
    if (!is_calculable() || !rhs.is_calculable()) {
        throw_fakelua_exception(std::format("operand of '<' must be number, got {} {}, {} {}", magic_enum::enum_name(type()), to_string(),
                                            magic_enum::enum_name(rhs.type()), rhs.to_string()));
    }

    if (is_calculable_integer() && rhs.is_calculable_integer()) {
        result.set_bool(get_calculable_int() < rhs.get_calculable_int());
    } else {
        result.set_bool(get_calculable_number() < rhs.get_calculable_number());
    }
}

void var::less_equal(const var &rhs, var &result) const {
    if (!is_calculable() || !rhs.is_calculable()) {
        throw_fakelua_exception(std::format("operand of '<=' must be number, got {} {}, {} {}", magic_enum::enum_name(type()), to_string(),
                                            magic_enum::enum_name(rhs.type()), rhs.to_string()));
    }

    if (is_calculable_integer() && rhs.is_calculable_integer()) {
        result.set_bool(get_calculable_int() <= rhs.get_calculable_int());
    } else {
        result.set_bool(get_calculable_number() <= rhs.get_calculable_number());
    }
}

void var::more(const var &rhs, var &result) const {
    if (!is_calculable() || !rhs.is_calculable()) {
        throw_fakelua_exception(std::format("operand of '>' must be number, got {} {}, {} {}", magic_enum::enum_name(type()), to_string(),
                                            magic_enum::enum_name(rhs.type()), rhs.to_string()));
    }

    if (is_calculable_integer() && rhs.is_calculable_integer()) {
        result.set_bool(get_calculable_int() > rhs.get_calculable_int());
    } else {
        result.set_bool(get_calculable_number() > rhs.get_calculable_number());
    }
}

void var::more_equal(const var &rhs, var &result) const {
    if (!is_calculable() || !rhs.is_calculable()) {
        throw_fakelua_exception(std::format("operand of '>=' must be number, got {} {}, {} {}", magic_enum::enum_name(type()), to_string(),
                                            magic_enum::enum_name(rhs.type()), rhs.to_string()));
    }

    if (is_calculable_integer() && rhs.is_calculable_integer()) {
        result.set_bool(get_calculable_int() >= rhs.get_calculable_int());
    } else {
        result.set_bool(get_calculable_number() >= rhs.get_calculable_number());
    }
}

void var::equal(const var &rhs, var &result) const {
    result.set_bool(equal(rhs));
}

void var::not_equal(const var &rhs, var &result) const {
    result.set_bool(!equal(rhs));
}

bool var::test_true() const {
    switch (type()) {
        case var_type::VAR_NIL:
            return false;
        case var_type::VAR_BOOL:
            return get_bool();
        default:
            return true;
    }
}

void var::unop_minus(var &result) const {
    if (!is_calculable()) {
        throw_fakelua_exception(std::format("operand of '-' must be number, got {} {}", magic_enum::enum_name(type()), to_string()));
    }
    if (is_calculable_integer()) {
        result.set_int(-get_calculable_int());
    } else {
        result.set_float(-get_calculable_number());
    }
}

void var::unop_not(var &result) const {
    result.set_bool(!test_true());
}

void var::unop_number_sign(var &result) const {
    if (type() != var_type::VAR_STRING && type() != var_type::VAR_TABLE) {
        throw_fakelua_exception(
                std::format("operand of '#' must be string or table, got {} {}", magic_enum::enum_name(type()), to_string()));
    }

    if (type() == var_type::VAR_STRING) {
        result.set_int(static_cast<int64_t>(data_.s->size()));
    } else {
        result.set_int(static_cast<int64_t>(data_.t->size()));
    }
}

void var::unop_bitnot(var &result) const {
    if (type() != var_type::VAR_INT) {
        throw_fakelua_exception(std::format("operand of '~' must be integer, got {} {}", magic_enum::enum_name(type()), to_string()));
    }

    result.set_int(~get_int());
}

void var::table_set(const var &key, const var &val, bool can_be_nil) {
    if (type() != var_type::VAR_TABLE) {
        throw_fakelua_exception(std::format("operand of 'table_set' must be table, got {} {}", magic_enum::enum_name(type()), to_string()));
    }

    get_table()->set(key, val, can_be_nil);
}

const var *var::table_get(const var &key) const {
    if (type() != var_type::VAR_TABLE) {
        throw_fakelua_exception(std::format("operand of 'table_get' must be table, got {} {}", magic_enum::enum_name(type()), to_string()));
    }

    return get_table()->get(key);
}

size_t var::table_size() const {
    if (type() != var_type::VAR_TABLE) {
        throw_fakelua_exception(
                std::format("operand of 'table_size' must be table, got {} {}", magic_enum::enum_name(type()), to_string()));
    }

    return get_table()->size();
}

const var *var::table_key_at(size_t pos) const {
    DEBUG_ASSERT(type() == var_type::VAR_TABLE);
    DEBUG_ASSERT(pos < get_table()->size());
    return get_table()->key_at(pos);
}

const var *var::table_value_at(size_t pos) const {
    DEBUG_ASSERT(type() == var_type::VAR_TABLE);
    DEBUG_ASSERT(pos < get_table()->size());
    return get_table()->value_at(pos);
}

}// namespace fakelua
