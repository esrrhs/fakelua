#include "var_table_heap.h"
#include "state.h"
#include "util/common.h"
#include "var/var_table.h"

namespace fakelua {

var_table *var_table_heap::alloc(bool is_const) {
    auto t = new var_table();
    if (is_const) {
        const_table_vec_.emplace_back(t);
    } else {
        tmp_table_vec_.emplace_back(t);
    }
}

void var_table_heap::reset() {
    for (auto &iter: tmp_table_vec_) {
        delete iter;
    }
    tmp_table_vec_.clear();
}

}// namespace fakelua
