#include "string_util.h"
#include "exception.h"

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
    for (std::string::const_iterator it = str.begin(); it != str.end(); ) {
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
                        throw_fakelua_exception("invalid escape sequence \\" + *it);
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

}// namespace fakelua
