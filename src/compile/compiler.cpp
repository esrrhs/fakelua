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
// 旧接口：返回解析结果
// ─────────────────────────────────────────────────────────────────────────
ParseResult Compiler::CompileFile(const std::string &file, const CompileConfig &cfg) {
    LOG_INFO("start CompileFile {}", file);
    MyFlexer f;
    f.InputFile(file);
    return Compile(f, cfg, nullptr);
}

ParseResult Compiler::CompileString(const std::string &str, const CompileConfig &cfg) {
    LOG_INFO("start CompileString");
    MyFlexer f;
    f.InputString(str);
    return Compile(f, cfg, nullptr);
}

// ─────────────────────────────────────────────────────────────────────────
// 新接口：返回完整管线结果
// ─────────────────────────────────────────────────────────────────────────
CompileResult Compiler::CompileFileTo(const std::string &file, const CompileConfig &cfg) {
    LOG_INFO("start CompileFileTo {}", file);
    MyFlexer f;
    f.InputFile(file);
    CompileResult result;
    result.parse_result = Compile(f, cfg, &result);
    return result;
}

CompileResult Compiler::CompileStringTo(const std::string &str, const CompileConfig &cfg) {
    LOG_INFO("start CompileStringTo");
    MyFlexer f;
    f.InputString(str);
    CompileResult result;
    result.parse_result = Compile(f, cfg, &result);
    return result;
}

// ─────────────────────────────────────────────────────────────────────────
// 核心编译管线
//
// 管线阶段:
//   1. Lexer / Parser → AST
//   2. PreProcessor → 降糖后的 AST
//   3. SemanticAnalysis → AR (函数元、调用点)
//   4. TypeInferencer → IR (SSA + Shape + Escape)
//   5. CGen → GR (C 代码)
//   6. TCC / GCC JIT → 目标代码
//
// 当 out_result != nullptr 时，会把 IR / AR / GR 写入 *out_result。
// 当 out_result == nullptr 时，仅执行管线不产出结果（兼容旧接口）。
// ─────────────────────────────────────────────────────────────────────────
ParseResult Compiler::Compile(MyFlexer &f, const CompileConfig &cfg,
                               CompileResult *out_result) {
    LOG_INFO("start compile {}", f.GetFilename());

    ParseResult pr;
    pr.file_name = f.GetFilename();

    // ──[ 1 ] Lexer / Parser ───────────────────────────────────────────
    yy::parser parse(&f);
    auto code = parse.parse();
    LOG_INFO("compile ret {}", code);

    if (code != 0) {
        ThrowFakeluaException(std::format("Parse failed with code {}", code));
    }
    pr.chunk = f.GetChunk();

    if (cfg.debug_mode) {
        WalkSyntaxTree(pr.chunk, [](const SyntaxTreeInterfacePtr &) {});
    }

    // 复制 AST 到 CompileResult（保持 lifetime）
    if (out_result) {
        out_result->infer_result.chunk = pr.chunk;
    }

    // 快速退出：skip_jit 意味着不做任何编译/分析
    if (cfg.skip_jit) {
        return pr;
    }

    // ──[ 2 ] PreProcessor ─────────────────────────────────────────────
    // 降糖处理：local function → LocalFunctionDef,
    //          vararg → 显式 vararg 数组,
    //          多赋值 → 单条链表。
    PreProcessor pp(s_);
    pp.Process(pr, cfg);

    // ──[ 3 ] SemanticAnalysis ─────────────────────────────────────────
    // 校验：常量不重复定义、函数参数合规、变量不冲突。
    // 产出 AR：函数最大返回数、调用图等。
    SemanticAnalysis semantic_analysis(s_);
    AnalysisResult ar = semantic_analysis.Analyze(pr, cfg);
    if (out_result) {
        out_result->analysis_result = std::move(ar);
    }

    // ──[ 4 ] TypeInferencer (SSA/CFG/Shape) ─────────────────────────
    // 这是新管线的核心模块:
    //   - 对每个函数构建 CFG 和 SSA。
    //   - 工作表不动点迭代推导出每节点的静态类型。
    //   - Shape Meet / Widening 保证收敛。
    //   - 逃逸分析和函数摘要为 CGen 过程间接口提供便利。
    TypeInferencer inferencer;
    InferResult ir = inferencer.InferTypes(pr, cfg);

    // ──[ 5 ] CGen → C 代码 ──────────────────────────────────────────
    // CGen 综合 IR + AR，生成可直接编译的 C 源码。
    // 关键决策:
    //   - 逃逸 record → by-reference 或 heap 装箱。
    //   - 非逃逸 record → struct-of-fields 偏移访问。
    //   - 所有"未知"退化为 CVar + 运行时 hash。
    CGen cgen(s_);
    GenResult gr = cgen.Generate(pr, ir, ar, cfg);

    if (out_result) {
        out_result->infer_result = std::move(ir);
        out_result->gen_result = std::move(gr);
    }

    // ──[ 6 ] JIT 编译 ─────────────────────────────────────────────────
    // 把 C 源码编译为函数入口指针，注册到 State::vm 中。
    if (!cfg.disable_jit[JIT_TCC]) {
        TccJitter jitter(s_);
        jitter.Compile(pr, gr, cfg);
    }
    if (!cfg.disable_jit[JIT_GCC]) {
        GccJitter jitter(s_);
        jitter.Compile(pr, gr, cfg);
    }

    return pr;
}

}// namespace fakelua
