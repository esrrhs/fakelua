#include "vm.h"
#include "fakelua.h"
#include "state/state.h"
#include "util/common.h"

namespace fakelua {

static var *alloc_val_helper(fakelua_state *s, gcc_jit_handle *h, bool is_const) {
    DEBUG_ASSERT(s);
    DEBUG_ASSERT(h);
    if (is_const) {
        return h->alloc_var();
    } else {
        return dynamic_cast<state *>(s)->get_var_pool().alloc();
    }
}

extern "C" __attribute__((used)) var *new_var_nil(fakelua_state *s, gcc_jit_handle *h, bool is_const) {
    return &const_null_var;
}

extern "C" __attribute__((used)) var *new_var_false(fakelua_state *s, gcc_jit_handle *h, bool is_const) {
    return &const_false_var;
}

extern "C" __attribute__((used)) var *new_var_true(fakelua_state *s, gcc_jit_handle *h, bool is_const) {
    return &const_true_var;
}

extern "C" __attribute__((used)) var *new_var_int(fakelua_state *s, gcc_jit_handle *h, bool is_const, int64_t val) {
    auto ret = alloc_val_helper(s, h, is_const);
    ret->set_int(val);
    return ret;
}

extern "C" __attribute__((used)) var *new_var_float(fakelua_state *s, gcc_jit_handle *h, bool is_const, double val) {
    auto ret = alloc_val_helper(s, h, is_const);
    ret->set_float(val);
    return ret;
}

extern "C" __attribute__((used)) var *new_var_string(fakelua_state *s, gcc_jit_handle *h, bool is_const, const char *val, int len) {
    DEBUG_ASSERT(len >= 0);
    auto ret = alloc_val_helper(s, h, is_const);
    ret->set_string(s, std::string_view(val, len));
    return ret;
}

extern "C" __attribute__((used)) var *new_var_table(fakelua_state *s, gcc_jit_handle *h, bool is_const, int n, ...) {
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

    auto ret = alloc_val_helper(s, h, is_const);
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
                auto key = alloc_val_helper(s, h, is_const);
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
                k = alloc_val_helper(s, h, is_const);
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

extern "C" __attribute__((used)) var *wrap_return_var(fakelua_state *s, gcc_jit_handle *h, bool is_const, int n, ...) {
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

    auto ret = alloc_val_helper(s, h, is_const);
    ret->set_table();
    ret->set_variadic(true);

    // push ... to ret
    int index = 0;
    for (size_t i = 0; i < params.size(); i++) {
        auto arg = params[i];

        if (!arg->is_variadic()) {
            auto k = alloc_val_helper(s, h, is_const);
            k->set_int(index + 1);
            ret->get_table().set(k, arg, true);
            index++;
        } else {
            DEBUG_ASSERT(arg->type() == var_type::VAR_TABLE);

            auto &table = arg->get_table();
            for (int j = 1; j <= (int) table.size(); j++) {
                auto k = alloc_val_helper(s, h, is_const);
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

extern "C" __attribute__((used)) void assign_var(fakelua_state *s, gcc_jit_handle *h, bool is_const, int left_n, int right_n, ...) {
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

extern "C" __attribute__((used)) var *binop_plus(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *l, var *r) {
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    auto ret = alloc_val_helper(s, h, is_const);
    l->plus(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_minus(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *l, var *r) {
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    auto ret = alloc_val_helper(s, h, is_const);
    l->minus(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_star(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *l, var *r) {
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    auto ret = alloc_val_helper(s, h, is_const);
    l->star(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_slash(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *l, var *r) {
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    auto ret = alloc_val_helper(s, h, is_const);
    l->slash(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_double_slash(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *l, var *r) {
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    auto ret = alloc_val_helper(s, h, is_const);
    l->double_slash(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_pow(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *l, var *r) {
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    auto ret = alloc_val_helper(s, h, is_const);
    l->pow(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_mod(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *l, var *r) {
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    auto ret = alloc_val_helper(s, h, is_const);
    l->mod(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_bitand(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *l, var *r) {
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    auto ret = alloc_val_helper(s, h, is_const);
    l->bitand_(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_xor(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *l, var *r) {
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    auto ret = alloc_val_helper(s, h, is_const);
    l->xor_(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_bitor(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *l, var *r) {
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    auto ret = alloc_val_helper(s, h, is_const);
    l->bitor_(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_right_shift(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *l, var *r) {
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    auto ret = alloc_val_helper(s, h, is_const);
    l->right_shift(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_left_shift(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *l, var *r) {
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    auto ret = alloc_val_helper(s, h, is_const);
    l->left_shift(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_concat(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *l, var *r) {
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    auto ret = alloc_val_helper(s, h, is_const);
    l->concat(s, *r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_less(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *l, var *r) {
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    auto ret = alloc_val_helper(s, h, is_const);
    l->less(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_less_equal(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *l, var *r) {
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    auto ret = alloc_val_helper(s, h, is_const);
    l->less_equal(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_more(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *l, var *r) {
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    auto ret = alloc_val_helper(s, h, is_const);
    l->more(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_more_equal(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *l, var *r) {
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    auto ret = alloc_val_helper(s, h, is_const);
    l->more_equal(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_equal(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *l, var *r) {
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    auto ret = alloc_val_helper(s, h, is_const);
    l->equal(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) var *binop_not_equal(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *l, var *r) {
    DEBUG_ASSERT(l);
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(l->type() >= var_type::VAR_MIN && l->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    auto ret = alloc_val_helper(s, h, is_const);
    l->not_equal(*r, *ret);
    return ret;
}

extern "C" __attribute__((used)) bool test_var(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *v) {
    DEBUG_ASSERT(v);
    DEBUG_ASSERT(v->type() >= var_type::VAR_MIN && v->type() <= var_type::VAR_MAX);
    return v->test_true();
}

extern "C" __attribute__((used)) bool test_not_var(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *v) {
    DEBUG_ASSERT(v);
    DEBUG_ASSERT(v->type() >= var_type::VAR_MIN && v->type() <= var_type::VAR_MAX);
    return !v->test_true();
}

extern "C" __attribute__((used)) var *unop_minus(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *r) {
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    auto ret = alloc_val_helper(s, h, is_const);
    r->unop_minus(*ret);
    return ret;
}

extern "C" __attribute__((used)) var *unop_not(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *r) {
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    auto ret = alloc_val_helper(s, h, is_const);
    r->unop_not(*ret);
    return ret;
}

extern "C" __attribute__((used)) var *unop_number_sign(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *r) {
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    auto ret = alloc_val_helper(s, h, is_const);
    r->unop_number_sign(*ret);
    return ret;
}

extern "C" __attribute__((used)) var *unop_bitnot(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *r) {
    DEBUG_ASSERT(r);
    DEBUG_ASSERT(r->type() >= var_type::VAR_MIN && r->type() <= var_type::VAR_MAX);
    auto ret = alloc_val_helper(s, h, is_const);
    r->unop_bitnot(*ret);
    return ret;
}

extern "C" __attribute__((used)) var *call_var(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *func, var *col_key, int n, ...) {
    DEBUG_ASSERT(func);
    DEBUG_ASSERT(func->type() >= var_type::VAR_MIN && func->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(n >= 0);

    // prepare params
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

    // it is a colon call, we need to get function from table
    if (col_key) {
        // now func must be table type
        if (func->type() != var_type::VAR_TABLE) {
            throw_fakelua_exception(std::format("call_var: colon func must be table type, but got {}", func->to_string()));
        }
        auto real_func = func->table_get(col_key);
        DEBUG_ASSERT(real_func);
        // add the 1st self param
        params.insert(params.begin(), func);
        func = real_func;
    }

    // func must be string type
    if (func->type() != var_type::VAR_STRING) {
        throw_fakelua_exception(std::format("call_var: func must be string type, but got {}", func->to_string()));
    }

    // get function address
    auto name = func->get_string();
    auto function = dynamic_cast<state *>(s)->get_vm().get_function(std::string(name));
    if (!function) {
        throw_fakelua_exception(std::format("call_var: function {} not found", name));
    }

    // check params count
    if (!func->is_variadic() && (int) params.size() != function->get_arg_count()) {
        throw_fakelua_exception(
                std::format("call_var: function {} expect {} params, but got {}", name, function->get_arg_count(), params.size()));
    }

    // call function
    DEBUG_ASSERT(function->get_addr());
    auto func_addr = reinterpret_cast<var *(*) (...)>(function->get_addr());
    auto ret = call_var_func(func_addr, params);
    DEBUG_ASSERT(ret);

    return ret;
}

extern "C" __attribute__((used)) var *table_index_by_var(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *table, var *key) {
    DEBUG_ASSERT(table);
    DEBUG_ASSERT(table->type() >= var_type::VAR_MIN && table->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(key);
    DEBUG_ASSERT(key->type() >= var_type::VAR_MIN && key->type() <= var_type::VAR_MAX);
    return table->table_get(key);
}

extern "C" __attribute__((used)) var *table_set(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *table, var *key, var *val) {
    DEBUG_ASSERT(table);
    DEBUG_ASSERT(table->type() >= var_type::VAR_MIN && table->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(key);
    DEBUG_ASSERT(key->type() >= var_type::VAR_MIN && key->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(val);
    DEBUG_ASSERT(val->type() >= var_type::VAR_MIN && val->type() <= var_type::VAR_MAX);
    table->table_set(key, val);
    return table;
}

extern "C" __attribute__((used)) size_t table_size(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *table) {
    DEBUG_ASSERT(table);
    DEBUG_ASSERT(table->type() >= var_type::VAR_MIN && table->type() <= var_type::VAR_MAX);
    return table->table_size();
}

extern "C" __attribute__((used)) var *table_key_by_pos(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *table, size_t pos) {
    DEBUG_ASSERT(table);
    DEBUG_ASSERT(table->type() >= var_type::VAR_MIN && table->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(pos < table->table_size());
    return table->table_key_at(pos);
}

extern "C" __attribute__((used)) var *table_value_by_pos(fakelua_state *s, gcc_jit_handle *h, bool is_const, var *table, size_t pos) {
    DEBUG_ASSERT(table);
    DEBUG_ASSERT(table->type() >= var_type::VAR_MIN && table->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(pos < table->table_size());
    return table->table_value_at(pos);
}

}// namespace fakelua
