#pragma once

#include "fakelua.h"
#include "util/common.h"
#include "var_type.h"

namespace fakelua {

class var;

size_t table_var_hash_func(const var *v);
bool table_var_equal_func(const var *lhs, const var *rhs);

// table type, like the lua table. but we implement it in a simple way.
// only support integer key and short-string key.
class var_table {
public:
    var_table() = default;

    ~var_table() = default;

    // get value by key. if the key is not exist, return const var(nullptr).
    var *get(var *key);

    // set value by key. if the key is not exist, insert a new key-value pair.
    void set(var *key, var *val, bool can_be_nil = false);

    // range for
    void range(std::function<void(var *, var *)> iter);

    // get size
    size_t size() const {
        return table_.size();
    }

private:
    struct table_var_hash {
        size_t operator()(const var *k) const {
            return table_var_hash_func(k);
        }
    };
    struct table_var_equal {
        bool operator()(const var *lhs, const var *rhs) const {
            return table_var_equal_func(lhs, rhs);
        }
    };
    std::unordered_map<var *, var *, table_var_hash, table_var_equal> table_;
};

}// namespace fakelua
