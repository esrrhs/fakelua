#include "var_table.h"
#include "fakelua.h"
#include "util/common.h"
#include "var.h"

namespace fakelua {

size_t table_var_hash_func(const var *v) {
    return v->hash();
}

bool table_var_equal_func(const var *lhs, const var *rhs) {
    return lhs->equal(*rhs);
}

var *var_table::get(var *key) {
    auto it = table_.find(key);
    if (it == table_.end()) {
        return &const_null_var;
    }
    return it->second;
}

void var_table::set(var *key, var *val, bool can_be_nil) {
    auto type = key->type();
    if (!can_be_nil && (!val || val->type() == var_type::VAR_NIL)) {
        table_.erase(key);
        return;
    }
    table_[key] = val;
}

void var_table::range(std::function<void(var *, var *)> iter) {
    for (auto &it: table_) {
        iter(it.first, it.second);
    }
}

}// namespace fakelua
