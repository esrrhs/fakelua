#include "var_pool.h"
#include "fakelua.h"
#include "util/common.h"

namespace fakelua {

var *var_pool::alloc() {
    if (next_var_index_ < vars_.size()) {
        auto ret = &vars_[next_var_index_];
        next_var_index_++;
        return ret;
    }

    auto v = std::make_shared<var>();
    tmp_vars_.push_back(v);
    return v.get();
}

void var_pool::reset() {
    next_var_index_ = 0;
    auto tmp_size = tmp_vars_.size();
    tmp_vars_.clear();

    if (tmp_size > 0) {
        vars_.resize(vars_.size() + tmp_size * 2);
    }
}

}// namespace fakelua
