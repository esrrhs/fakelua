#include "compile/compiler.h"
#include "bison/parser.h"
#include "compile/c_gen.h"
#include "compile/preprocessor.h"
#include "jit/gcc_jit.h"
#include "jit/tcc_jit.h"
#include "state/state.h"

namespace fakelua {

// 构造函数
Compiler::Compiler(State *s) : s_(s) {
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
    ret.file_name = f.GetFilename();

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

    if (cfg.skip_jit) {
        return ret;
    }

    // 2. 预处理语法树
    PreProcessor pp(s_);
    pp.Process(ret, cfg);

    // 3. 转译为C
    CGen cgen(s_);
    cgen.Generate(ret, cfg);

    // 4. JIT编译
    if (cfg.tcc_jit) {
        if (!s_->GetStateConfig().open_tcc_jit) {
            ThrowFakeluaException("TCC JIT is not enabled in StateConfig");
        }
        TccJitter jitter(s_);
        jitter.Compile(ret, cfg);
    }

    return ret;
}

}// namespace fakelua
