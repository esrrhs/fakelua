#include "compile/compiler.h"
#include "bison/parser.h"

namespace fakelua {

compiler::compiler() {
}

compiler::~compiler() {
}

compile_result compiler::compile_file(const std::string &file, compile_config cfg) {
    LOG(INFO) << "start compile_file " << file;
    myflexer f;
    f.input_file(file);
    return compile(f, cfg);
}

compile_result compiler::compile_string(const std::string &str, compile_config cfg) {
    LOG(INFO) << "start compile_string";
    myflexer f;
    f.input_string(str);
    return compile(f, cfg);
}

compile_result compiler::compile(myflexer &f, compile_config cfg) {
    compile_result ret;

    ret.chunk_name = f.get_filename();

    // compile chunk
    yy::parser parse(&f);
    auto code = parse.parse();
    LOG(INFO) << "compile ret " << code;
    if (code) {
        throw std::runtime_error("compile failed");
    }
    ret.chunk = f.get_chunk();

    // compile interpreter
    if (!cfg.skip_interpreter) {
        ret.interpreter = std::make_shared<interpreter>();
        ret.interpreter->compile(ret.chunk);
    }

    return ret;
}

}// namespace fakelua
