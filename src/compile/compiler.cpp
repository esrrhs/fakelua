#include "compile/compiler.h"
#include "bison/parser.h"

namespace fakelua {

// 构造函数
Compiler::Compiler(State *s) : s_(s), pp_(s), jitter_(s) {
}

// 编译文件接口
CompileResult Compiler::CompileFile(const std::string &file, const CompileConfig &cfg) {
    LOG_INFO("start CompileFile {}", file);
    MyFlexer f;
    f.InputFile(file);
    return Compile(f, cfg);
}

// 编译字符串接口
CompileResult Compiler::CompileString(const std::string &str, const CompileConfig &cfg) {
    LOG_INFO("start CompileString");
    MyFlexer f;
    f.InputString(str);
    return Compile(f, cfg);
}

// 核心编译逻辑
CompileResult Compiler::Compile(MyFlexer &f, const CompileConfig &cfg) {
    LOG_INFO("start compile {}", f.GetFilename());

    CompileResult ret;
    ret.fileName = f.GetFilename();

    // 1. 生成语法树（AST）
    yy::parser parse(&f);
    auto code = parse.parse();
    LOG_INFO("compile ret {}", code);

    // 断言语法解析无误
    DEBUG_ASSERT(code == 0);
    ret.chunk = f.GetChunk();

    // 调试模式下遍历语法树，可用于语法树检查
    if (cfg.debug_mode) {
        WalkSyntaxTree(ret.chunk, [](const SyntaxTreeInterfacePtr &ptr) {});
    }

    // 2. 编译语法树
    if (!cfg.skip_jit) {
        // 预处理语法树
        pp_.Process(cfg, ret.fileName, ret.chunk);

        // 编译为原生机器码
        jitter_.Compile(cfg, ret.fileName, ret.chunk);
    }

    return ret;
}

}// namespace fakelua
