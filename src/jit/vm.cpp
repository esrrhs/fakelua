#include "vm.h"
#include "fakelua.h"
#include "state/state.h"
#include "util/common.h"

namespace fakelua {

extern "C" __attribute__((used)) var *new_var_nil(fakelua_state *s) {
    auto ret = dynamic_cast<state *>(s)->get_var_pool().alloc();
    return ret;
}

extern "C" __attribute__((used)) var *new_var_false(fakelua_state *s) {
    auto ret = dynamic_cast<state *>(s)->get_var_pool().alloc();
    ret->set_bool(false);
    return ret;
}

extern "C" __attribute__((used)) var *new_var_true(fakelua_state *s) {
    auto ret = dynamic_cast<state *>(s)->get_var_pool().alloc();
    ret->set_bool(true);
    return ret;
}

extern "C" __attribute__((used)) var *new_var_int(fakelua_state *s, int64_t val) {
    auto ret = dynamic_cast<state *>(s)->get_var_pool().alloc();
    ret->set_int(val);
    return ret;
}

extern "C" __attribute__((used)) var *new_var_float(fakelua_state *s, double val) {
    auto ret = dynamic_cast<state *>(s)->get_var_pool().alloc();
    ret->set_float(val);
    return ret;
}

extern "C" __attribute__((used)) var *new_var_string(fakelua_state *s, const char *val) {
    auto ret = dynamic_cast<state *>(s)->get_var_pool().alloc();
    ret->set_string(s, val);
    return ret;
}

extern "C" __attribute__((used)) var *wrap_return_var(fakelua_state *s, int n, ...) {
    auto ret = dynamic_cast<state *>(s)->get_var_pool().alloc();
    ret->set_table();
    // push ... to ret
    va_list args;
    va_start(args, n);
    for (int i = 0; i < n; i++) {
        auto arg = va_arg(args, var *);

        if (arg->type() <= var_type::VAR_INVALID || arg->type() >= var_type::VAR_MAX) {
            throw std::runtime_error(std::format("wrap_return_var: invalid arg type {}", (int) arg->type()));
        }

        auto k = dynamic_cast<state *>(s)->get_var_pool().alloc();
        k->set_int(i + 1);
        ret->get_table().set(k, arg, true);
    }
    va_end(args);
    return ret;
}

}// namespace fakelua
