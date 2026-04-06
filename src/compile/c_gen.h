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

    std::string GenTab();

    void GenerateHeader();

    void GenerateGlobal();

    void GenerateDecls(CompileResult &cr);

    std::string CompileFuncName(const SyntaxTreeInterfacePtr &ptr);

    std::vector<std::string> CompileParList(const SyntaxTreeInterfacePtr &parlist);

    void GenerateImpl(CompileResult &cr);

    void CompileStmtBlock(const SyntaxTreeInterfacePtr &block);

    void CompileStmt(const std::shared_ptr<SyntaxTreeInterface> &stmt);

    void CompileStmtReturn(const SyntaxTreeInterfacePtr &stmt);

    void CompileStmtLocalVar(const SyntaxTreeInterfacePtr &stmt);

    void CompileStmtAssign(const SyntaxTreeInterfacePtr &stmt);

    void CompileStmtFunctioncall(SyntaxTreeInterfacePtr shared);

    void CompileStmtLabel(const SyntaxTreeInterfacePtr &stmt);

    void CompileStmtWhile(const SyntaxTreeInterfacePtr &stmt);

    void CompileStmtRepeat(SyntaxTreeInterfacePtr stmt);

    void CompileStmtIf(SyntaxTreeInterfacePtr stmt);

    void CompileStmtBreak(SyntaxTreeInterfacePtr stmt);

    void CompileStmtForLoop(SyntaxTreeInterfacePtr stmt);

    void CompileStmtForIn(const SyntaxTreeInterfacePtr &stmt);

    std::string CompileExp(const SyntaxTreeInterfacePtr &exp);

    std::string CompilePrefixexp(const SyntaxTreeInterfacePtr &pe);

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
    std::unordered_set<std::string> global_const_vars_;
};

}// namespace fakelua
