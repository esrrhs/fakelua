#pragma once

#include "fakelua.h"
#include "util/common.h"

namespace fakelua {

class VarTable;

// table heap holds all the table, like string, it has two types: const table and tmp string.
class VarTableHeap {
public:
    // alloc a table
    VarTable *alloc();

    // clear the table heap. usually called before running.
    void reset();

    // get size
    [[nodiscard]] size_t size() const {
        return const_table_vec_.size() + tmp_table_vec_.size();
    }

private:
    // the key is the input string_view, the value is the stored string
    std::vector<VarTable *> const_table_vec_;
    std::vector<VarTable *> tmp_table_vec_;
};

}// namespace fakelua
