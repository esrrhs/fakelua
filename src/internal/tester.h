#pragma once

#include "types.h"
#include "err.h"

class tester {
public:
    std::tuple<err, std::vector<std::tuple<std::string, int, int>>> split_string(const std::string &file_name,
                                                                                 const std::string &str);

    std::string change_comment_to_space(std::string str);

    std::string replace_multi_comment(std::string str);

    std::string replace_comment(std::string str);
};
