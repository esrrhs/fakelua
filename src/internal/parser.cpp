#include "parser.h"
#include "fakelua.h"

int parser::parse(const std::string &file_name, const std::string &content) {

    auto ret = lexer(file_name, content);
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

int parser::lexer(const std::string &file_name, const std::string &content) {
    // TODO
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
