#pragma once

namespace fakelua {

// define the StrContainerPtr
typedef std::shared_ptr<std::string> StrContainerPtr;

static const auto g_number_regex =
        std::regex("^[+-]?[0-9]+(\\.[0-9]+)?([eE][+-]?[0-9]+)?$|^[+-]?0[xX][0-9a-fA-F]+(\\.[0-9a-fA-F]+)?([pP][+-]?[0-9]+)?$");

static const auto g_integer_regex = std::regex("^[+-]?[0-9]+$|^[+-]?0[xX][0-9a-fA-F]+$");

inline bool IsNumber(const std::string_view &s) {
    return std::regex_match(s.begin(), s.end(), g_number_regex);
}

inline bool IsInteger(const std::string_view &s) {
    return std::regex_match(s.begin(), s.end(), g_integer_regex);
}

int64_t ToInteger(const std::string_view &s);

double ToFloat(const std::string_view &s);

inline std::string JoinString(const std::vector<std::string> &strs, const std::string &sep) {
    return std::accumulate(std::next(strs.begin()), strs.end(), strs[0],
                           [&](std::string a, const std::string &b) { return std::move(a) + sep + b; });
}

std::string ReplaceEscapeChars(const std::string &str);

inline void TrimInplace(std::string &s) {
    // Left trim
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));

    // Right trim
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
}

}// namespace fakelua
