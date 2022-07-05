#include "compile/myflexer.h"
#include "glog/logging.h"

namespace fakelua {

myflexer::myflexer() {

}

myflexer::~myflexer() {

}

void myflexer::input_file(const std::string &file) {
    file_.open(file.c_str());
    if (file_.fail()) {
        throw std::runtime_error("open file failed");
    }
    filename_ = file;
    location_.initialize(&filename_);
    yy_flex_debug = trace_scanning_;
    switch_streams(&file_, nullptr);
}

void myflexer::input_string(const std::string &str) {
    if (str.empty()) {
        throw std::runtime_error("input string is empty");
    }
    filename_ = "<string>";
    location_.initialize(&filename_);
    yy_flex_debug = trace_scanning_;
    string_ = std::istringstream(str);
    switch_streams(&string_, nullptr);
}

}
