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

////////////////////////////////////

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

#ifndef NDEBUG
#define DEBUG(...) debug(std::experimental::source_location::current(), __VA_ARGS__)
#define ERR(...) error(std::experimental::source_location::current(), __VA_ARGS__)
#else
#define DEBUG
#define ERR
#endif

////////////////////////////////////

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

// Returns type name of enum.
template<typename E>
[[nodiscard]] constexpr auto
enum_type_name() noexcept -> std::enable_if_t<std::is_enum_v<std::decay_t<E>>, std::string_view> {
    using D = std::decay_t<E>;
    constexpr std::string_view name = get_type_name_v<D>;
    static_assert(name.size() > 0, "Enum type does not have a name.");
    return name;
}

template<auto V>
[[nodiscard]] constexpr auto
enum_name() noexcept -> std::enable_if_t<std::is_enum_v<std::decay_t<decltype(V)>>, std::string_view> {
    using D = std::decay_t<decltype(V)>;
    constexpr std::string_view name = get_val_name_v<D, V>;
    static_assert(name.size() > 0, "Enum value does not have a name.");
    return name;
}

////////////////////////////////////
