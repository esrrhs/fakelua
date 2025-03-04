#include "compile/compiler.h"
#include "bison/parser.h"
#include "jit/preprocessor.h"
#include "util/exception.h"

namespace fakelua {

compile_result compiler::compile_file(fakelua_state_ptr sp, const std::string &file, compile_config cfg) {
    LOG_INFO("start compile_file {}", file);
    myflexer f;
    f.input_file(file);
    return compile(sp, f, cfg);
}

compile_result compiler::compile_string(fakelua_state_ptr sp, const std::string &str, compile_config cfg) {
    LOG_INFO("start compile_string");
    myflexer f;
    f.input_string(str);
    return compile(sp, f, cfg);
}

compile_result compiler::compile(fakelua_state_ptr sp, myflexer &f, compile_config cfg) {
    LOG_INFO("start compile {}", f.get_filename());

    compile_result ret;

    ret.file_name = f.get_filename();

    // generate the tree
    yy::parser parse(&f);
    auto code = parse.parse();
    LOG_INFO("compile ret {}", code);
    DEBUG_ASSERT(code == 0);
    ret.chunk = f.get_chunk();

    if (cfg.debug_mode) {
        // just walk the tree, do nothing
        walk_syntax_tree(ret.chunk, [](const syntax_tree_interface_ptr &ptr) {});
    }

    // compile tree
    if (!cfg.skip_jit) {
        // preprocess
        pre_processor pp;
        pp.process(sp, cfg, ret.file_name, ret.chunk);

        // compile
        gcc_jitter jitter;
        jitter.compile(sp, cfg, ret.file_name, ret.chunk);
    }

    return ret;
}

}// namespace fakelua
