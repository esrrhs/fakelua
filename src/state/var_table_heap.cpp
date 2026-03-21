#include "var_table_heap.h"
#include "util/common.h"
#include "var/var_table.h"

namespace fakelua {

var_table *var_table_heap::alloc() {
    auto t = new var_table();
    tmp_table_vec_.emplace_back(t);
    return t;
}

void var_table_heap::reset() {
    for (const auto &iter: tmp_table_vec_) {
        delete iter;
    }
    tmp_table_vec_.clear();
}

}// namespace fakelua
