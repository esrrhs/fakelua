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

    std::string CompileNumericExp(const SyntaxTreeInterfacePtr &exp);

    std::string BoxNativeValue(const std::string &expr, InferredType type) const;

    void EnterNativeVarScope();

    void ExitNativeVarScope();

    void DeclareNativeVar(const std::string &name, bool typed_native);

    [[nodiscard]] bool IsTypedNativeVar(const std::string &name) const;

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

    // 当前编译单元中声明的函数名（及参数数量）。
    // 在 GenerateDecls 期间填充，以便 CompileFunctioncall 能够区分
    // 同文件的直接调用和跨文件的 FakeluaCallByName 调用。
    std::unordered_map<std::string, int> local_func_names_;
    // cur_output_ 指向当前目标流（headers_、globals_、decls_、impls_、body_ss_ 等）。
    // 所有代码生成应通过 *cur_output_ 进行以保持一致性。
    std::ostream *cur_output_ = nullptr;
    std::stringstream func_temp_decls_;
    // 在编译期间缓冲函数体（跨函数重用，每次清空）。
    std::stringstream body_ss_;
    // 作用域化的本地变量声明信息：name -> 是否声明为原生数值类型
    // (int64_t/double)。反向查找首个命中的声明即可处理遮蔽关系：
    // - true  : 当前可见绑定是原生类型变量
    // - false : 当前可见绑定是 CVar（即使外层同名变量是原生类型）
    std::vector<std::unordered_map<std::string, bool>> native_var_scopes_;
};

}// namespace fakelua
