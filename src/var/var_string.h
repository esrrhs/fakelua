#pragma once

#include "fakelua.h"
#include "util/common.h"
#include "var_type.h"
#include "var_string_index.h"
#include "util/copyable.h"

namespace fakelua {

// the string type
class var_string : public copyable<var_string> {
public:
    var_string() : index_(0) {}

    var_string(bool short_str, uint32_t index) : index_(
            short_str ? make_short_string_index(index) : make_long_string_index(index)) {}

    ~var_string() = default;

    uint32_t string_index() const {
        return index_;
    }

private:
    // index_ is the string heap alloc index.
    uint32_t index_;
};

}// namespace fakelua
