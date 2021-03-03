#include "parser.h"
#include "fakelua.h"

parser::parser() {
}

parser::~parser() {
}

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
    auto content_no_comment = replace_multi_comment(content);
    content_no_comment = replace_comment(content_no_comment);

    auto tokens = token_string(content_no_comment);

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


std::string parser::change_comment_to_space(std::string str) {
    std::replace_if(str.begin(), str.end(), [](char x) { return x != '\n'; }, ' ');
    return str;
}

std::string parser::replace_multi_comment(std::string str) {
    std::regex e("--\\[\\[(?:[^\\]\\]]|)*\\]\\]");
    std::smatch sm;
    std::string ret = "";
    while (regex_search(str, sm, e)) {
        ret += sm.prefix();
        ret += change_comment_to_space(sm.str());
        str = sm.suffix();
    }
    ret += str;
    return ret;
}

std::string parser::replace_comment(std::string str) {
    std::regex e("--(?:[^\\n]|)*(\\n|$)");
    std::smatch sm;
    std::string ret = "";
    while (regex_search(str, sm, e)) {
        ret += sm.prefix();
        ret += change_comment_to_space(sm.str());
        str = sm.suffix();
    }
    ret += str;
    return ret;
}
