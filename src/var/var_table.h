#pragma once

#include "fakelua.h"
#include "util/common.h"
#include "util/copyable.h"
#include "util/rich_hashmap.h"
#include "var_type.h"

namespace fakelua {

class var;

// table type, like the lua table. but we implement it in a simple way.
// only support integer key and short-string key.
class var_table : public copyable<var_table> {
public:
    var_table() : str_table_(VAR_TABLE_HASHMAP_INIT_BUCKET_SIZE), int_table_(VAR_TABLE_HASHMAP_INIT_BUCKET_SIZE) {
    }

    ~var_table() = default;

    // get value by key. if the key is not exist, return const var(nullptr).
    var *get(var *key);
    var *get_by_int(int64_t key);
    var *get_by_str(const std::string_view &key);

    // set value by key. if the key is not exist, insert a new key-value pair.
    void set(var *key, var *val, bool can_be_nil = false);

    // range for
    void range(std::function<void(var *, var *)> iter);

    // get size
    size_t size() const {
        return str_table_.size() + int_table_.size();
    }

private:
    typedef std::pair<var *, var *> pair_type;
    rich_hashmap<int64_t, pair_type, MAX_VAR_TABLE_HASHMAP_BUCKET_HEIGHT> str_table_;
    rich_hashmap<int64_t, pair_type, MAX_VAR_TABLE_HASHMAP_BUCKET_HEIGHT> int_table_;
};

}// namespace fakelua
