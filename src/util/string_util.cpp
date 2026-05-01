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
                    int dec_val = 0; /* 结果累加器 */
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
                    break;
            }
        } else {
            result += *it;
            ++it;
        }
    }
    return result;
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

    // Use strtoll/strtoull for cross-platform compatibility.
    // std::from_chars may not be available on all platforms (e.g., older macOS).
    std::string str(begin, input.end());
    char *end_ptr = nullptr;
    errno = 0;

    if (negative && base == 16) {
        // For negative hex, use strtoull so that the magnitude 0x8000000000000000
        // (which is INT64_MIN) does not cause ERANGE on strtoll.
        const uint64_t uval = strtoull(str.c_str(), &end_ptr, base);
        if (end_ptr == str.c_str() || *end_ptr != '\0') {
            ThrowFakeluaException(std::format("ToInteger failed, invalid argument: {}", input));
        } else if (errno == ERANGE || uval > static_cast<uint64_t>(INT64_MAX) + 1ULL) {
            ThrowFakeluaException(std::format("ToInteger failed, result out of range: {}", input));
        }
        // Reinterpret as signed: -0x8000000000000000 == INT64_MIN is valid.
        result = -static_cast<int64_t>(uval);
    } else {
        result = strtoll(str.c_str(), &end_ptr, base);
        if (end_ptr == str.c_str() || *end_ptr != '\0') {
            ThrowFakeluaException(std::format("ToInteger failed, invalid argument: {}", input));
        } else if (errno == ERANGE) {
            ThrowFakeluaException(std::format("ToInteger failed, result out of range: {}", input));
        }
        if (negative) {
            result = -result;
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
            prefix_len = 3;  // +0x prefix
        } else if (input[0] == '-' && input.length() > 3 && input[1] == '0' && (input[2] == 'x' || input[2] == 'X')) {
            hex_format = true;
            negative = true;
            prefix_len = 3;  // -0x prefix
        }
    }

    // libc++ (macOS) doesn't support std::from_chars for floating point types.
    // Use strtod/strtof as fallback for cross-platform compatibility.
    char *end_ptr = nullptr;
    errno = 0;

    if (hex_format) {
        // Check if it's a hex float (contains '.' or 'p'/'P')
        bool is_hex_float = input.find('.') != std::string_view::npos ||
                            input.find('p') != std::string_view::npos ||
                            input.find('P') != std::string_view::npos;

        if (is_hex_float) {
            // Hex float: pass full string to strtod (it handles sign and 0x prefix)
            // Don't apply negative separately since strtod already handles it.
            std::string str(input.begin(), input.end());
            result = strtod(str.c_str(), &end_ptr);
            negative = false;  // strtod already handled the sign
            if (end_ptr == str.c_str() || *end_ptr != '\0') {
                ThrowFakeluaException(std::format("ToFloat failed, invalid argument: {}", input));
            }
        } else {
            // Hex integer without fractional part: strip prefix and parse as unsigned
            // to correctly handle values >= 0x8000000000000000 (e.g. 0xFFFFFFFFFFFFFFFF).
            std::string str(input.begin() + prefix_len, input.end());
            const uint64_t uval = strtoull(str.c_str(), &end_ptr, 16);
            result = static_cast<double>(uval);
            if (end_ptr == str.c_str() || *end_ptr != '\0') {
                ThrowFakeluaException(std::format("ToFloat failed, invalid argument: {}", input));
            }
        }
    } else {
        std::string str(input.begin(), input.end());
        result = strtod(str.c_str(), &end_ptr);
        if (end_ptr == str.c_str() || *end_ptr != '\0') {
            ThrowFakeluaException(std::format("ToFloat failed, invalid argument: {}", input));
        }
    }

    if (errno == ERANGE) {
        ThrowFakeluaException(std::format("ToFloat failed, result out of range: {}", input));
    }

    if (negative) {
        result = -result;
    }

    return result;
}

}// namespace fakelua
