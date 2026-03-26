#pragma once

#include "fakelua.h"
#include "util/common.h"
#include "var/var.h"

namespace fakelua {

// 纯数组实现的Table，不用Hash是因为大部分情况Table的Key都比较少，Hash反而慢。
// 并且在JIT层面本身会有加速查找的方式。
class VarTable {
public:
    // get value by key. if the key does not exist, return const var(nullptr).
    [[nodiscard]] Var Get(const Var &key) const;

    // set value by key. if the key does not exist, insert a new key-value pair.
    void Set(const Var &key, const Var &val, bool can_be_nil);

    // get size
    [[nodiscard]] size_t Size() const {
        return table_.size();
    }

    // get key at pos
    [[nodiscard]] Var KeyAt(size_t pos) const;

    // get value at pos
    [[nodiscard]] Var ValueAt(size_t pos) const;

private:
    std::vector<std::pair<Var, Var>> table_;
};

}// namespace fakelua
