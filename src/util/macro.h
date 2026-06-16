#pragma once

namespace fakelua {

#define LIKELY(x)   __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)

#define SET_FLAG_BIT(flag, pos, value)                                                                                                     \
    if (value) {                                                                                                                           \
        (flag) |= (1U << (pos));                                                                                                           \
    } else {                                                                                                                               \
        (flag) &= ~(1U << (pos));                                                                                                          \
    }

#define GET_FLAG_BIT(flag, pos) (((flag) >> (pos)) & 1U)

}// namespace fakelua
