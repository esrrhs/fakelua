#include "compile/compiler.h"
#include "bison/parser.h"

namespace fakelua {

compiler::compiler() {
}

compiler::~compiler() {

}

void compiler::compile_file(const std::string &file) {
    myflexer f;
    f.input_file(file);
    yy::parser parse(&f);
}

void compiler::compile_string(const std::string &str) {
    myflexer f;
    f.input_string(str);
    yy::parser parse(&f);
}

}
