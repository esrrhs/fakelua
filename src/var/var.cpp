#include "var.h"
#include "fakelua.h"
#include "state/state.h"
#include "state/var_string_heap.h"
#include "util/common.h"

namespace fakelua {

var const_null_var;
var const_false_var(false);
var const_true_var(true);

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

std::string var::to_string() const {
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
        case var_type::VAR_FLOAT:
            ret = std::to_string(get_float());
            break;
        case var_type::VAR_STRING:
            ret = std::format("\"{}\"", string_);
            break;
        case var_type::VAR_TABLE:
            ret = std::format("table({})", (void *) this);
            break;
    }

    if (is_const()) {
        ret += "(const)";
    }

    if (is_variadic()) {
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
    return type() == var_type::VAR_INT || type() == var_type::VAR_FLOAT;
}

void var::plus(const var &rhs, var &result) const {
    if (!is_calculable() || !rhs.is_calculable()) {
        throw_fakelua_exception(std::format("operand of '+' must be number, got {} {}, {} {}", magic_enum::enum_name(type()), to_string(),
                                            magic_enum::enum_name(rhs.type()), rhs.to_string()));
    }
    if (type() == var_type::VAR_INT && rhs.type() == var_type::VAR_INT) {
        result.set_int(get_int() + rhs.get_int());
    } else {
        result.set_float(get_number() + rhs.get_number());
    }
}

void var::minus(const var &rhs, var &result) const {
    if (!is_calculable() || !rhs.is_calculable()) {
        throw_fakelua_exception(std::format("operand of '-' must be number, got {} {}, {} {}", magic_enum::enum_name(type()), to_string(),
                                            magic_enum::enum_name(rhs.type()), rhs.to_string()));
    }
    if (type() == var_type::VAR_INT && rhs.type() == var_type::VAR_INT) {
        result.set_int(get_int() - rhs.get_int());
    } else {
        result.set_float(get_number() - rhs.get_number());
    }
}

void var::star(const var &rhs, var &result) const {
    if (!is_calculable() || !rhs.is_calculable()) {
        throw_fakelua_exception(std::format("operand of '*' must be number, got {} {}, {} {}", magic_enum::enum_name(type()), to_string(),
                                            magic_enum::enum_name(rhs.type()), rhs.to_string()));
    }
    if (type() == var_type::VAR_INT && rhs.type() == var_type::VAR_INT) {
        result.set_int(get_int() * rhs.get_int());
    } else {
        result.set_float(get_number() * rhs.get_number());
    }
}

void var::slash(const var &rhs, var &result) const {
    if (!is_calculable() || !rhs.is_calculable()) {
        throw_fakelua_exception(std::format("operand of '/' must be number, got {} {}, {} {}", magic_enum::enum_name(type()), to_string(),
                                            magic_enum::enum_name(rhs.type()), rhs.to_string()));
    }
    result.set_float(get_number() / rhs.get_number());
}

void var::double_slash(const var &rhs, var &result) const {
    if (!is_calculable() || !rhs.is_calculable()) {
        throw_fakelua_exception(std::format("operand of '//' must be number, got {} {}, {} {}", magic_enum::enum_name(type()), to_string(),
                                            magic_enum::enum_name(rhs.type()), rhs.to_string()));
    }
    if (type() == var_type::VAR_INT && rhs.type() == var_type::VAR_INT) {
        result.set_int(get_number() / rhs.get_number());
    } else {
        result.set_float(std::floor(get_number() / rhs.get_number()));
    }
}

void var::xor_(const var &rhs, var &result) const {
    if (!is_calculable() || !rhs.is_calculable()) {
        throw_fakelua_exception(std::format("operand of '^' must be number, got {} {}, {} {}", magic_enum::enum_name(type()), to_string(),
                                            magic_enum::enum_name(rhs.type()), rhs.to_string()));
    }
    if (type() == var_type::VAR_INT && rhs.type() == var_type::VAR_INT) {
        result.set_int(get_int() ^ rhs.get_int());
    } else {
        throw_fakelua_exception(std::format("operand of '^' must be integer, got {} {}, {} {}", magic_enum::enum_name(type()), to_string(),
                                            magic_enum::enum_name(rhs.type()), rhs.to_string()));
    }
}

void var::mod(const var &rhs, var &result) const {
}

void var::bitand_(const var &rhs, var &result) const {
}

void var::bitnot(const var &rhs, var &result) const {
}

void var::bitor_(const var &rhs, var &result) const {
}

void var::right_shift(const var &rhs, var &result) const {
}

void var::left_shift(const var &rhs, var &result) const {
}

void var::concat(const var &rhs, var &result) const {
}
void var::less(const var &rhs, var &result) const {
}

void var::less_equal(const var &rhs, var &result) const {
}

void var::more(const var &rhs, var &result) const {
}

void var::more_equal(const var &rhs, var &result) const {
}

void var::equal(const var &rhs, var &result) const {
}

void var::not_equal(const var &rhs, var &result) const {
}

}// namespace fakelua
