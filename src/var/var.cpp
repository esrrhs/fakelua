#include "var.h"
#include "fakelua.h"
#include "state/state.h"
#include "state/var_string_heap.h"
#include "util/common.h"

namespace fakelua {

var const_null_var(nullptr, true);
var const_false_var(false, true);
var const_true_var(true, true);

var::var(const fakelua_state_ptr &s, const std::string &val) : type_(var_type::VAR_STRING) {
    auto &string_heap = std::dynamic_pointer_cast<state>(s)->get_var_string_heap();
    string_ = string_heap.alloc(val);
}

var::var(const fakelua_state_ptr &s, std::string &&val) : type_(var_type::VAR_STRING) {
    auto &string_heap = std::dynamic_pointer_cast<state>(s)->get_var_string_heap();
    string_ = string_heap.alloc(std::move(val));
}

var::var(const fakelua_state_ptr &s, const char *val) : type_(var_type::VAR_STRING) {
    auto &string_heap = std::dynamic_pointer_cast<state>(s)->get_var_string_heap();
    string_ = string_heap.alloc(val);
}

var::var(const fakelua_state_ptr &s, std::string_view val) : type_(var_type::VAR_STRING) {
    auto &string_heap = std::dynamic_pointer_cast<state>(s)->get_var_string_heap();
    string_ = string_heap.alloc(std::string(val));
}

void var::set_string(const fakelua_state_ptr &s, const std::string &val) {
    type_ = var_type::VAR_STRING;
    auto &string_heap = std::dynamic_pointer_cast<state>(s)->get_var_string_heap();
    string_ = string_heap.alloc(val);
}

void var::set_string(const fakelua_state_ptr &s, std::string &&val) {
    type_ = var_type::VAR_STRING;
    auto &string_heap = std::dynamic_pointer_cast<state>(s)->get_var_string_heap();
    string_ = string_heap.alloc(std::move(val));
}

void var::set_string(const fakelua_state_ptr &s, const char *val) {
    type_ = var_type::VAR_STRING;
    auto &string_heap = std::dynamic_pointer_cast<state>(s)->get_var_string_heap();
    string_ = string_heap.alloc(val);
}

void var::set_string(const fakelua_state_ptr &s, std::string_view val) {
    type_ = var_type::VAR_STRING;
    auto &string_heap = std::dynamic_pointer_cast<state>(s)->get_var_string_heap();
    string_ = string_heap.alloc(std::string(val));
}

void var::set_string(fakelua_state *s, const std::string &val) {
    type_ = var_type::VAR_STRING;
    auto &string_heap = dynamic_cast<state *>(s)->get_var_string_heap();
    string_ = string_heap.alloc(val);
}

void var::set_string(fakelua_state *s, std::string &&val) {
    type_ = var_type::VAR_STRING;
    auto &string_heap = dynamic_cast<state *>(s)->get_var_string_heap();
    string_ = string_heap.alloc(std::move(val));
}

void var::set_string(fakelua_state *s, const char *val) {
    type_ = var_type::VAR_STRING;
    auto &string_heap = dynamic_cast<state *>(s)->get_var_string_heap();
    string_ = string_heap.alloc(val);
}

void var::set_string(fakelua_state *s, std::string_view val) {
    type_ = var_type::VAR_STRING;
    auto &string_heap = dynamic_cast<state *>(s)->get_var_string_heap();
    string_ = string_heap.alloc(std::string(val));
}

void var::set_table() {
    type_ = var_type::VAR_TABLE;
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
            ret = std::to_string(get_int());
            break;
        case var_type::VAR_FLOAT: {
            char buffer[64];
            char *end = std::to_chars(std::begin(buffer), std::end(buffer), get_float(), std::chars_format::general).ptr;
            ret = std::string(buffer, end);
            break;
        }
        case var_type::VAR_STRING:
            ret = has_quote ? std::format("\"{}\"", string_) : std::format("{}", string_);
            break;
        case var_type::VAR_TABLE:
            ret = std::format("table({})", (void *) this);
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
            return std::hash<int64_t>()(get_int());
        case var_type::VAR_FLOAT:
            return std::hash<double>()(get_float());
        case var_type::VAR_STRING:
            if (is_short_string()) {
                return std::hash<int64_t>()(reinterpret_cast<int64_t>(string_.data()));
            } else {
                return std::hash<std::string_view>()(string_);
            }
        case var_type::VAR_TABLE:
            return std::hash<size_t>()(reinterpret_cast<size_t>(this));
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
            return get_bool() == rhs.get_bool();
        case var_type::VAR_INT:
            return get_int() == rhs.get_int();
        case var_type::VAR_FLOAT:
            if (std::isnan(get_float()) && std::isnan(rhs.get_float())) {
                return true;
            }
            return get_float() == rhs.get_float();
        case var_type::VAR_STRING:
            if (is_short_string() && rhs.is_short_string()) {
                return string_.data() == rhs.string_.data();
            }
            return string_ == rhs.string_;
        case var_type::VAR_TABLE:
            return this == &rhs;
        default:
            return false;
    }
}

