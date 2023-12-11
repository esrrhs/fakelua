#include "var.h"
#include "fakelua.h"
#include "state/state.h"
#include "state/var_string_heap.h"
#include "util/common.h"

namespace fakelua {

var const_null_var;

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
    switch (type()) {
        case var_type::VAR_NIL:
            return "nil";
        case var_type::VAR_BOOL:
            return get_bool() ? "true" : "false";
        case var_type::VAR_INT:
            return std::to_string(get_int());
        case var_type::VAR_FLOAT:
            return std::to_string(get_float());
        case var_type::VAR_STRING:
            return std::format("\"{}\"", string_);
        case var_type::VAR_TABLE:
            return std::format("table({})", (void *) this);
        default:
            return "unknown";
    }
}

}// namespace fakelua
