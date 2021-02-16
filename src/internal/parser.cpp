#include "parser.h"
#include "fakelua.h"

int parser::parse(const std::string &file_name, const std::string &content) {

    auto ret = lex(file_name, content);
    if (ret != FAKELUA_OK) {
        return ret;
    }

    ret = parsing();
    if (ret != FAKELUA_OK) {
        return ret;
    }

    ret = compile();
    if (ret != FAKELUA_OK) {
        return ret;
    }

    return ret;
}

int parser::lex(const std::string &file_name, const std::string &content) {
    // replace comment with space
    auto tmp = replace_multi_comment(content);
    tmp = replace_comment(tmp);

    return FAKELUA_OK;
}

int parser::parsing() {
    // TODO
    return FAKELUA_OK;
}

int parser::compile() {
    // TODO
    return FAKELUA_OK;
}
