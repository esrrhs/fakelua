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
        DEBUG_ASSERT(!arg || arg->is_const());
        keys.push_back(arg);
        arg = va_arg(args, var *);
        DEBUG_ASSERT(arg->type() >= var_type::VAR_MIN && arg->type() <= var_type::VAR_MAX);
        DEBUG_ASSERT(arg->is_const());
        values.push_back(arg);
    }
    va_end(args);

    auto ret = h->alloc_var();
    ret->set_table();
    ret->set_const(true);
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

extern "C" __attribute__((used)) var *binop_const_plus(gcc_jit_handle *h, var *l, var *r) {
    DEBUG_ASSERT(h);
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(l->is_const());
    DEBUG_ASSERT(r->is_const());
    auto ret = h->alloc_var();
    ret->set_const(true);
    l->plus(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_const_minus(gcc_jit_handle *h, var *l, var *r) {
    DEBUG_ASSERT(h);
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(l->is_const());
    DEBUG_ASSERT(r->is_const());
    auto ret = h->alloc_var();
    ret->set_const(true);
    l->minus(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_const_star(gcc_jit_handle *h, var *l, var *r) {
    DEBUG_ASSERT(h);
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(l->is_const());
    DEBUG_ASSERT(r->is_const());
    auto ret = h->alloc_var();
    ret->set_const(true);
    l->star(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_const_slash(gcc_jit_handle *h, var *l, var *r) {
    DEBUG_ASSERT(h);
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(l->is_const());
    DEBUG_ASSERT(r->is_const());
    auto ret = h->alloc_var();
    ret->set_const(true);
    l->slash(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_const_double_slash(gcc_jit_handle *h, var *l, var *r) {
    DEBUG_ASSERT(h);
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(l->is_const());
    DEBUG_ASSERT(r->is_const());
    auto ret = h->alloc_var();
    ret->set_const(true);
    l->double_slash(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_const_pow(gcc_jit_handle *h, var *l, var *r) {
    DEBUG_ASSERT(h);
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(l->is_const());
    DEBUG_ASSERT(r->is_const());
    auto ret = h->alloc_var();
    ret->set_const(true);
    l->pow(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_const_mod(gcc_jit_handle *h, var *l, var *r) {
    DEBUG_ASSERT(h);
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(l->is_const());
    DEBUG_ASSERT(r->is_const());
    auto ret = h->alloc_var();
    ret->set_const(true);
    l->mod(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_const_bitand(gcc_jit_handle *h, var *l, var *r) {
    DEBUG_ASSERT(h);
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(l->is_const());
    DEBUG_ASSERT(r->is_const());
    auto ret = h->alloc_var();
    ret->set_const(true);
    l->bitand_(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_const_xor(gcc_jit_handle *h, var *l, var *r) {
    DEBUG_ASSERT(h);
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(l->is_const());
    DEBUG_ASSERT(r->is_const());
    auto ret = h->alloc_var();
    ret->set_const(true);
    l->xor_(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_const_bitor(gcc_jit_handle *h, var *l, var *r) {
    DEBUG_ASSERT(h);
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(l->is_const());
    DEBUG_ASSERT(r->is_const());
    auto ret = h->alloc_var();
    ret->set_const(true);
    l->bitor_(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_const_right_shift(gcc_jit_handle *h, var *l, var *r) {
    DEBUG_ASSERT(h);
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(l->is_const());
    DEBUG_ASSERT(r->is_const());
    auto ret = h->alloc_var();
    ret->set_const(true);
    l->right_shift(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_const_left_shift(gcc_jit_handle *h, var *l, var *r) {
    DEBUG_ASSERT(h);
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(l->is_const());
    DEBUG_ASSERT(r->is_const());
    auto ret = h->alloc_var();
    ret->set_const(true);
    l->left_shift(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_const_concat(gcc_jit_handle *h, var *l, var *r) {
    DEBUG_ASSERT(h);
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(l->is_const());
    DEBUG_ASSERT(r->is_const());
    auto ret = h->alloc_var();
    ret->set_const(true);
    l->concat(h->get_state(), *r, *ret);
    h->alloc_str(ret->get_string());// make the string const
    return ret;
}

extern "C" __attribute__((used)) var *binop_const_less(gcc_jit_handle *h, var *l, var *r) {
    DEBUG_ASSERT(h);
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(l->is_const());
    DEBUG_ASSERT(r->is_const());
    auto ret = h->alloc_var();
    ret->set_const(true);
    l->less(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_const_less_equal(gcc_jit_handle *h, var *l, var *r) {
    DEBUG_ASSERT(h);
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(l->is_const());
    DEBUG_ASSERT(r->is_const());
    auto ret = h->alloc_var();
    ret->set_const(true);
    l->less_equal(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_const_more(gcc_jit_handle *h, var *l, var *r) {
    DEBUG_ASSERT(h);
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(l->is_const());
    DEBUG_ASSERT(r->is_const());
    auto ret = h->alloc_var();
    ret->set_const(true);
    l->more(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_const_more_equal(gcc_jit_handle *h, var *l, var *r) {
    DEBUG_ASSERT(h);
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(l->is_const());
    DEBUG_ASSERT(r->is_const());
    auto ret = h->alloc_var();
    ret->set_const(true);
    l->more_equal(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_const_equal(gcc_jit_handle *h, var *l, var *r) {
    DEBUG_ASSERT(h);
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(l->is_const());
    DEBUG_ASSERT(r->is_const());
    auto ret = h->alloc_var();
    ret->set_const(true);
    l->equal(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_const_not_equal(gcc_jit_handle *h, var *l, var *r) {
    DEBUG_ASSERT(h);
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(l->is_const());
    DEBUG_ASSERT(r->is_const());
    auto ret = h->alloc_var();
    ret->set_const(true);
    l->not_equal(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) bool test_const_var(gcc_jit_handle *h, var *v) {
    DEBUG_ASSERT(h);
    DEBUG_ASSERT(v);
    DEBUG_ASSERT(v->type() >= var_type::VAR_MIN && v->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(v->is_const());
    return v->test_true();
}

extern "C" __attribute__((used)) bool test_const_not_var(gcc_jit_handle *h, var *v) {
    DEBUG_ASSERT(h);
    DEBUG_ASSERT(v);
    DEBUG_ASSERT(v->type() >= var_type::VAR_MIN && v->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(v->is_const());
    return !v->test_true();
}

extern "C" __attribute__((used)) var *unop_const_minus(gcc_jit_handle *h, var *r) {
    DEBUG_ASSERT(h);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->is_const());
    auto ret = h->alloc_var();
    ret->set_const(true);
    r->unop_minus(*ret);
    return ret;
}

extern "C" __attribute__((used)) var *unop_const_not(gcc_jit_handle *h, var *r) {
    DEBUG_ASSERT(h);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->is_const());
    auto ret = h->alloc_var();
    ret->set_const(true);
    r->unop_not(*ret);
    return ret;
}

extern "C" __attribute__((used)) var *unop_const_number_sign(gcc_jit_handle *h, var *r) {
    DEBUG_ASSERT(h);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->is_const());
    auto ret = h->alloc_var();
    ret->set_const(true);
    r->unop_number_sign(*ret);
    return ret;
}

extern "C" __attribute__((used)) var *unop_const_bitnot(gcc_jit_handle *h, var *r) {
    DEBUG_ASSERT(h);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->is_const());
    auto ret = h->alloc_var();
    ret->set_const(true);
    r->unop_bitnot(*ret);
    return ret;
}

///////////////////////////////////////////////////////////////////////////////////

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
        DEBUG_ASSERT(dst);
        DEBUG_ASSERT(!*dst || (*dst && (*dst)->type() >= var_type::VAR_MIN && (*dst)->type() <= var_type::VAR_MAX));
        left.push_back(dst);
    }
    for (int i = 0; i < right_n; i++) {
        auto arg = va_arg(args, var *);
        DEBUG_ASSERT(arg);
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

extern "C" __attribute__((used)) var *binop_plus(fakelua_state *s, var *l, var *r) {
    DEBUG_ASSERT(s);
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    auto ret = dynamic_cast<state *>(s)->get_var_pool().alloc();
    l->plus(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_minus(fakelua_state *s, var *l, var *r) {
    DEBUG_ASSERT(s);
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    auto ret = dynamic_cast<state *>(s)->get_var_pool().alloc();
    l->minus(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_star(fakelua_state *s, var *l, var *r) {
    DEBUG_ASSERT(s);
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    auto ret = dynamic_cast<state *>(s)->get_var_pool().alloc();
    l->star(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_slash(fakelua_state *s, var *l, var *r) {
    DEBUG_ASSERT(s);
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    auto ret = dynamic_cast<state *>(s)->get_var_pool().alloc();
    l->slash(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_double_slash(fakelua_state *s, var *l, var *r) {
    DEBUG_ASSERT(s);
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    auto ret = dynamic_cast<state *>(s)->get_var_pool().alloc();
    l->double_slash(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_pow(fakelua_state *s, var *l, var *r) {
    DEBUG_ASSERT(s);
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    auto ret = dynamic_cast<state *>(s)->get_var_pool().alloc();
    l->pow(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_mod(fakelua_state *s, var *l, var *r) {
    DEBUG_ASSERT(s);
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    auto ret = dynamic_cast<state *>(s)->get_var_pool().alloc();
    l->mod(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_bitand(fakelua_state *s, var *l, var *r) {
    DEBUG_ASSERT(s);
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    auto ret = dynamic_cast<state *>(s)->get_var_pool().alloc();
    l->bitand_(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_xor(fakelua_state *s, var *l, var *r) {
    DEBUG_ASSERT(s);
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    auto ret = dynamic_cast<state *>(s)->get_var_pool().alloc();
    l->xor_(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_bitor(fakelua_state *s, var *l, var *r) {
    DEBUG_ASSERT(s);
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    auto ret = dynamic_cast<state *>(s)->get_var_pool().alloc();
    l->bitor_(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_right_shift(fakelua_state *s, var *l, var *r) {
    DEBUG_ASSERT(s);
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    auto ret = dynamic_cast<state *>(s)->get_var_pool().alloc();
    l->right_shift(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_left_shift(fakelua_state *s, var *l, var *r) {
    DEBUG_ASSERT(s);
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    auto ret = dynamic_cast<state *>(s)->get_var_pool().alloc();
    l->left_shift(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_concat(fakelua_state *s, var *l, var *r) {
    DEBUG_ASSERT(s);
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    auto ret = dynamic_cast<state *>(s)->get_var_pool().alloc();
    l->concat(s, *r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_less(fakelua_state *s, var *l, var *r) {
    DEBUG_ASSERT(s);
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    auto ret = dynamic_cast<state *>(s)->get_var_pool().alloc();
    l->less(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_less_equal(fakelua_state *s, var *l, var *r) {
    DEBUG_ASSERT(s);
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    auto ret = dynamic_cast<state *>(s)->get_var_pool().alloc();
    l->less_equal(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_more(fakelua_state *s, var *l, var *r) {
    DEBUG_ASSERT(s);
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    auto ret = dynamic_cast<state *>(s)->get_var_pool().alloc();
    l->more(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_more_equal(fakelua_state *s, var *l, var *r) {
    DEBUG_ASSERT(s);
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    auto ret = dynamic_cast<state *>(s)->get_var_pool().alloc();
    l->more_equal(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_equal(fakelua_state *s, var *l, var *r) {
    DEBUG_ASSERT(s);
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    auto ret = dynamic_cast<state *>(s)->get_var_pool().alloc();
    l->equal(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_not_equal(fakelua_state *s, var *l, var *r) {
    DEBUG_ASSERT(s);
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    auto ret = dynamic_cast<state *>(s)->get_var_pool().alloc();
    l->not_equal(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) bool test_var(fakelua_state *s, var *v) {
    DEBUG_ASSERT(s);
    DEBUG_ASSERT(v);
    DEBUG_ASSERT(v->type() >= var_type::VAR_MIN && v->type() <= var_type::VAR_MAX);
    return v->test_true();
}

extern "C" __attribute__((used)) bool test_not_var(fakelua_state *s, var *v) {
    DEBUG_ASSERT(s);
    DEBUG_ASSERT(v);
    DEBUG_ASSERT(v->type() >= var_type::VAR_MIN && v->type() <= var_type::VAR_MAX);
    return !v->test_true();
}

extern "C" __attribute__((used)) var *unop_minus(fakelua_state *s, var *r) {
    DEBUG_ASSERT(s);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    auto ret = dynamic_cast<state *>(s)->get_var_pool().alloc();
    r->unop_minus(*ret);
    return ret;
}

extern "C" __attribute__((used)) var *unop_not(fakelua_state *s, var *r) {
    DEBUG_ASSERT(s);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    auto ret = dynamic_cast<state *>(s)->get_var_pool().alloc();
    r->unop_not(*ret);
    return ret;
}

extern "C" __attribute__((used)) var *unop_number_sign(fakelua_state *s, var *r) {
    DEBUG_ASSERT(s);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    auto ret = dynamic_cast<state *>(s)->get_var_pool().alloc();
    r->unop_number_sign(*ret);
    return ret;
}

extern "C" __attribute__((used)) var *unop_bitnot(fakelua_state *s, var *r) {
    DEBUG_ASSERT(s);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    auto ret = dynamic_cast<state *>(s)->get_var_pool().alloc();
    r->unop_bitnot(*ret);
    return ret;
}

extern "C" __attribute__((used)) var *call_var(fakelua_state *s, var *func, int n, ...) {
    DEBUG_ASSERT(s);
    DEBUG_ASSERT(func);
    DEBUG_ASSERT(func->type() >= var_type::VAR_MIN && func->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(n >= 0);

    // TODO

    return nullptr;
}

extern "C" __attribute__((used)) var *table_index_var(fakelua_state *s, var *table, var *key) {
    DEBUG_ASSERT(s);
    DEBUG_ASSERT(table);
    DEBUG_ASSERT(table->type() >= var_type::VAR_MIN && table->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(key);
    DEBUG_ASSERT(key->type() >= var_type::VAR_MIN && key->type() <= var_type::VAR_MAX);

    // TODO

    return nullptr;
}

}// namespace fakelua
