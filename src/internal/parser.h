#pragma once

#include "types.h"
#include "tester.h"

class parser {
    friend class tester;

public:
    parser();

    virtual ~parser();

    parser(const parser &) = delete;

    parser &operator=(const parser &) = delete;

public:
    int parse(const std::string &file_name, const std::string &content);

private:
    int lex(const std::string &file_name, const std::string &content);

    int parsing();

    int compile();

private:
    std::vector<std::tuple<std::string, int, int>> token_string(const std::string &str);

};
