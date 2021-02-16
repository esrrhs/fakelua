#include "types.h"

std::string change_comment_to_space(std::string str) {
    std::replace_if(str.begin(), str.end(), [](char x) { return x != '\n'; }, ' ');
    return str;
}

std::string replace_multi_comment(std::string str) {
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

std::string replace_comment(std::string str) {
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
