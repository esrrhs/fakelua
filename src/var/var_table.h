#pragma once

#include "fakelua.h"
#include "util/common.h"
#include "util/rich_hashmap.h"
#include "var_type.h"

namespace fakelua {

class var;

// table type, like the lua table. but we implement it in a simple way.
// only support integer key and short-string key.
class var_table {
public:
    var_table() : str_table_(VAR_TABLE_HASHMAP_INIT_BUCKET_SIZE), int_table_(VAR_TABLE_HASHMAP_INIT_BUCKET_SIZE) {}

    ~var_table() = default;

private:
    rich_hashmap<uint32_t, var *, MAX_VAR_TABLE_HASHMAP_BUCKET_HEIGHT> str_table_;
    rich_hashmap<int64_t, var *, MAX_VAR_TABLE_HASHMAP_BUCKET_HEIGHT> int_table_;
};

}
