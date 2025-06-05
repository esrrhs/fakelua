#pragma once

#include "fakelua.h"
#include "util/common.h"

namespace fakelua {

class var_table;

// table heap holds all the table, like string, it has two type: const table and tmp string.
class var_table_heap {
public:
    // alloc a table
    var_table *alloc(bool is_const);

    // clear the table heap. usually called before running.
    void reset();

    // get size
    [[nodiscard]] size_t size() const {
        return const_table_vec_.size() + tmp_table_vec_.size();
    }

private:
    // the key is the input string_view, the value is the stored string
    std::vector<var_table *> const_table_vec_;
    std::vector<var_table *> tmp_table_vec_;
};

}// namespace fakelua
