#pragma once

#include "fakelua.h"
#include "util/common.h"
#include "var/var.h"

namespace fakelua {

// table type, like the lua table. but we implement it in a simple way.
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
