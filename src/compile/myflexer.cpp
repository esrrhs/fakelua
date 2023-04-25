#include "compile/myflexer.h"
#include "glog/logging.h"

namespace fakelua {

myflexer::myflexer() {

}

myflexer::~myflexer() {

}

void myflexer::input_file(const std::string &file) {
    file_.open(file.c_str(), std::ios::binary);
    if (file_.fail()) {
        throw std::runtime_error("open file failed");
    }
    filename_ = file;
    location_.initialize(&filename_);
    set_debug(0);
    switch_streams(&file_, nullptr);
}

void myflexer::input_string(const std::string &str) {
    if (str.empty()) {
        throw std::runtime_error("input string is empty");
    }
    filename_ = "<string>";
    location_.initialize(&filename_);
    set_debug(0);
    string_ = std::istringstream(str, std::ios::binary);
    switch_streams(&string_, nullptr);
}

void myflexer::set_chunk(const syntax_tree_interface_ptr &chunk) {
    chunk_ = chunk;
}

syntax_tree_interface_ptr myflexer::get_chunk() const {
    return chunk_;
}

std::string myflexer::remove_quotes(const std::string &str) {
    if (str.size() < 2) {
        throw std::runtime_error("remove quotes but input string is empty");
    }
    if (str[0] == '\'' && str[str.size() - 1] == '\'') {
        return str.substr(1, str.size() - 2);
    } else if (str[0] == '"' && str[str.size() - 1] == '"') {
        return str.substr(1, str.size() - 2);
    } else {
        if (str.size() < 4) {
            throw std::runtime_error("remove quotes but input string is empty");
        }
        if (str[0] == '[' && str[1] == '[' && str[str.size() - 1] == ']' && str[str.size() - 2] == ']') {
            return str.substr(2, str.size() - 4);
        } else {
            throw std::runtime_error("remove quotes but input string is not valid");
        }
    }
}

}
