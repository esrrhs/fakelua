#include "compile/compiler.h"
#include "bison/parser.h"

namespace fakelua {

compiler::compiler() {
}

compiler::~compiler() {

}

void compiler::compile_file(const std::string &file) {
    LOG(INFO) << "start compile_file " << file;
    myflexer f;
    f.input_file(file);
    yy::parser parse(&f);
    auto ret = parse.parse();
    LOG(INFO) << "compile_file " << file << " ret " << ret;
    if (ret) {
        throw std::runtime_error("compile_file failed");
    }
}

void compiler::compile_string(const std::string &str) {
    LOG(INFO) << "start compile_string";
    myflexer f;
    f.input_string(str);
    yy::parser parse(&f);
    auto ret = parse.parse();
    LOG(INFO) << "compile_string ret " << ret;
    if (ret) {
        throw std::runtime_error("compile_string failed");
    }
}

}
