#include "string_util.h"

namespace fakelua {

/*
    \a: This displays an alarm bell symbol.
    \b or \\: These print a backslash in our string.
    \f: This is an escape sequence literal that is used to create a form feed. A form feed causes the cursor to move down to the next line without returning to the start of the line.
    \n: This is a very popular and commonly used sequence that creates new print lines.
    \r: This is a carriage return escape sequence. It moves the cursor to the beginning of the line without moving to the next line.
    \t: This creates a horizontal tab space.
    \v: This creates a vertical tab space.
    \": This allows us to insert double quotes in our string without special interpretation.
    \': This allows us to insert single quotes in our string without special interpretation.
    \z: zap following span of spaces
    \digit: read digits from buffer
 */

std::string replace_escape_chars(const std::string &str) {
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
                    // zap following span of spaces
                    ++it;
                    while (it != str.end() && *it == ' ') {
                        ++it;
                    }
                    break;
                default:
                    if (!isdigit(*it)) {
                        throw_fakelua_exception(std::format("invalid escape sequence \\{}", *it));
                    }
                    // read up to 3 digits
                    int r = 0; /* result accumulator */
                    for (int i = 0; i < 3; ++i) {
                        if (it == str.end() || !isdigit(*it)) {
                            break;
                        }
                        r = 10 * r + *it - '0';
                        ++it;
                    }
                    if (r > 0xFF) {
                        throw_fakelua_exception("decimal escape too large \\" + std::to_string(r));
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

int64_t to_integer(const std::string_view &s) {
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
        throw_fakelua_exception(std::format("invalid argument: {}", s));
    } else if (ec == std::errc::result_out_of_range) {
        throw_fakelua_exception(std::format("result out of range: {}", s));
    }

    if (negative) {
        result = -result;
    }

    return result;
}

double to_float(const std::string_view &s) {
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
    auto [ptr, ec] = std::from_chars(begin, s.data() + s.size(), result, fmt);
    if (ec == std::errc::invalid_argument) {
        throw_fakelua_exception(std::format("invalid argument: {}", s));
    } else if (ec == std::errc::result_out_of_range) {
        throw_fakelua_exception(std::format("result out of range: {}", s));
    }

    if (negative) {
        result = -result;
    }

    return result;
}

}// namespace fakelua
