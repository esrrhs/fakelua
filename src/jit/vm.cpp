#include "vm.h"
#include "fakelua.h"
#include "state/state.h"
#include "util/common.h"

namespace fakelua {

extern "C" __attribute__((used)) var *new_const_var_nil(gcc_jit_handle *h) {
    auto ret = h->alloc_var();
    ret->set_const(true);
    return ret;
}

extern "C" __attribute__((used)) var *new_const_var_false(gcc_jit_handle *h) {
    auto ret = h->alloc_var();
    ret->set_bool(false);
    ret->set_const(true);
    return ret;
}

extern "C" __attribute__((used)) var *new_const_var_true(gcc_jit_handle *h) {
    auto ret = h->alloc_var();
    ret->set_bool(true);
    ret->set_const(true);
    return ret;
}

extern "C" __attribute__((used)) var *new_const_var_int(gcc_jit_handle *h, int64_t val) {
    auto ret = h->alloc_var();
    ret->set_int(val);
    ret->set_const(true);
    return ret;
}

extern "C" __attribute__((used)) var *new_const_var_float(gcc_jit_handle *h, double val) {
    auto ret = h->alloc_var();
    ret->set_float(val);
    ret->set_const(true);
    return ret;
}

extern "C" __attribute__((used)) var *new_const_var_string(gcc_jit_handle *h, const char *val, int len) {
    auto ret = h->alloc_var();
    ret->set_string(h->get_state(), std::string_view(val, len));
    ret->set_const(true);
    return ret;
}

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

extern "C" __attribute__((used)) var *new_var_string(fakelua_state *s, const char *val, int len) {
    auto ret = dynamic_cast<state *>(s)->get_var_pool().alloc();
    ret->set_string(s, std::string_view(val, len));
    return ret;
}

extern "C" __attribute__((used)) var *new_var_wrap(fakelua_state *s, var *val) {
    return val;
}

extern "C" __attribute__((used)) var *wrap_return_var(fakelua_state *s, int n, ...) {
    auto ret = dynamic_cast<state *>(s)->get_var_pool().alloc();
    ret->set_table();
    // push ... to ret
    va_list args;
    va_start(args, n);
    int index = 0;
    for (int i = 0; i < n; i++) {
        auto arg = va_arg(args, var *);

        if (arg->type() <= var_type::VAR_INVALID || arg->type() >= var_type::VAR_MAX) {
            va_end(args);
            throw std::runtime_error(std::format("wrap_return_var: invalid arg type {}", (int) arg->type()));
        }

        if (!arg->is_variadic()) {
            auto k = dynamic_cast<state *>(s)->get_var_pool().alloc();
            k->set_int(index + 1);
            ret->get_table().set(k, arg, true);
            index++;
        } else {
            if (arg->type() != var_type::VAR_TABLE) {
                va_end(args);
                throw std::runtime_error(std::format("wrap_return_var: invalid variadic arg type {}", (int) arg->type()));
            }

            auto &table = arg->get_table();
            for (int j = 1; j <= (int) table.size(); j++) {
                auto k = dynamic_cast<state *>(s)->get_var_pool().alloc();
                k->set_int(index + 1);
                var tmp;
                tmp.set_int(j);
                auto v = table.get(&tmp);
                ret->get_table().set(k, v, true);
                index++;
            }
        }
    }
    va_end(args);
    return ret;
}

}// namespace fakelua
