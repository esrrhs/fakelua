#pragma once

#include "compile/compile_common.h"
#include "compile/syntax_tree.h"
#include "fakelua.h"

namespace fakelua {

// 从语法树生成C代码
class CGen {
public:
    // 构造函数
    explicit CGen(State *s);

    ~CGen() = default;

    void Generate(CompileResult &cr, const CompileConfig &cfg);

private:
    std::string Build(CompileResult &cr, const CompileConfig &cfg);

    [[noreturn]] void ThrowError(const std::string &msg, const SyntaxTreeInterfacePtr &ptr);

    std::string GenTab() const;

    void GenerateHeader();

    void GenerateGlobal(CompileResult &cr);

    void GenerateDecls(CompileResult &cr);

    std::string CompileFuncName(const SyntaxTreeInterfacePtr &ptr);

    std::vector<std::string> CompileParList(const SyntaxTreeInterfacePtr &parlist);

    void GenerateImpl(CompileResult &cr);

    void CompileStmtBlock(const SyntaxTreeInterfacePtr &block);

    void CompileStmt(const std::shared_ptr<SyntaxTreeInterface> &stmt);

    void CompileStmtReturn(const SyntaxTreeInterfacePtr &stmt);

    void CompileStmtLocalVar(const SyntaxTreeInterfacePtr &stmt);

    void CompileStmtAssign(const SyntaxTreeInterfacePtr &stmt);

    void CompileStmtFunctioncall(const SyntaxTreeInterfacePtr &shared);

    void CompileStmtLabel(const SyntaxTreeInterfacePtr &stmt);

    void CompileStmtWhile(const SyntaxTreeInterfacePtr &stmt);

    void CompileStmtRepeat(const SyntaxTreeInterfacePtr &stmt);

    void CompileStmtIf(const SyntaxTreeInterfacePtr &stmt);

    void CompileStmtBreak(const SyntaxTreeInterfacePtr &stmt);

    void CompileStmtForLoop(const SyntaxTreeInterfacePtr &stmt);

    void CompileStmtForIn(const SyntaxTreeInterfacePtr &stmt);

    std::string CompileExp(const SyntaxTreeInterfacePtr &exp);

    std::string CompilePrefixexp(const SyntaxTreeInterfacePtr &pe);

    std::string CompileVar(const SyntaxTreeInterfacePtr &v);

    std::string CompileFunctioncall(const SyntaxTreeInterfacePtr &functioncall);

    std::string CompileTableconstructor(const SyntaxTreeInterfacePtr &tc);

    std::string CompileBinop(const SyntaxTreeInterfacePtr &left, const SyntaxTreeInterfacePtr &right, const SyntaxTreeInterfacePtr &op);

    std::string CompileUnop(const SyntaxTreeInterfacePtr &right, const SyntaxTreeInterfacePtr &op);

private:
    State *s_;
    std::string file_name_;

    std::stringstream headers_;
    std::stringstream globals_;
    std::stringstream decls_;
    std::stringstream impls_;

    int cur_tab_ = 0;
    bool in_global_init_ = false;
    std::unordered_set<std::string> global_const_vars_;
    int tmp_var_counter_ = 0;

    // Function names (and arg counts) declared in the current compilation unit.
    // Populated during GenerateDecls so that CompileFunctioncall can distinguish
    // same-file direct calls from cross-file FakeluaCallByName calls.
    std::unordered_map<std::string, int> local_func_names_;
    // cur_output_ points to the current target stream (headers_, globals_, decls_, impls_, body_ss_, etc.).
    // All code emission should go through *cur_output_ for consistency.
    std::ostream *cur_output_ = nullptr;
    std::stringstream func_temp_decls_;
    // Buffers the function body during compilation (reused across functions, cleared each time).
    std::stringstream body_ss_;
};

}// namespace fakelua
