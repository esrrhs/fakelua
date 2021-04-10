#pragma once

#include <string>
#include <list>
#include <vector>
#include <map>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <typeinfo>
#include <time.h>
#include <stdarg.h>
#include <assert.h>
#include <math.h>
#include <fstream>
#include <filesystem>
#include <string_view>
#include <experimental/source_location>
#include <sstream>
#include <regex>
#include <array>
#include <variant>

////////////////////////////////////
// macro define

#ifndef NDEBUG
#define FAKELUA_DEBUG
#else
#endif

////////////////////////////////////
// debug log module

const std::string LOG_DEBUG = "DEBUG";
const std::string LOG_ERROR = "ERROR";

template<typename FormatString, typename... Args>
inline void log(const std::experimental::source_location &location, const std::string &level, const FormatString &fmt,
                Args &&...args) {
    char buf[256] = {0};
    snprintf(buf, sizeof(buf), fmt, std::forward<Args>(args)...);
    std::stringstream ss;
    ss << "[" << level << "]" << location.function_name() << "("
       << std::filesystem::path(location.file_name()).filename().string() << ":"
       << location.line() << "): " << buf << std::endl;
    std::cout << (ss.str());
    std::ofstream ofs;
    ofs.open(level + ".log", std::ios::out | std::ios::app);
    if (!ofs.is_open()) {
        std::cout << "Error opening " << level << ".log for output" << std::endl;
        return;
    }
    ofs << (ss.str());
    ofs.close();
}

template<typename FormatString, typename... Args>
inline void debug(const std::experimental::source_location &location, const FormatString &fmt, Args &&...args) {
    log(location, LOG_DEBUG, fmt, std::forward<Args>(args)...);
}

template<typename FormatString, typename... Args>
inline void error(const std::experimental::source_location &location, const FormatString &fmt, Args &&...args) {
    log(location, LOG_ERROR, fmt, std::forward<Args>(args)...);
}

#ifdef FAKELUA_DEBUG
#define DEBUG(...) debug(std::experimental::source_location::current(), __VA_ARGS__)
#define ERR(...) error(std::experimental::source_location::current(), __VA_ARGS__)
#else
#define DEBUG(...)
#define ERR(...)
#endif

////////////////////////////////////
// enum to string templates

constexpr std::string_view pretty_name(std::string_view name) noexcept {
    for (std::size_t i = name.size(); i > 0; --i) {
        if (!((name[i - 1] >= '0' && name[i - 1] <= '9') || (name[i - 1] >= 'a' && name[i - 1] <= 'z') ||
              (name[i - 1] >= 'A' && name[i - 1] <= 'Z') || (name[i - 1] == '_'))) {
            name.remove_prefix(i);
            break;
        }
    }

    if (name.size() > 0 &&
        ((name.front() >= 'a' && name.front() <= 'z') || (name.front() >= 'A' && name.front() <= 'Z') ||
         (name.front() == '_'))) {
        return name;
    }

    return {}; // Invalid name.
}

template<std::size_t N>
class static_string {
public:
    constexpr explicit static_string(std::string_view str) noexcept : static_string{str,
                                                                                    std::make_index_sequence<N>{}} {
        assert(str.size() == N);
    }

    constexpr const char *data() const noexcept { return chars_.data(); }

    constexpr std::size_t size() const noexcept { return N; }

    constexpr operator std::string_view() const noexcept { return {data(), size()}; }

private:
    template<std::size_t... I>
    constexpr static_string(std::string_view str, std::index_sequence<I...>) noexcept : chars_{{str[I]..., '\0'}} {}

    const std::array<char, N + 1> chars_;
};

template<typename E>
constexpr auto get_type_name() {
    constexpr auto name = pretty_name({__PRETTY_FUNCTION__, sizeof(__PRETTY_FUNCTION__) - 2});
    return static_string<name.size()>{name};
}

template<typename E, E V>
constexpr auto get_val_name() {
    constexpr auto name = pretty_name({__PRETTY_FUNCTION__, sizeof(__PRETTY_FUNCTION__) - 2});
    return static_string<name.size()>{name};
}

