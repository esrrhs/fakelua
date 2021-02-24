#include "token.h"

const std::string TOKEN_NAME[] = {
        "NONE",

        "and",
        "break",
        "do",
        "else",
        "elseif",
        "end",
        "false",
        "for",
        "function",
        "goto",
        "if",
        "in",
        "local",
        "nil",
        "not",
        "or",
        "repeat",
        "return",
        "then",
        "true",
        "until",
        "while",
        "//",
        "..",
        "...",
        "==",
        ">=",
        "<=",
        "~=",
        "<<",
        ">>",
        "::",
        "<eof>",
        "<number>",
        "<integer>",
        "<name>",
        "<string>",

        "MAX",
};

token::token(TOKEN tk, const location &loc) : m_tk(tk), m_loc(loc) {

}

token::~token() {

}

std::string token::to_string() {
    auto token_name = m_tk > TK_NONE && m_tk < TK_MAX ?
                      TOKEN_NAME[static_cast<int>(m_tk)] : string_format("error token %d", static_cast<int>(m_tk));
    auto loc_str = m_loc.to_string();
    if (loc_str.empty()) {
        return token_name;
    } else {
        return string_format("%s(%s)", token_name.c_str(), loc_str.c_str());
    }
}
