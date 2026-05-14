#include "compile/compiler.h"
#include "bison/parser.h"
#include "compile/c_gen.h"
#include "compile/preprocessor.h"
#include "compile/type_inferencer.h"
#include "jit/gcc_jit.h"
#include "jit/tcc_jit.h"
#include "state/state.h"

namespace fakelua {

// 构造函数
Compiler::Compiler(State *s) : s_(s) {
}

// 编译文件接口
ParseResult Compiler::CompileFile(const std::string &file, const CompileConfig &cfg) {
    LOG_INFO("start CompileFile {}", file);
    MyFlexer f;
    f.InputFile(file);
    return Compile(f, cfg);
}

// 编译字符串接口
ParseResult Compiler::CompileString(const std::string &str, const CompileConfig &cfg) {
    LOG_INFO("start CompileString");
    MyFlexer f;
    f.InputString(str);
    return Compile(f, cfg);
}

// 核心编译逻辑
ParseResult Compiler::Compile(MyFlexer &f, const CompileConfig &cfg) {
    LOG_INFO("start compile {}", f.GetFilename());

    ParseResult pr;
    pr.file_name = f.GetFilename();

    // 1. 生成语法树（AST）
    yy::parser parse(&f);
    auto code = parse.parse();
    LOG_INFO("compile ret {}", code);

    // 检查语法解析结果，解析失败必须抛出异常
    if (code != 0) {
        ThrowFakeluaException(std::format("Parse failed with code {}", code));
    }
    pr.chunk = f.GetChunk();

    // 调试模式下遍历语法树，可用于语法树检查
    if (cfg.debug_mode) {
        WalkSyntaxTree(pr.chunk, [](const SyntaxTreeInterfacePtr &ptr) {});
    }

    if (cfg.skip_jit) {
        return pr;
    }

    // 2. 预处理语法树
    PreProcessor pp(s_);
    pp.Process(pr, cfg);

    // 3. 类型推导（同时识别数学参数）
    TypeInferencer inferencer;
    InferResult ir = inferencer.Process(pr);

    // 4. 转译为C
    CGen cgen(s_);
    GenResult gr = cgen.Generate(pr, ir, cfg);

    // 5. JIT编译
    if (!cfg.disable_jit[JIT_TCC]) {
        TccJitter jitter(s_);
        jitter.Compile(pr, gr, cfg);
    }
    if (!cfg.disable_jit[JIT_GCC]) {
        GccJitter jitter(s_);
        jitter.Compile(pr, gr, cfg);
    }

    if (cfg.record_c_code) {
        last_recorded_c_code_ = gr.recorded_c_code;
    }

    return pr;
}

}// namespace fakelua
