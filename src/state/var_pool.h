#pragma once

#include "fakelua.h"
#include "util/common.h"
#include "var/var.h"

namespace fakelua {

// var_pool only used to transfer data from and to native.
// it temply alloc var pointer to hold the data.
class var_pool {
public:
    var_pool() = default;

    ~var_pool() = default;

    // allocate a var with the type of nil.
    var *alloc();

    // reset the var pool used index and check size. usually called before running.
    void reset();

private:
    // the vector stores the global vars. the length of the vector is the max we can use.
    // if next_var_index_ is equal to the length of the vector, we can't allocate more vars.
    // so we use tmp_vars_ to store the vars which are allocated temporarily.
    // and then we resize the vars_ to bigger size.
    std::vector<var> vars_;

    // the index of the next var to be allocated.
    uint32_t next_var_index_ = 0;

    // the vector stores the pointers to the vars which are allocated temporarily.
    std::vector<std::shared_ptr<var>> tmp_vars_;
};

}// namespace fakelua
