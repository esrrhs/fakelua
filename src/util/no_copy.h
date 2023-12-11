#pragma once

#include "fakelua.h"
#include "hash_func.h"
#include "util/common.h"

namespace fakelua {

template<typename T>
class no_copy {
public:
    no_copy() = default;

    no_copy(const no_copy &) = delete;

    no_copy &operator=(const no_copy &) = delete;

    no_copy(no_copy &&) = default;

    no_copy &operator=(no_copy &&) = default;

    virtual ~no_copy() = default;
};

}// namespace fakelua
