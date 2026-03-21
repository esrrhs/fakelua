#include "compile/Compiler.h"
#include "bison/parser.h"
#include "jit/preprocessor.h"
#include "util/exception.h"

namespace fakelua {

CompileResult Compiler::CompileFile(const FakeluaStatePtr &sp, const std::string &file, const CompileConfig &cfg) {
    LOG_INFO("start CompileFile {}", file);
    MyFlexer f;
    f.InputFile(file);
    return Compile(sp, f, cfg);
}

CompileResult Compiler::CompileString(const FakeluaStatePtr &sp, const std::string &str, const CompileConfig &cfg) {
    LOG_INFO("start CompileString");
    MyFlexer f;
    f.InputString(str);
    return Compile(sp, f, cfg);
}

CompileResult Compiler::Compile(const FakeluaStatePtr &sp, MyFlexer &f, const CompileConfig &cfg) {
    LOG_INFO("start compile {}", f.GetFilename());

    CompileResult ret;

    ret.fileName = f.GetFilename();

    // generate the tree
    yy::parser parse(&f);
    auto code = parse.parse();
    LOG_INFO("compile ret {}", code);
    DEBUG_ASSERT(code == 0);
    ret.chunk = f.GetChunk();

    if (cfg.debug_mode) {
        // just walk the tree, do nothing
        WalkSyntaxTree(ret.chunk, [](const SyntaxTreeInterfacePtr &ptr) {});
    }

    // compile tree
    if (!cfg.skip_jit) {
        // preprocess
        PreProcessor pp;
        pp.Process(sp, cfg, ret.fileName, ret.chunk);

        // compile
        GccJitter jitter;
        jitter.Compile(sp, cfg, ret.fileName, ret.chunk);
    }

    return ret;
}

}// namespace fakelua
