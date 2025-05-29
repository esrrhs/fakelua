#pragma once

namespace fakelua {

#define SET_FLAG_BIT(flag, pos, value) \
    if (value) { \
        flag |= (1 << (pos)); \
    } else { \
        flag &= ~(1 << (pos)); \
    }

#define GET_FLAG_BIT(flag, pos) ((flag >> (pos)) & 1)

}// namespace fakelua