bool var::is_calculable() const {
    return type() == var_type::VAR_INT || type() == var_type::VAR_FLOAT || (type() == var_type::VAR_STRING && is_number(get_string()));
}

bool var::is_calculable_integer() const {
    return type() == var_type::VAR_INT || (type() == var_type::VAR_STRING && is_integer(get_string()));
}

int64_t var::get_calculable_int() const {
    if (type() == var_type::VAR_INT) {
        return get_int();
    } else {// if (type() == var_type::VAR_STRING)
        DEBUG_ASSERT(type() == var_type::VAR_STRING);
        DEBUG_ASSERT(is_integer(get_string()));
        return to_integer(get_string());
    }
}

double var::get_calculable_number() const {
    if (type_ == var_type::VAR_INT) {
        return (double) int_;
    } else if (type_ == var_type::VAR_FLOAT) {
        return float_;
    } else {// if (type_ == var_type::VAR_STRING)
        DEBUG_ASSERT(type_ == var_type::VAR_STRING);
        DEBUG_ASSERT(is_number(get_string()));
        return to_float(get_string());
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
        result.set_int(((uint64_t) get_calculable_int()) >> shift);
    } else {
        result.set_int(((uint64_t) get_calculable_int()) << (-shift));
    }
}

void var::left_shift(const var &rhs, var &result) const {
    if (!is_calculable_integer() || !rhs.is_calculable_integer()) {
        throw_fakelua_exception(std::format("operand of '<<' must be integer, got {} {}, {} {}", magic_enum::enum_name(type()), to_string(),
                                            magic_enum::enum_name(rhs.type()), rhs.to_string()));
    }
    auto shift = rhs.get_calculable_int();
    if (shift >= 0) {
        result.set_int(((uint64_t) get_calculable_int()) << shift);
    } else {
        result.set_int(((uint64_t) get_calculable_int()) >> (-shift));
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
        result.set_int(get_string().size());
    } else {
        result.set_int(get_table().size());
    }
}

void var::unop_bitnot(var &result) const {
    if (type() != var_type::VAR_INT) {
        throw_fakelua_exception(std::format("operand of '~' must be integer, got {} {}", magic_enum::enum_name(type()), to_string()));
    }

    result.set_int(~get_int());
}

void var::table_set(var *key, var *val) {
    if (type() != var_type::VAR_TABLE) {
        throw_fakelua_exception(std::format("operand of 'table_set' must be table, got {} {}", magic_enum::enum_name(type()), to_string()));
    }

    get_table().set(key, val);
}

var *var::table_get(var *key) {
    if (type() != var_type::VAR_TABLE) {
        throw_fakelua_exception(std::format("operand of 'table_get' must be table, got {} {}", magic_enum::enum_name(type()), to_string()));
    }

    return get_table().get(key);
}

}// namespace fakelua
