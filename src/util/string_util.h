#pragma once

namespace fakelua {

// 定义 StrContainerPtr
using StrContainerPtr = std::shared_ptr<std::string>;

inline const auto g_number_regex =
        std::regex("^[+-]?[0-9]+(\\.[0-9]+)?([eE][+-]?[0-9]+)?$|^[+-]?0[xX][0-9a-fA-F]+(\\.[0-9a-fA-F]+)?([pP][+-]?[0-9]+)?$");

inline const auto g_integer_regex = std::regex("^[+-]?[0-9]+$|^[+-]?0[xX][0-9a-fA-F]+$");

inline bool IsNumber(const std::string_view &str) {
    return std::regex_match(str.begin(), str.end(), g_number_regex);
}

inline bool IsInteger(const std::string_view &str) {
    return std::regex_match(str.begin(), str.end(), g_integer_regex);
}

int64_t ToInteger(const std::string_view &input);

double ToFloat(const std::string_view &input);

inline std::string JoinString(const std::vector<std::string> &strs, const std::string &sep) {
    if (strs.empty()) {
        return {};
    }
    return std::accumulate(std::next(strs.begin()), strs.end(), strs[0],
                           [&](std::string acc, const std::string &item) { return std::move(acc) + sep + item; });
}

std::string ReplaceEscapeChars(const std::string &str);

inline void TrimInplace(std::string &str) {
    // 左侧去空格
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char chr) { return !std::isspace(chr); }));

    // 右侧去空格
    str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char chr) { return !std::isspace(chr); }).base(), str.end());
}

}// namespace fakelua
