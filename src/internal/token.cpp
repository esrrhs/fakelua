#include "fakelua.h"
#include "token.h"

const std::string TOKEN_NAME[] = {
        "NONE",
        "and", "break", "do", "else", "elseif",
        "end", "false", "for", "function", "goto", "if",
        "in", "local", "nil", "not", "or", "repeat",
        "return", "then", "true", "until", "while",
        "//", "..", "...", "==", ">=", "<=", "~=",
        "<<", ">>", "::", "<eof>",
        "<number>", "<integer>", "<name>", "<string>",
        "MAX",
};

const char *token::to_string() {
    if (m_format_str.empty()) {
        std::string token_name =
                m_tk > TK_NONE && m_tk < TK_MAX ? TOKEN_NAME[int(m_tk)] : string_format("error token %d", int(m_tk));
        std::string loc_str = m_loc.to_string();
        if (loc_str.empty()) {
            m_format_str = string_format("%s", token_name.c_str());
        } else {
            m_format_str = string_format("%s(%s)", token_name.c_str(), m_loc.to_string());
        }
    }
    return m_format_str.c_str();
}
