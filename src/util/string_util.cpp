#include "common.h"

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/charconv.hpp>
#include <boost/regex.hpp>
#include <locale>

namespace fakelua {

std::string JoinString(const std::vector<std::string> &strs, const std::string &sep) {
    return boost::algorithm::join(strs, sep);
}

void TrimInplace(std::string &str) {
    boost::algorithm::trim(str, std::locale::classic());
}

/*
    \a: 显示警报铃声符号。
    \b 或 \\: 在字符串中打印反斜杠。
    \f: 换页符，使光标移到下一行开头而不回到行首。
    \n: 常用的换行符，创建新的打印行。
    \r: 回车符，将光标移到当前行开头而不回到行首。
    \t: 水平制表符。
    \v: 垂直制表符。
    \": 在字符串中插入双引号而不被特殊解释。
    \': 在字符串中插入单引号而不被特殊解释。
    \z: 跳过后续的空格序列
    \digit: 从缓冲区读取数字
 */

std::string ReplaceEscapeChars(const std::string &str) {
    std::string result;
    for (auto it = str.begin(); it != str.end();) {
        if (*it == '\\') {
            ++it;
            if (it == str.end()) {
                break;
            }
            if (isdigit(static_cast<unsigned char>(*it))) {
                int dec_val = 0;
                for (int i = 0; i < 3; ++i) {
                    if (it == str.end() || !isdigit(static_cast<unsigned char>(*it))) {
                        break;
                    }
                    dec_val = 10 * dec_val + *it - '0';
                    ++it;
                }
                if (dec_val > 0xFF) {
                    ThrowFakeluaException("ReplaceEscapeChars failed, decimal escape too large \\" + std::to_string(dec_val));
                }
                result += static_cast<char>(dec_val);
            } else {
                char esc = *it;
                ++it;
                switch (esc) {
                    case 'a':
                        result += '\a';
                        break;
                    case 'b':
                        result += '\b';
                        break;
                    case '\\':
                        result += '\\';
                        break;
                    case 'f':
                        result += '\f';
                        break;
                    case 'n':
                        result += '\n';
                        break;
                    case 'r':
                        result += '\r';
                        break;
                    case 't':
                        result += '\t';
                        break;
                    case 'v':
                        result += '\v';
                        break;
                    case '\"':
                        result += '\"';
                        break;
                    case '\'':
                        result += '\'';
                        break;
                    case 'z':
                        while (it != str.end() && std::isspace(static_cast<unsigned char>(*it))) {
                            ++it;
                        }
                        break;
                    default:
                        ThrowFakeluaException(std::format("ReplaceEscapeChars failed, invalid escape sequence \\{}", esc));
                        break;
                }
            }
        } else {
            result += *it;
            ++it;
        }
    }
    return result;
}

bool IsNumber(const std::string_view &str) {
    static const boost::regex re("^[+-]?[0-9]+(\\.[0-9]+)?([eE][+-]?[0-9]+)?$|^[+-]?0[xX][0-9a-fA-F]+(\\.[0-9a-fA-F]+)?([pP][+-]?[0-9]+)?$");
    return boost::regex_match(str.begin(), str.end(), re);
}

bool IsInteger(const std::string_view &str) {
    static const boost::regex re("^[+-]?[0-9]+$|^[+-]?0[xX][0-9a-fA-F]+$");
    return boost::regex_match(str.begin(), str.end(), re);
}

namespace {

const char *SkipPlus(const char *first, const char *last) {
    if (first < last && *first == '+') {
        return first + 1;
    }
    return first;
}

bool ConsumeAll(boost::charconv::from_chars_result r, const char *last) {
    return r.ec == std::errc{} && r.ptr == last;
}

}// namespace

bool TryParseInt64(const std::string_view &input, int64_t &out) {
    if (input.empty()) {
        return false;
    }
    const char *first = input.data();
    const char *last = first + input.size();
    first = SkipPlus(first, last);
    if (first >= last) {
        return false;
    }

    auto r = boost::charconv::from_chars(first, last, out, 10);
    return ConsumeAll(r, last);
}

bool TryParseDouble(const std::string_view &input, double &out) {
    if (input.empty()) {
        return false;
    }
    const char *first = input.data();
    const char *last = first + input.size();
    first = SkipPlus(first, last);
    if (first >= last) {
        return false;
    }

    auto r = boost::charconv::from_chars(first, last, out);
    if (!ConsumeAll(r, last)) {
        return false;
    }
    return std::isfinite(out);
}

int64_t ToInteger(const std::string_view &input) {
    int64_t result = 0;

    auto begin = input.begin();
    auto base = 10;
    bool negative = false;
    if (input.length() > 1) {
        if (input[0] == '+') {//+123
            begin += 1;
            if (input.length() > 3 && input[1] == '0' && (input[2] == 'x' || input[2] == 'X')) {// +0x123
                begin += 2;
                base = 16;
            }
        } else if (input.length() > 2) {
            if (input[0] == '0' && (input[1] == 'x' || input[1] == 'X')) {// 0x123
                begin += 2;
                base = 16;
            } else if (input.length() > 3 && input[0] == '-' && input[1] == '0' && (input[2] == 'x' || input[2] == 'X')) {// -0x123
                begin += 3;
                base = 16;
                negative = true;
            }
        }
    }

    const char *first = &*begin;
    const char *last = input.data() + input.size();
    if (first >= last) {
        ThrowFakeluaException(std::format("ToInteger failed, invalid argument: {}", input));
    }

    if (negative && base == 16) {
        // For negative hex, parse as unsigned so the magnitude 0x8000000000000000
        // (which is INT64_MIN) does not overflow a signed from_chars.
        uint64_t uval = 0;
        auto r = boost::charconv::from_chars(first, last, uval, base);
        if (!ConsumeAll(r, last)) {
            if (r.ec == std::errc::result_out_of_range) {
                ThrowFakeluaException(std::format("ToInteger failed, result out of range: {}", input));
            }
            ThrowFakeluaException(std::format("ToInteger failed, invalid argument: {}", input));
        }
        if (uval > static_cast<uint64_t>(INT64_MAX) + 1ULL) {
            ThrowFakeluaException(std::format("ToInteger failed, result out of range: {}", input));
        }
        // Reinterpret as signed: -0x8000000000000000 == INT64_MIN is valid。
        // 直接 reinterpret 避免 -INT64_MIN 溢出（UB）。
        // uval 范围 [0, 2^63]，对应 int64 范围 [0, INT64_MIN]，全部合法。
        result = static_cast<int64_t>(uval);
        if (uval == static_cast<uint64_t>(INT64_MAX) + 1ULL) {
            result = INT64_MIN;// 唯一负值情况
        } else if (uval != 0) {
            result = -result;// 其他情况取负
        }
    } else {
        auto r = boost::charconv::from_chars(first, last, result, base);
        if (!ConsumeAll(r, last)) {
            if (r.ec == std::errc::result_out_of_range) {
                ThrowFakeluaException(std::format("ToInteger failed, result out of range: {}", input));
            }
            ThrowFakeluaException(std::format("ToInteger failed, invalid argument: {}", input));
        }
    }

    return result;
}

double ToFloat(const std::string_view &input) {
    double result = 0;

    // Check for hex format prefix (0x or 0X)
    bool hex_format = false;
    bool negative = false;
    size_t prefix_len = 0;

    if (input.length() > 2) {
        if (input[0] == '0' && (input[1] == 'x' || input[1] == 'X')) {
            hex_format = true;
            prefix_len = 2;
        } else if (input[0] == '+' && input.length() > 3 && input[1] == '0' && (input[2] == 'x' || input[2] == 'X')) {
            hex_format = true;
            prefix_len = 3;// +0x prefix
        } else if (input[0] == '-' && input.length() > 3 && input[1] == '0' && (input[2] == 'x' || input[2] == 'X')) {
            hex_format = true;
            negative = true;
            prefix_len = 3;// -0x prefix
        }
    }

    const char *first = input.data();
    const char *last = first + input.size();

    if (hex_format) {
        if (bool is_hex_float = input.contains('.') || input.contains('p') || input.contains('P'); is_hex_float) {
            // Hex float: Boost.Charconv accepts an optional 0x prefix with chars_format::hex.
            // Sign is parsed by from_chars, so pass the original string.
            first = SkipPlus(input.data(), last);
            auto r = boost::charconv::from_chars(first, last, result, boost::charconv::chars_format::hex);
            if (!ConsumeAll(r, last)) {
                ThrowFakeluaException(std::format("ToFloat failed, invalid argument: {}", input));
            }
            negative = false;
        } else {
            // Hex integer without fractional part: strip prefix and parse as unsigned
            // to correctly handle values >= 0x8000000000000000 (e.g. 0xFFFFFFFFFFFFFFFF).
            const char *digits = input.data() + prefix_len;
            uint64_t uval = 0;
            auto r = boost::charconv::from_chars(digits, last, uval, 16);
            if (!ConsumeAll(r, last)) {
                if (r.ec == std::errc::result_out_of_range) {
                    ThrowFakeluaException(std::format("ToFloat failed, result out of range: {}", input));
                }
                ThrowFakeluaException(std::format("ToFloat failed, invalid argument: {}", input));
            }
            result = static_cast<double>(uval);
        }
    } else {
        first = SkipPlus(first, last);
        auto r = boost::charconv::from_chars(first, last, result);
        if (!ConsumeAll(r, last)) {
            if (r.ec == std::errc::result_out_of_range) {
                ThrowFakeluaException(std::format("ToFloat failed, result out of range: {}", input));
            }
            ThrowFakeluaException(std::format("ToFloat failed, invalid argument: {}", input));
        }
        negative = false;
        // from_chars already applied a leading minus; SkipPlus only dropped '+'.
        if (!input.empty() && input[0] == '-') {
            // already in result
        }
    }

    if (negative) {
        result = -result;
    }

    return result;
}

}// namespace fakelua
