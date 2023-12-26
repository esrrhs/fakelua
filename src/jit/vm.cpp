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
    std::vector<var *> keys;
    std::vector<var *> values;
    va_list args;
    va_start(args, n);
    for (int i = 0; i < n; i += 2) {
        auto arg = va_arg(args, var *);
        DEBUG_ASSERT(!arg || (arg->type() >= var_type::VAR_MIN && arg->type() <= var_type::VAR_MAX));
        keys.push_back(arg);
        arg = va_arg(args, var *);
        DEBUG_ASSERT(arg->type() >= var_type::VAR_MIN && arg->type() <= var_type::VAR_MAX);
        values.push_back(arg);
    }
    va_end(args);

    auto ret = h->alloc_var();
    ret->set_table();
    // push ... to ret
    int index = 1;
    for (size_t i = 0; i < keys.size(); i++) {
        auto k = keys[i];
        if (!k) {
            k = h->alloc_var();
            k->set_int(index);
            index++;
        }
        auto v = values[i];
        ret->get_table().set(k, v);
    }
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
    std::vector<var *> keys;
    std::vector<var *> values;
    va_list args;
    va_start(args, n);
    for (int i = 0; i < n; i += 2) {
        auto arg = va_arg(args, var *);
        DEBUG_ASSERT(!arg || (arg->type() >= var_type::VAR_MIN && arg->type() <= var_type::VAR_MAX));
        keys.push_back(arg);
        arg = va_arg(args, var *);
        DEBUG_ASSERT(arg->type() >= var_type::VAR_MIN && arg->type() <= var_type::VAR_MAX);
        values.push_back(arg);
    }
    va_end(args);

    auto ret = dynamic_cast<state *>(s)->get_var_pool().alloc();
    ret->set_table();
    // push ... to ret
    int index = 1;
    for (size_t i = 0; i < keys.size(); i++) {
        auto k = keys[i];
        auto v = values[i];

        // a = { ... }
        if (!k && i == keys.size() - 1 && v->is_variadic()) {
            DEBUG_ASSERT(v->type() == var_type::VAR_TABLE);

            auto &table = v->get_table();
            for (int j = 1; j <= (int) table.size(); j++) {
                auto key = dynamic_cast<state *>(s)->get_var_pool().alloc();
                key->set_int(index);
                var tmp;
                tmp.set_int(j);
                auto value = table.get(&tmp);
                ret->get_table().set(key, value);
                index++;
            }
        } else {
            if (!k) {
                // local a = {x, y, z}  ->  local a = {[1]=x, [2]=y, [3]=z}
                k = dynamic_cast<state *>(s)->get_var_pool().alloc();
                k->set_int(index);
                index++;
            }
            if (!v->is_variadic()) {
                ret->get_table().set(k, v);
            } else {
                // local a = {x, ..., z}
                DEBUG_ASSERT(v->type() == var_type::VAR_TABLE);

                auto &table = v->get_table();
                // get 1st element
                var tmp;
                tmp.set_int(1);
                auto first = table.get(&tmp);
                ret->get_table().set(k, first);
            }
        }
    }
    return ret;
}

extern "C" __attribute__((used)) var *new_var_wrap(void *p, var *val) {
    DEBUG_ASSERT(p);
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
        DEBUG_ASSERT(arg);
        DEBUG_ASSERT(arg->type() >= var_type::VAR_MIN && arg->type() <= var_type::VAR_MAX);
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
        DEBUG_ASSERT((*dst)->type() >= var_type::VAR_MIN && (*dst)->type() <= var_type::VAR_MAX);
        left.push_back(dst);
    }
    for (int i = 0; i < right_n; i++) {
        auto arg = va_arg(args, var *);
        DEBUG_ASSERT(arg->type() >= var_type::VAR_MIN && arg->type() <= var_type::VAR_MAX);
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
