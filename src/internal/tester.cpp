#include "tester.h"
#include "parser.h"

std::tuple<err, std::vector<std::tuple<std::string, int, int>>> tester::split_string(const std::string &file_name,
                                                                                     const std::string &str) {
    parser p;
    return p.split_string(file_name, str);
}

std::string tester::change_comment_to_space(std::string str) {
    parser p;
    return p.change_comment_to_space(str);
}

std::string tester::replace_multi_comment(std::string str) {
    parser p;
    return p.replace_multi_comment(str);
}

std::string tester::replace_comment(std::string str) {
    parser p;
    return p.replace_comment(str);
}
