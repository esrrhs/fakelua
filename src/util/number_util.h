#pragma once

#include <cmath>
#include <limits>
#include <optional>

namespace fakelua {

// Try to convert a double to int64_t if it's a finite integer value within int64_t range.
// Returns nullopt if the conversion is not possible.
//
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
