#include "compile/compiler.h"
#include "bison/parser.h"
#include "compile/c_gen.h"
#include "compile/preprocessor.h"
#include "compile/semantic_analysis.h"
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
    LOG_DEBUG("engine", "start CompileFile {}", file);
    MyFlexer f;
    f.InputFile(file);
    return Compile(f, cfg);
}

// 编译字符串接口
ParseResult Compiler::CompileString(const std::string &str, const CompileConfig &cfg) {
    LOG_DEBUG("engine", "start CompileString");
    MyFlexer f;
    f.InputString(str);
    return Compile(f, cfg);
}

// 核心编译逻辑
ParseResult Compiler::Compile(MyFlexer &f, const CompileConfig &cfg) {
    LOG_DEBUG("engine", "start compile {}", f.GetFilename());

    ParseResult pr;
    pr.file_name = f.GetFilename();

    // 1. 生成语法树（AST）
    LOG_DEBUG("engine", "step 1: parsing AST");
    yy::parser parse(&f);
    auto code = parse.parse();
    LOG_DEBUG("engine", "compile ret {}", code);

    // 检查语法解析结果，解析失败必须抛出异常
    if (code != 0) {
        ThrowFakeluaException(std::format("Parse failed with code {}", code));
    }
    pr.chunk = f.GetChunk();
    LOG_DEBUG("engine", "AST generated, top-level stmts: {}",
              pr.chunk && pr.chunk->Type() == SyntaxTreeType::Block
                  ? std::dynamic_pointer_cast<SyntaxTreeBlock>(pr.chunk)->Stmts().size() : 0);

    // 调试模式下遍历语法树，可用于语法树检查
    if (cfg.debug_mode) {
        LOG_DEBUG("engine", "debug_mode: walking syntax tree");
        WalkSyntaxTree(pr.chunk, [](const SyntaxTreeInterfacePtr &ptr) {});
    }

    if (cfg.skip_jit) {
        LOG_DEBUG("engine", "skip_jit enabled, returning after parsing");
        return pr;
    }

    // 2. 文件级语句校验：必须在预处理改写语法树之前，此时顶层语句还保持源码里的原始形态
    LOG_DEBUG("engine", "step 2: semantic analysis - file level stmt check");
    SemanticAnalysis semantic_analysis(s_);
    semantic_analysis.CheckFileLevelStmts(pr);

    // 3. 预处理语法树
    LOG_DEBUG("engine", "step 3: preprocessing");
    PreProcessor pp(s_);
    pp.Process(pr, cfg);

    // 4. 语义与控制流分析
    LOG_DEBUG("engine", "step 4: semantic analysis");
    AnalysisResult ar = semantic_analysis.Analyze(pr, cfg);

    // 5. 类型推导（同时识别数学参数）
    LOG_DEBUG("engine", "step 5: type inference");
    TypeInferencer inferencer;
    InferResult ir = inferencer.InferTypes(pr, cfg);

    // 6. 转译为C
    LOG_DEBUG("engine", "step 6: C code generation");
    CGen cgen(s_);
    GenResult gr = cgen.Generate(pr, ir, ar, cfg);

    // 7. JIT编译
    LOG_DEBUG("engine", "step 7: JIT compilation (TCC={}, GCC={})", !cfg.disable_jit[JIT_TCC], !cfg.disable_jit[JIT_GCC]);
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
        LOG_DEBUG("engine", "record_c_code: {} bytes", gr.recorded_c_code.size());
    }

    LOG_DEBUG("engine", "compile finished: {}, functions: {}", pr.file_name, gr.function_names.size());
    return pr;
}

}// namespace fakelua
