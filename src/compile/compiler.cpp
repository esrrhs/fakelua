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

// ─────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────
Compiler::Compiler(State *s) : s_(s) {
}

// ─────────────────────────────────────────────────────────────────────────
// 编译 Lua 文件 → 返回完整管线结果
// ─────────────────────────────────────────────────────────────────────────
CompileResult Compiler::CompileFile(const std::string &file, const CompileConfig &cfg) {
    LOG_INFO("start CompileFile {}", file);
    MyFlexer f;
    f.InputFile(file);
    return Build(f, cfg);
}

// ─────────────────────────────────────────────────────────────────────────
// 编译 Lua 字符串 → 返回完整管线结果
// ─────────────────────────────────────────────────────────────────────────
CompileResult Compiler::CompileString(const std::string &str, const CompileConfig &cfg) {
    LOG_INFO("start CompileString");
    MyFlexer f;
    f.InputString(str);
    return Build(f, cfg);
}

// ─────────────────────────────────────────────────────────────────────────
// 核心编译管线
//
// 管线阶段:
//   [1] Lexer / Parser → AST
//   [2] PreProcessor → 降糖后的 AST
//   [3] SemanticAnalysis → AR (函数元、调用点)
//   [4] TypeInferencer → IR (SSA + Shape + Escape)
//   [5] CGen → GR (C 代码)
//   [6] TCC / GCC JIT → 目标代码
//
// 产物全部写入返回的 CompileResult。
// ─────────────────────────────────────────────────────────────────────────
CompileResult Compiler::Build(MyFlexer &f, const CompileConfig &cfg) {
    LOG_INFO("start build {}", f.GetFilename());

    CompileResult result;

    // ──[ 1 ] Lexer / Parser ───────────────────────────────────────────
    yy::parser parse(&f);
    auto code = parse.parse();
    LOG_INFO("build ret {}", code);

    if (code != 0) {
        ThrowFakeluaException(std::format("Parse failed with code {}", code));
    }
    result.parse_result.file_name = f.GetFilename();
    result.parse_result.chunk = f.GetChunk();
    // AST 生命周期写入 InferResult
    result.infer_result.chunk = result.parse_result.chunk;

    if (cfg.debug_mode) {
        WalkSyntaxTree(result.parse_result.chunk, [](const SyntaxTreeInterfacePtr &) {});
    }

    // 快速退出：skip_jit 意味着不做任何编译/分析
    if (cfg.skip_jit) {
        return result;
    }

    // ──[ 2 ] PreProcessor ─────────────────────────────────────────────
    // 降糖处理：local function → LocalFunctionDef,
    //          vararg → 显式 vararg 数组,
    //          多赋值 → 单条链表。
    PreProcessor pp(s_);
    pp.Process(result.parse_result, cfg);

    // ──[ 3 ] SemanticAnalysis ─────────────────────────────────────────
    // 校验：常量不重复定义、函数参数合规、变量不冲突。
    // 产出 AR：函数最大返回数、调用图等。
    SemanticAnalysis semantic_analysis(s_);
    result.analysis_result = semantic_analysis.Analyze(result.parse_result, cfg);

    // ──[ 4 ] TypeInferencer (SSA/CFG/Shape) ─────────────────────────
    // 对每个函数构建 CFG 和 SSA。
    // 工作表不动点迭代推导出每节点的静态类型。
    // Shape Meet / Widening 保证收敛。
    // 逃逸分析和函数摘要为 CGen 过程间接口提供便利。
    TypeInferencer inferencer;
    result.infer_result = inferencer.InferTypes(result.parse_result, cfg);
    // AST 生命周期已被 InferTypes 设置

    // ──[ 5 ] CGen → C 代码 ──────────────────────────────────────────
    // 综合 IR + AR，生成可直接编译的 C 源码。
    CGen cgen(s_);
    result.gen_result = cgen.Generate(result.parse_result, result.infer_result, result.analysis_result, cfg);

    // ──[ 6 ] JIT 编译 ─────────────────────────────────────────────────
    // 把 C 源码编译为函数入口指针，注册到 State::vm 中。
    if (!cfg.disable_jit[JIT_TCC]) {
        TccJitter jitter(s_);
        jitter.Compile(result.parse_result, result.gen_result, cfg);
    }
    if (!cfg.disable_jit[JIT_GCC]) {
        GccJitter jitter(s_);
        jitter.Compile(result.parse_result, result.gen_result, cfg);
    }

    return result;
}

}// namespace fakelua
