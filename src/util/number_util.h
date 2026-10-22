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

// Compare an integer with a float without first rounding the integer to double.
// Returns -1/0/1 for integer < / == / > floating.  NaN is unordered and must
// be handled by the caller (we return 0 only to keep the helper total).
inline int CompareInt64Double(int64_t integer, double floating) {
    if (std::isnan(floating)) {
        return 0;
    }
    constexpr double kInt64LowerBound = -9223372036854775808.0; // -2^63
    constexpr double kInt64UpperBound = 9223372036854775808.0;  //  2^63
    if (floating >= kInt64UpperBound) {
        return -1;
    }
    if (floating < kInt64LowerBound) {
        return 1;
    }

    // The range check above makes this conversion defined.  Comparing the
    // truncated integer first preserves the fractional ordering as well.
    const int64_t truncated = static_cast<int64_t>(floating);
    if (integer < truncated) return -1;
    if (integer > truncated) return 1;
    const double truncated_as_double = static_cast<double>(truncated);
    if (floating > truncated_as_double) return -1;
    if (floating < truncated_as_double) return 1;
    return 0;
}

inline bool Int64DoubleEqual(int64_t integer, double floating) {
    return !std::isnan(floating) && CompareInt64Double(integer, floating) == 0;
}

}// namespace fakelua
