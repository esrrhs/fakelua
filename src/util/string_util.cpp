#include "common.h"

namespace fakelua {

/*
    \a: 显示警报铃声符号。
    \b 或 \\: 在字符串中打印反斜杠。
    \f: 换页符，使光标移到下一行开头而不回到行首。
    \n: 常用的换行符，创建新的打印行。
    \r: 回车符，将光标移到当前行开头而不换行。
    \t: 水平制表符。
    \v: 垂直制表符。
    \": 在字符串中插入双引号而不被特殊解释。
    \': 在字符串中插入单引号而不被特殊解释。
    \z: 跳过后续的空格序列
    \digit: 从缓冲区读取数字
 */

std::string ReplaceEscapeChars(const std::string &str) {
    std::string result;
    for (std::string::const_iterator it = str.begin(); it != str.end();) {
        if (*it == '\\') {
            ++it;
            if (it == str.end()) {
                break;
            }
            switch (*it) {
                case 'a':
                    result += '\a';
                    ++it;
                    break;
                case 'b':
                    result += '\b';
                    ++it;
                    break;
                case '\\':
                    result += '\\';
                    ++it;
                    break;
                case 'f':
                    result += '\f';
                    ++it;
                    break;
                case 'n':
                    result += '\n';
                    ++it;
                    break;
                case 'r':
                    result += '\r';
                    ++it;
                    break;
                case 't':
                    result += '\t';
                    ++it;
                    break;
                case 'v':
                    result += '\v';
                    ++it;
                    break;
                case '\"':
                    result += '\"';
                    ++it;
                    break;
                case '\'':
                    result += '\'';
                    ++it;
                    break;
                case 'z':
                    // 跳过后续的空白字符序列（包括空格、换行、制表符等）
                    ++it;
                    while (it != str.end() && std::isspace(static_cast<unsigned char>(*it))) {
                        ++it;
                    }
                    break;
                default:
                    if (!isdigit(static_cast<unsigned char>(*it))) {
                        ThrowFakeluaException(std::format("ReplaceEscapeChars failed, invalid escape sequence \\{}", *it));
                    }
                    // 最多读取3位数字
                    int r = 0; /* 结果累加器 */
                    for (int i = 0; i < 3; ++i) {
                        if (it == str.end() || !isdigit(static_cast<unsigned char>(*it))) {
                            break;
                        }
                        r = 10 * r + *it - '0';
                        ++it;
                    }
                    if (r > 0xFF) {
                        ThrowFakeluaException("ReplaceEscapeChars failed, decimal escape too large \\" + std::to_string(r));
                    }
                    result += static_cast<char>(r);
                    break;
            }
        } else {
            result += *it;
            ++it;
        }
    }
    return result;
}

int64_t ToInteger(const std::string_view &s) {
    int64_t result = 0;

    auto begin = s.begin();
    auto base = 10;
    bool negative = false;
    if (s.length() > 1) {
        if (s[0] == '+') {//+123
            begin += 1;
            if (s.length() > 3 && s[1] == '0' && (s[2] == 'x' || s[2] == 'X')) {// +0x123
                begin += 2;
                base = 16;
            }
        } else if (s.length() > 2) {
            if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {// 0x123
                begin += 2;
                base = 16;
            } else if (s.length() > 3 && s[0] == '-' && s[1] == '0' && (s[2] == 'x' || s[2] == 'X')) {// -0x123
                begin += 3;
                base = 16;
                negative = true;
            }
        }
    }

    auto [ptr, ec] = std::from_chars(begin, s.data() + s.size(), result, base);
    if (ec == std::errc::invalid_argument) {
        ThrowFakeluaException(std::format("ToInteger failed, invalid argument: {}", s));
    } else if (ec == std::errc::result_out_of_range) {
        ThrowFakeluaException(std::format("ToInteger failed, result out of range: {}", s));
    }

    if (negative) {
        result = -result;
    }

    return result;
}

double ToFloat(const std::string_view &s) {
    double result = 0;

    auto begin = s.begin();
    auto fmt = std::chars_format::general;
    bool negative = false;
    if (s.length() > 1) {
        if (s[0] == '+') {//+123
            begin += 1;
            if (s.length() > 3 && s[1] == '0' && (s[2] == 'x' || s[2] == 'X')) {// +0x123
                begin += 2;
                fmt = std::chars_format::hex;
            }
        } else if (s.length() > 2) {
            if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {// 0x123
                begin += 2;
                fmt = std::chars_format::hex;
            } else if (s.length() > 3 && s[0] == '-' && s[1] == '0' && (s[2] == 'x' || s[2] == 'X')) {// -0x123
                begin += 3;
                fmt = std::chars_format::hex;
                negative = true;
            }
        }
    }
#ifdef __APPLE__
    // Apple Clang libc++ does not support std::from_chars for floating-point
    std::string tmp(begin, s.data() + s.size());
    char *end_ptr = nullptr;
    errno = 0;
    result = std::strtod(tmp.c_str(), &end_ptr);
    if (end_ptr == tmp.c_str()) {
        ThrowFakeluaException(std::format("ToFloat failed, invalid argument: {}", s));
    } else if (errno == ERANGE) {
        ThrowFakeluaException(std::format("ToFloat failed, result out of range: {}", s));
    }
#else
    auto [ptr, ec] = std::from_chars(begin, s.data() + s.size(), result, fmt);
    if (ec == std::errc::invalid_argument) {
        ThrowFakeluaException(std::format("ToFloat failed, invalid argument: {}", s));
    } else if (ec == std::errc::result_out_of_range) {
        ThrowFakeluaException(std::format("ToFloat failed, result out of range: {}", s));
    }
#endif

    if (negative) {
        result = -result;
    }

    return result;
}

}// namespace fakelua
