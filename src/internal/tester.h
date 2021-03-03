#pragma once

#include "types.h"

class tester {
public:
    std::vector<std::tuple<std::string, int, int>> token_string(const std::string &str);
    std::string change_comment_to_space(std::string str);
    std::string replace_multi_comment(std::string str);
    std::string replace_comment(std::string str);
};
