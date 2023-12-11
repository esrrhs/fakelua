#pragma once

#include "fakelua.h"
#include "hash_func.h"
#include "util/common.h"

namespace fakelua {

// explicitly specifying a class as a copyable object
template<typename T>
class copyable {
public:
    copyable() = default;

    copyable(const copyable &) = default;

    copyable &operator=(const copyable &) = default;

    copyable(copyable &&) = default;

    copyable &operator=(copyable &&) = default;

    virtual ~copyable() = default;
};

}// namespace fakelua
