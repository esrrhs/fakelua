#pragma once

#include "fakelua.h"
#include "util/common.h"
#include "var/var.h"

namespace fakelua {

// table type, like the lua table. but we implement it in a simple way.
class var_table {
public:
    // get value by key. if the key does not exist, return const var(nullptr).
    [[nodiscard]] var get(const var &key) const;

    // set value by key. if the key does not exist, insert a new key-value pair.
    void set(const var &key, const var &val, bool can_be_nil);

    // get size
    [[nodiscard]] size_t size() const {
        return table_.size();
    }

    // get key at pos
    [[nodiscard]] var key_at(size_t pos) const;

    // get value at pos
    [[nodiscard]] var value_at(size_t pos) const;

private:
    std::vector<std::pair<var, var>> table_;
};

}// namespace fakelua
