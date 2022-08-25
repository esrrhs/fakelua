#include "compile/compiler.h"
#include "bison/parser.h"

namespace fakelua {

compiler::compiler() {
}

compiler::~compiler() {

}

compile_result compiler::compile_file(const std::string &file) {
    LOG(INFO) << "start compile_file " << file;
    myflexer f;
    f.input_file(file);
    yy::parser parse(&f);
    auto code = parse.parse();
    LOG(INFO) << "compile_file " << file << " ret " << code;
    if (code) {
        throw std::runtime_error("compile_file failed");
    }
    compile_result ret;
    ret.chunk = f.get_chunk();
    return ret;
}

compile_result compiler::compile_string(const std::string &str) {
    LOG(INFO) << "start compile_string";
    myflexer f;
    f.input_string(str);
    yy::parser parse(&f);
    auto code = parse.parse();
    LOG(INFO) << "compile_string ret " << code;
    if (code) {
        throw std::runtime_error("compile_string failed");
    }
    compile_result ret;
    ret.chunk = f.get_chunk();
    return ret;
}

}
