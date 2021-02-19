#include "tester.h"
#include "parser.h"

std::vector<std::tuple<std::string, int, int>> tester::token_string(const std::string &str) {
    parser p;
    return p.token_string(str);
}