template<typename E>
inline constexpr auto get_type_name_v = get_type_name<E>();

template<typename E, E V>
inline constexpr auto get_val_name_v = get_val_name<E, V>();

// Returns type name of enum type
template<typename E>
[[nodiscard]] constexpr auto
enum_type_name() noexcept -> std::enable_if_t<std::is_enum_v<std::decay_t<E>>, std::string_view> {
    using D = std::decay_t<E>;
    constexpr std::string_view name = get_type_name_v<D>;
    static_assert(name.size() > 0, "Enum type does not have a name.");
    return name;
}

// Returns type name of enum value
template<auto V>
[[nodiscard]] constexpr auto
enum_name() noexcept -> std::enable_if_t<std::is_enum_v<std::decay_t<decltype(V)>>, std::string_view> {
    using D = std::decay_t<decltype(V)>;
    constexpr std::string_view name = get_val_name_v<D, V>;
    static_assert(name.size() > 0, "Enum value does not have a name.");
    return name;
}

template<typename T>
inline constexpr bool is_enum_v = std::is_enum_v<T> && std::is_same_v<T, std::decay_t<T>>;

template<typename E, auto V>
constexpr bool is_valid() noexcept {
    return get_val_name<E, static_cast<E>(V)>().size() != 0;
}

template<typename E, int O, typename U = std::underlying_type_t<E>>
constexpr E enum_value(std::size_t i) noexcept {
    static_assert(is_enum_v<E>, "enum_value requires enum type.");
    return static_cast<E>(static_cast<int>(i) + O);
}

template<std::size_t N>
constexpr std::size_t enum_values_count(const std::array<bool, N> &valid) noexcept {
    auto count = std::size_t{0};
    for (std::size_t i = 0; i < valid.size(); ++i) {
        if (valid[i]) {
            ++count;
        }
    }

    return count;
}

template<typename E, int Min, std::size_t... I>
constexpr auto enum_values(std::index_sequence<I...>) noexcept {
    static_assert(is_enum_v<E>, "enum_values requires enum type.");
    constexpr std::array<bool, sizeof...(I)> valid{{is_valid<E, enum_value<E, Min>(I)>()...}};
    constexpr std::size_t count = enum_values_count(valid);

    std::array<E, count> values{};
    for (std::size_t i = 0, v = 0; v < count; ++i) {
        if (valid[i]) {
            values[v++] = enum_value<E, Min>(i);
        }
    }

    return values;
}

template<typename L, typename R>
constexpr bool cmp_less(L lhs, R rhs) noexcept {
    static_assert(std::is_integral_v<L> && std::is_integral_v<R>, "cmp_less requires integral type.");

    if constexpr (std::is_signed_v<L> == std::is_signed_v<R>) {
        // If same signedness (both signed or both unsigned).
        return lhs < rhs;
    } else if constexpr (std::is_signed_v<R>) {
        // If 'right' is negative, then result is 'false', otherwise cast & compare.
        return rhs > 0 && lhs < static_cast<std::make_unsigned_t<R>>(rhs);
    } else {
        // If 'left' is negative, then result is 'true', otherwise cast & compare.
        return lhs < 0 || static_cast<std::make_unsigned_t<L>>(lhs) < rhs;
    }
}

inline constexpr int MIN_ENUM_VALUE = -128;
inline constexpr int MAX_ENUM_VALUE = 128;

template<typename E, typename U = std::underlying_type_t<E>>
constexpr auto enum_values() noexcept {
    static_assert(is_enum_v<E>, "enum_values requires enum type.");
    constexpr int min = MIN_ENUM_VALUE;
    constexpr int max = MAX_ENUM_VALUE;
    constexpr auto range_size = max - min + 1;
    static_assert(range_size > 0, "enum_range requires valid size.");
    static_assert(range_size < (std::numeric_limits<std::uint16_t>::max)(), "enum_range requires valid size.");
    if constexpr (cmp_less((std::numeric_limits<U>::min)(), min)) {
        static_assert(!is_valid<E, enum_value<E, min - 1>(0)>(),
                      "enum_range detects enum value smaller than min range size.");
    }
    if constexpr (cmp_less(range_size, (std::numeric_limits<U>::max)())) {
        static_assert(!is_valid<E, enum_value<E, min>(range_size + 1)>(),
                      "enum_range detects enum value larger than max range size.");
    }

    return enum_values<E, min>(std::make_index_sequence<range_size>{});
}

