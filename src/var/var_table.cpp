#include "var_table.h"
#include "fakelua.h"
#include "util/common.h"
#include "var.h"

namespace fakelua {

const var *var_table::get(var *key) const {
    DEBUG_ASSERT(key);
    DEBUG_ASSERT(key->type() >= var_type::VAR_MIN && key->type() <= var_type::VAR_MAX);
    for (const auto &it: table_) {
        if (it.first.equal(*key)) {
            return &it.second;
        }
    }
    return &const_null_var;
}

void var_table::set(const var *key, const var *val) {
    DEBUG_ASSERT(key);
    DEBUG_ASSERT(key->type() >= var_type::VAR_MIN && key->type() <= var_type::VAR_MAX);
    DEBUG_ASSERT(val);
    DEBUG_ASSERT(val->type() >= var_type::VAR_MIN && val->type() <= var_type::VAR_MAX);

    // set nil means delete
    if (val->type() == var_type::VAR_NIL) {
        for (size_t index = 0; index < table_.size(); ++index) {
            const auto &it = table_[index];
            if (it.first.equal(*key)) {
                if (index < table_.size() - 1) {
                    // swap with the last element
                    table_[index] = table_.back();
                }
                // pop back
                table_.pop_back();
                break;
            }
        }
        return;
    }

    // find and update the value
    for (auto &it: table_) {
        if (it.first.equal(*key)) {
            it.second = *val;
            return;
        }
    }

    // not found, insert a new key-value pair
    table_.emplace_back(*key, *val);
}

const var *var_table::key_at(size_t pos) const {
    if (pos >= table_.size()) {
        return &const_null_var;
    }
    return &table_[pos].first;
}

const var *var_table::value_at(size_t pos) const {
    if (pos >= table_.size()) {
        return &const_null_var;
    }
    return &table_[pos].second;
}

}// namespace fakelua
