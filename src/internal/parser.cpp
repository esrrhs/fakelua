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
    auto tokens = token_string(content);

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

std::vector<std::tuple<std::string, int, int>> parser::token_string(const std::string &str) {
    std::vector<std::tuple<std::string, int, int>> ret;
    // TODO
    return ret;
}