template<typename E>
inline constexpr auto enum_values_v = enum_values<E>();

template<typename E>
inline constexpr auto enum_count_v = enum_values_v<E>.size();

template<typename E, typename U = std::underlying_type_t<E>>
inline constexpr auto enum_min_v = static_cast<U>(enum_values_v<E>.front());

template<typename E, typename U = std::underlying_type_t<E>>
inline constexpr auto enum_max_v = static_cast<U>(enum_values_v<E>.back());

template<typename E, typename U = std::underlying_type_t<E>>
constexpr std::size_t enum_range_size() noexcept {
    static_assert(is_enum_v<E>, "enum_range_size requires enum type.");
    constexpr auto max = enum_max_v<E>;
    constexpr auto min = enum_min_v<E>;
    constexpr auto range_size = max - min + U{1};
    static_assert(range_size > 0, "enum_range requires valid size.");
    static_assert(range_size < (std::numeric_limits<std::uint16_t>::max)(), "enum_range requires valid size.");
    return static_cast<std::size_t>(range_size);
}

template<typename E>
inline constexpr auto enum_range_size_v = enum_range_size<E>();

template<typename E>
using enum_index_t = std::conditional_t<
        enum_range_size_v<E> < (std::numeric_limits<std::uint8_t>::max)(), std::uint8_t, std::uint16_t>;

template<typename E>
inline constexpr auto invalid_index_v = (std::numeric_limits<enum_index_t<E>>::max)();

template<typename E, std::size_t... I>
constexpr auto enum_names(std::index_sequence<I...>) noexcept {
    static_assert(is_enum_v<E>, "enum_names requires enum type.");
    return std::array<std::string_view, sizeof...(I)>{{get_val_name_v<E, enum_values_v<E>[I]> ...}};
}

template<typename E>
inline constexpr auto enum_names_v = enum_names<E>(std::make_index_sequence<enum_count_v<E>>{});


template<typename E, typename U = std::underlying_type_t<E>>
constexpr bool is_enum_sparse() noexcept {
    static_assert(is_enum_v<E>, "is_enum_sparse requires enum type.");

    return enum_range_size_v<E> != enum_count_v<E>;
}

template<typename E>
inline constexpr bool is_enum_sparse_v = is_enum_sparse<E>();

template<typename E, std::size_t... I>
constexpr auto enum_indexes(std::index_sequence<I...>) noexcept {
    static_assert(is_enum_v<E>, "enum_indexes requires enum type.");
    constexpr auto min = enum_min_v<E>;
    [[maybe_unused]] auto i = enum_index_t<E>{0};

    return std::array<decltype(i), sizeof...(I)>{{(is_valid<E, enum_value<E, min>(I)>() ? i++
                                                                                        : invalid_index_v<E>)...}};
}

template<typename E>
inline constexpr auto enum_indexes_v = enum_indexes<E>(std::make_index_sequence<enum_range_size_v<E>>{});

template<typename E, typename U = std::underlying_type_t<E>>
constexpr std::size_t enum_index(E v) noexcept {
    static_assert(is_enum_v<E>, "enum_index requires enum type.");

    U value = static_cast<U>(v);

    if (const auto i = static_cast<std::size_t>(value - enum_min_v<E>); value >= enum_min_v<E> &&
                                                                        value <= enum_max_v<E>) {
        if constexpr (is_enum_sparse_v<E>) {
            if (const auto idx = enum_indexes_v<E>[i]; idx != invalid_index_v<E>) {
                return idx;
            }
        } else {
            return i;
        }
    }

    return invalid_index_v<E>; // Value out of range.
}

