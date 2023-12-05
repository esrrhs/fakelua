#include "compile/compiler.h"
#include "bison/parser.h"

namespace fakelua {

compile_result compiler::compile_file(fakelua_state_ptr sp, const std::string &file, compile_config cfg) {
    LOG(INFO) << "start compile_file " << file;
    myflexer f;
    f.input_file(file);
    return compile(sp, f, cfg);
}

compile_result compiler::compile_string(fakelua_state_ptr sp, const std::string &str, compile_config cfg) {
    LOG(INFO) << "start compile_string";
    myflexer f;
    f.input_string(str);
    return compile(sp, f, cfg);
}

compile_result compiler::compile(fakelua_state_ptr sp, myflexer &f, compile_config cfg) {
    compile_result ret;

    ret.file_name = f.get_filename();

    // compile chunk
    yy::parser parse(&f);
    auto code = parse.parse();
    LOG(INFO) << "compile ret " << code;
    if (code) {
        throw std::runtime_error("compile failed");
    }
    ret.chunk = f.get_chunk();

    // compile interpreter
    if (!cfg.skip_jit) {
        ret.jitter = std::make_shared<gcc_jitter>();
        ret.jitter->compile(sp, ret.file_name, ret.chunk);
    }

    return ret;
}

}// namespace fakelua
