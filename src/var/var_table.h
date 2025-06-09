#pragma once

#include "fakelua.h"
#include "util/common.h"

namespace fakelua {

class var;
class var_string;

// table type, like the lua table. but we implement it in a simple way.
class var_table {
public:
    // get value by key. if the key is not exist, return const var(nullptr).
    [[nodiscard]] var get(const var &key) const;

    // set value by key. if the key is not exist, insert a new key-value pair.
    void set(const var &key, const var &val);

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
