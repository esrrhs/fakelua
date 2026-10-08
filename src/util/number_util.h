#pragma once

#include <cmath>
#include <cstdio>
#include <cstring>
#include <limits>
#include <optional>
#include <string>

namespace fakelua {

// Lua 5.4 number-to-string rule (lobject.c::tostringbuff):
// format with "%.14g"; if the result looks like an integer (only sign and
// digits), append ".0" so that floats never print in integer shape.
// Keeps float/integer distinguishable across tostring -> tonumber round-trips.
inline std::string FormatLuaFloat(double value) {
    char buf[64];
    int len = std::snprintf(buf, sizeof(buf), "%.14g", value);
    if (len <= 0) {
        return {};
    }
    // Equivalent to Lua's buff[strspn(buff, "-0123456789")] == '\0'
    size_t i = 0;
    if (buf[i] == '-') {
        ++i;
    }
    for (; i < static_cast<size_t>(len); ++i) {
        if (buf[i] < '0' || buf[i] > '9') {
            break;
        }
    }
    if (i == static_cast<size_t>(len) && static_cast<size_t>(len) + 2 < sizeof(buf)) {
        buf[len++] = '.';
        buf[len++] = '0';
    }
    return std::string(buf, static_cast<size_t>(len));
}

// Try to convert a double to int64_t if it's a finite integer value within int64_t range.
// Returns nullopt if the conversion is not possible.
// The upper bound must be checked as "< 2^63" rather than "<= INT64_MAX" because
// double cannot exactly represent INT64_MAX - converting it to double yields 2^63,
// and if we still compare against INT64_MAX, 2^63 would be incorrectly accepted,
// followed by static_cast<int64_t>(2^63) triggering UB.
inline std::optional<int64_t> TryConvertDoubleToInt64(double double_val) {
    if (!std::isfinite(double_val)) {
        return std::nullopt;
    }
    double int_part = 0;
    if (std::modf(double_val, &int_part) != 0.0) {
        return std::nullopt;
    }
    constexpr double kInt64UpperBoundExclusive = 9223372036854775808.0;// 2^63
    if (int_part < static_cast<double>(std::numeric_limits<int64_t>::min()) || int_part >= kInt64UpperBoundExclusive) {
        return std::nullopt;
    }
    return static_cast<int64_t>(int_part);
}

}// namespace fakelua
