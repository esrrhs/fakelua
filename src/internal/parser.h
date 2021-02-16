#pragma once

#include "types.h"

class parser {
public:
    parser() {}

    parser(const parser &) = delete;

    parser &operator=(const parser &) = delete;

public:
    int parse(const std::string & file_name, const std::string & content);

private:
    int lex(const std::string & file_name, const std::string & content);
    int parsing();
    int compile();

};