// Returns name from enum value.
// If enum value does not have name or value out of range, returns empty string.
template<typename E>
[[nodiscard]] constexpr auto
enum_name(E value) noexcept -> std::enable_if_t<std::is_enum_v<std::decay_t<E>>, std::string_view> {
    using D = std::decay_t<E>;
    if (const auto i = enum_index<D>(value); i != invalid_index_v<D>) {
        return enum_names_v<D>[i];
    }
    return {}; // Invalid value or out of range.
}

struct char_equal_to {
    constexpr bool operator()(char lhs, char rhs) const noexcept {
        return lhs == rhs;
    }
};

template<typename BinaryPredicate>
constexpr bool cmp_equal(std::string_view lhs, std::string_view rhs,
                         BinaryPredicate &&p) noexcept(std::is_nothrow_invocable_r_v<bool, BinaryPredicate, char, char>) {
    constexpr bool default_predicate = std::is_same_v<std::decay_t<BinaryPredicate>, char_equal_to>;

    if constexpr (default_predicate) {
        static_cast<void>(p);
        return lhs == rhs;
    } else {
        if (lhs.size() != rhs.size()) {
            return false;
        }

        const auto size = lhs.size();
        for (std::size_t i = 0; i < size; ++i) {
            if (!p(lhs[i], rhs[i])) {
                return false;
            }
        }

        return true;
    }
}

// Returns enum value at specified index.
// No bounds checking is performed: the behavior is undefined if index >= number of enum values.
template<typename E>
[[nodiscard]] constexpr auto
enum_value(std::size_t index) noexcept -> std::enable_if_t<std::is_enum_v<std::decay_t<E>>, std::decay_t<E>> {
    using D = std::decay_t<E>;
    static_assert(enum_count_v<D> > 0, "magic_enum requires enum implementation and valid max and min.");

    if constexpr (is_enum_sparse_v<D>) {
        return assert((index < enum_count_v<D>)), enum_values_v<D>[index];
    } else {
        return assert((index < enum_count_v<D>)), enum_value<D, enum_min_v<D>>(index);
    }
}

// Obtains enum value from name.
// Returns optional with enum value.
template<typename E, typename BinaryPredicate>
[[nodiscard]] constexpr auto enum_cast(std::string_view value,
                                       BinaryPredicate p) noexcept(std::is_nothrow_invocable_r_v<bool, BinaryPredicate, char, char>) -> std::enable_if_t<std::is_enum_v<std::decay_t<E>>, std::optional<std::decay_t<E>>> {
    static_assert(std::is_invocable_r_v<bool, BinaryPredicate, char, char>,
                  "enum_cast requires bool(char, char) invocable predicate.");
    using D = std::decay_t<E>;

    for (std::size_t i = 0; i < enum_count_v<D>; ++i) {
        if (cmp_equal(value, enum_names_v<D>[i], p)) {
            return enum_value<D>(i);
        }
    }

    return {}; // Invalid value or out of range.
}

// Obtains enum value from name.
// Returns optional with enum value.
template<typename E>
[[nodiscard]] constexpr auto enum_cast(
        std::string_view value) noexcept -> std::enable_if_t<std::is_enum_v<std::decay_t<E>>, std::optional<std::decay_t<E>>> {
    using D = std::decay_t<E>;

    return enum_cast<D>(value, char_equal_to{});
}

////////////////////////////////////
// strings

template<typename ... Args>
std::string string_format(const std::string_view &format, Args &&...args) {
    auto size = std::snprintf(nullptr, 0, format.data(), std::forward<Args>(args)...);
    std::vector<char> buf(size + 1);
    std::snprintf(&buf[0], buf.size(), format.data(), std::forward<Args>(args)...);
    return std::string(&buf[0], &buf[0] + buf.size() - 1);
}

////////////////////////////////////
// common function
template<typename T>
void USE(T t) {
}

////////////////////////////////////