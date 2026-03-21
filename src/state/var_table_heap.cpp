#include "var_table_heap.h"
#include "util/common.h"
#include "var/var_table.h"

namespace fakelua {

VarTable *VarTableHeap::alloc() {
    auto t = new VarTable();
    tmp_table_vec_.emplace_back(t);
    return t;
}

void VarTableHeap::reset() {
    for (const auto &iter: tmp_table_vec_) {
        delete iter;
    }
    tmp_table_vec_.clear();
}

}// namespace fakelua
