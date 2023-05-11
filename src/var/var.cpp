#include "var.h"
#include "fakelua.h"
#include "state/state.h"
#include "state/var_string_heap.h"
#include "util/common.h"

namespace fakelua {

var const_null_var;

var::var(state *s, const std::string &val) {
    auto &string_heap = s->get_var_string_heap();
    data_ = string_heap.alloc(val);
}

var::var(state *s, std::string &&val) {
    auto &string_heap = s->get_var_string_heap();
    data_ = string_heap.alloc(std::move(val));
}

var::var(state *s, const char *val) {
    auto &string_heap = s->get_var_string_heap();
    data_ = string_heap.alloc(val);
}

var::var(state *s, std::string_view val) {
    auto &string_heap = s->get_var_string_heap();
    data_ = string_heap.alloc(std::string(val));
}

std::string_view var::get_string_view(state *s) const {
    auto &string_heap = s->get_var_string_heap();
    auto str = std::get<var_string>(data_);
    return string_heap.get(str);
}

var &var::set(state *s, const std::string &val) {
    auto &string_heap = s->get_var_string_heap();
    data_ = string_heap.alloc(val);
    return *this;
}

var &var::set(state *s, std::string &&val) {
    auto &string_heap = s->get_var_string_heap();
    data_ = string_heap.alloc(std::move(val));
    return *this;
}

var &var::set(state *s, const char *val) {
    auto &string_heap = s->get_var_string_heap();
    data_ = string_heap.alloc(val);
    return *this;
}

var &var::set(state *s, std::string_view val) {
    auto &string_heap = s->get_var_string_heap();
    data_ = string_heap.alloc(std::string(val));
    return *this;
}

var &var::set(const var_table &val) {
    data_ = val;
    return *this;
}

var &var::set(var_table &&val) {
    data_ = std::move(val);
    return *this;
}

}// namespace fakelua
