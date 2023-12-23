#include "vm.h"
#include "fakelua.h"
#include "state/state.h"
#include "util/common.h"

namespace fakelua {

extern "C" __attribute__((used)) var *new_const_var_nil(gcc_jit_handle *h) {
    DEBUG_ASSERT(h);
    return &const_null_var;
}

extern "C" __attribute__((used)) var *new_const_var_false(gcc_jit_handle *h) {
    DEBUG_ASSERT(h);
    return &const_false_var;
}

extern "C" __attribute__((used)) var *new_const_var_true(gcc_jit_handle *h) {
    DEBUG_ASSERT(h);
    return &const_true_var;
}

extern "C" __attribute__((used)) var *new_const_var_int(gcc_jit_handle *h, int64_t val) {
    DEBUG_ASSERT(h);
    auto ret = h->alloc_var();
    ret->set_int(val);
    ret->set_const(true);
    return ret;
}

extern "C" __attribute__((used)) var *new_const_var_float(gcc_jit_handle *h, double val) {
    DEBUG_ASSERT(h);
    auto ret = h->alloc_var();
    ret->set_float(val);
    ret->set_const(true);
    return ret;
}

extern "C" __attribute__((used)) var *new_const_var_string(gcc_jit_handle *h, const char *val, int len) {
    DEBUG_ASSERT(h);
    DEBUG_ASSERT(val);
    DEBUG_ASSERT(len >= 0);
    auto ret = h->alloc_var();
    ret->set_string(h->get_state(), std::string_view(val, len));
    ret->set_const(true);
    return ret;
}

extern "C" __attribute__((used)) var *new_const_var_table(gcc_jit_handle *h, int n, ...) {
    DEBUG_ASSERT(h);
    DEBUG_ASSERT(n % 2 == 0);
    auto ret = h->alloc_var();
    ret->set_table();
    // push ... to ret
    va_list args;
    va_start(args, n);
    for (int i = 0; i < n; i += 2) {
        auto k = va_arg(args, var *);
        auto v = va_arg(args, var *);
        ret->get_table().set(k, v);
    }
    va_end(args);
    return ret;
}

extern "C" __attribute__((used)) var *new_var_nil(fakelua_state *s) {
    DEBUG_ASSERT(s);
    return &const_null_var;
}

extern "C" __attribute__((used)) var *new_var_false(fakelua_state *s) {
    DEBUG_ASSERT(s);
    return &const_false_var;
}

extern "C" __attribute__((used)) var *new_var_true(fakelua_state *s) {
    DEBUG_ASSERT(s);
    return &const_true_var;
}

extern "C" __attribute__((used)) var *new_var_int(fakelua_state *s, int64_t val) {
    DEBUG_ASSERT(s);
    auto ret = dynamic_cast<state *>(s)->get_var_pool().alloc();
    ret->set_int(val);
    return ret;
}

extern "C" __attribute__((used)) var *new_var_float(fakelua_state *s, double val) {
    DEBUG_ASSERT(s);
    auto ret = dynamic_cast<state *>(s)->get_var_pool().alloc();
    ret->set_float(val);
    return ret;
}

extern "C" __attribute__((used)) var *new_var_string(fakelua_state *s, const char *val, int len) {
    DEBUG_ASSERT(s);
    DEBUG_ASSERT(val);
    DEBUG_ASSERT(len >= 0);
    auto ret = dynamic_cast<state *>(s)->get_var_pool().alloc();
    ret->set_string(s, std::string_view(val, len));
    return ret;
}

extern "C" __attribute__((used)) var *new_var_table(fakelua_state *s, int n, ...) {
    DEBUG_ASSERT(s);
    DEBUG_ASSERT(n % 2 == 0);
    DEBUG_ASSERT(n >= 0);
    std::vector<var *> params;
    va_list args;
    va_start(args, n);
    for (int i = 0; i < n; i++) {
        auto arg = va_arg(args, var *);
        DEBUG_ASSERT(!arg || (arg->type() > var_type::VAR_INVALID && arg->type() < var_type::VAR_MAX));
        params.push_back(arg);
    }
    va_end(args);

    auto ret = dynamic_cast<state *>(s)->get_var_pool().alloc();
    ret->set_table();
    // push ... to ret
    for (size_t i = 0; i < params.size(); i += 2) {
        auto k = va_arg(args, var *);
        auto v = va_arg(args, var *);

        if (!v->is_variadic()) {
            ret->get_table().set(k, v);
        } else {
            DEBUG_ASSERT(v->type() == var_type::VAR_TABLE);

            auto &table = v->get_table();
            table.range([ret](var *key, var *value) { ret->get_table().set(key, value); });
        }
    }
    va_end(args);
    return ret;
}

extern "C" __attribute__((used)) var *new_var_wrap(fakelua_state *s, var *val) {
    DEBUG_ASSERT(s);
    return val;
}

extern "C" __attribute__((used)) var *wrap_return_var(fakelua_state *s, int n, ...) {
    DEBUG_ASSERT(s);
    DEBUG_ASSERT(n >= 0);
    std::vector<var *> params;
    va_list args;
    va_start(args, n);
    for (int i = 0; i < n; i++) {
        auto arg = va_arg(args, var *);
        DEBUG_ASSERT(arg->type() > var_type::VAR_INVALID && arg->type() < var_type::VAR_MAX);
        params.push_back(arg);
    }
    va_end(args);

    auto ret = dynamic_cast<state *>(s)->get_var_pool().alloc();
    ret->set_table();
    ret->set_variadic(true);

    // push ... to ret
    int index = 0;
    for (size_t i = 0; i < params.size(); i++) {
        auto arg = params[i];

        if (!arg->is_variadic()) {
            auto k = dynamic_cast<state *>(s)->get_var_pool().alloc();
            k->set_int(index + 1);
            ret->get_table().set(k, arg, true);
            index++;
        } else {
            DEBUG_ASSERT(arg->type() == var_type::VAR_TABLE);

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
    return ret;
}

extern "C" __attribute__((used)) void assign_var(fakelua_state *s, int left_n, int right_n, ...) {
    DEBUG_ASSERT(s);
    DEBUG_ASSERT(left_n >= 0);
    DEBUG_ASSERT(right_n >= 0);
    std::vector<var **> left;
    std::vector<var *> right;
    va_list args;
    va_start(args, right_n);
    for (int i = 0; i < left_n; i++) {
        auto dst = va_arg(args, var **);
        DEBUG_ASSERT((*dst)->type() > var_type::VAR_INVALID && (*dst)->type() < var_type::VAR_MAX);
        left.push_back(dst);
    }
    for (int i = 0; i < right_n; i++) {
        auto arg = va_arg(args, var *);
        DEBUG_ASSERT(arg->type() > var_type::VAR_INVALID && arg->type() < var_type::VAR_MAX);
        right.push_back(arg);
    }
    va_end(args);

    // local a, b, c = ...
    if (right_n == 1 && right[0]->is_variadic()) {
        auto v = right[0];
        DEBUG_ASSERT(v->type() == var_type::VAR_TABLE);

        for (size_t i = 0; i < left.size(); i++) {
            auto dst = left[i];
            auto &table = v->get_table();
            if (i < table.size()) {
                var tmp;
                tmp.set_int(i + 1);
                *dst = table.get(&tmp);
            } else {
                break;
            }
        }

        return;
    }

    // local a, b, c = x, y, z
    for (size_t i = 0; i < left.size(); i++) {
        auto dst = left[i];
        if (i < right.size()) {
            auto v = right[i];
            if (!v->is_variadic()) {
                *dst = v;
            } else {
                DEBUG_ASSERT(v->type() == var_type::VAR_TABLE);

                auto &table = v->get_table();
                var tmp;
                tmp.set_int(1);
                *dst = table.get(&tmp);
            }
        } else {
            break;
        }
    }
}

}// namespace fakelua
