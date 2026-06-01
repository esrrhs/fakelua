#pragma once

#include "compile/compile_common.h"
#include "compile/native_var_scope.h"
#include "fakelua.h"
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace fakelua {

class State;

// CGen —— 编译单元级别的 C 代码编排与函数体/表达式代码生成器。
class CGen {
public:
    explicit CGen(State *s);
    ~CGen() = default;

    // 核心代码生成入口
    GenResult Generate(const ParseResult &pr, const InferResult &ir, const CompileConfig &cfg);

private:
    // ==========================================
    // 第一部分：核心调度与编排
    // ==========================================
    GenResult Build(const ParseResult &pr, const InferResult &ir, const CompileConfig &cfg);
    void GenerateHeader();
    void GenerateGlobal(const SyntaxTreeInterfacePtr &chunk);
    void GenerateDecls(const SyntaxTreeInterfacePtr &chunk, GenResult &gr);
    void GenerateImpl(const SyntaxTreeInterfacePtr &chunk, GenResult &gr);
    
    std::string CompileFuncName(const SyntaxTreeInterfacePtr &ptr);
    std::vector<std::string> CompileParList(const SyntaxTreeInterfacePtr &parlist);
    void CompileFuncBody(const std::string &func_name,
                         const std::vector<std::string> &func_params,
                         const SyntaxTreeInterfacePtr &func_block,
                         int spec_bitmask,
                         std::ostream &out);
    void GenerateEntryDispatcher(const std::string &func_name,
                                 const std::vector<std::string> &func_params,
                                 const std::vector<int> &math_param_indices);
    [[nodiscard]] static bool BlockEndsWithReturn(const SyntaxTreeInterfacePtr &block);
    [[nodiscard]] InferredType GetSpecReturnType(const std::string &func_name, int bitmask) const;

    // ==========================================
    // 第二部分：语句编译
    // ==========================================
    void CompileStmtBlock(const SyntaxTreeInterfacePtr &block);
    void CompileStmt(const SyntaxTreeInterfacePtr &stmt);
    void CompileStmtReturn(const SyntaxTreeInterfacePtr &stmt);
    void CompileStmtLocalVar(const SyntaxTreeInterfacePtr &stmt);
    void CompileStmtAssign(const SyntaxTreeInterfacePtr &stmt);
    void CompileStmtFunctioncall(const SyntaxTreeInterfacePtr &stmt);
    void CompileStmtWhile(const SyntaxTreeInterfacePtr &stmt);
    void CompileStmtRepeat(const SyntaxTreeInterfacePtr &stmt);
    void CompileStmtIf(const SyntaxTreeInterfacePtr &stmt);
    void CompileStmtBreak(const SyntaxTreeInterfacePtr &stmt);
    void CompileStmtForLoop(const SyntaxTreeInterfacePtr &stmt);
    void CompileTypedIntForLoop(const std::shared_ptr<SyntaxTreeForLoop> &for_stmt);
    void CompileTypedFloatForLoop(const std::shared_ptr<SyntaxTreeForLoop> &for_stmt);
    void CompileDynamicForLoop(const std::shared_ptr<SyntaxTreeForLoop> &for_stmt);
    void CompileStmtForIn(const SyntaxTreeInterfacePtr &stmt);
    void CompileScopedBlock(const SyntaxTreeInterfacePtr &block);
    std::string CompileCondBoolExpr(const SyntaxTreeInterfacePtr &exp, const std::string &tmp_prefix);

    // ==========================================
    // 第三部分：表达式编译
    // ==========================================
    [[nodiscard]] std::string CompileExp(const SyntaxTreeInterfacePtr &exp);
    std::string CompilePrefixexp(const SyntaxTreeInterfacePtr &pe);
    std::string CompileVar(const SyntaxTreeInterfacePtr &v);
    std::string CompileFunctioncall(const SyntaxTreeInterfacePtr &functioncall);
    std::string CompileTableconstructor(const SyntaxTreeInterfacePtr &tc);
    std::string CompileBinop(const SyntaxTreeInterfacePtr &left,
                             const SyntaxTreeInterfacePtr &right,
                             const SyntaxTreeInterfacePtr &op);
    std::string CompileUnop(const SyntaxTreeInterfacePtr &right,
                            const SyntaxTreeInterfacePtr &op);

    // ==========================================
    // 第四部分：类型推断与原生优化辅助
    // ==========================================
    std::string CompileNumericExp(const SyntaxTreeInterfacePtr &exp);
    std::string TryCompileNativeExpr(const SyntaxTreeInterfacePtr &exp);
    std::string TryCompileNativeBoolExpr(const SyntaxTreeInterfacePtr &exp);
    std::string TryCompileNativeSpecCallExpr(const SyntaxTreeInterfacePtr &functioncall_node);
    
    // 原生算术、比较和一元操作编译的拆分辅助函数
    std::string CompileNativeArithBinop(const SyntaxTreeInterfacePtr &left,
                                        const SyntaxTreeInterfacePtr &right,
                                        BinOpKind op_kind,
                                        InferredType lt,
                                        InferredType rt);
    std::string CompileNativeCmpBinop(const SyntaxTreeInterfacePtr &left,
                                      const SyntaxTreeInterfacePtr &right,
                                      BinOpKind op_kind,
                                      InferredType lt,
                                      InferredType rt);
    std::string CompileNativeUnop(const SyntaxTreeInterfacePtr &right,
                                  UnOpKind op_kind,
                                  InferredType rt);

    [[nodiscard]] InferredType InferExpType(const SyntaxTreeInterfacePtr &exp) const;
    [[nodiscard]] InferredType InferArgTypeForSpec(const SyntaxTreeInterfacePtr &exp) const;
    [[nodiscard]] bool TryInferMathCallBitmask(const std::string &callee_name,
                                               const std::vector<SyntaxTreeInterfacePtr> &raw_args,
                                               int &bitmask) const;
    [[nodiscard]] bool TryInferMathCallSpec(const std::string &callee_name,
                                            const std::vector<SyntaxTreeInterfacePtr> &raw_args,
                                            int &bitmask,
                                            InferredType &spec_ret) const;
    [[nodiscard]] InferredType LookupNodeType(SyntaxTreeInterface *node) const;

    // 原生变量作用域管理
    void EnterNativeVarScope() { native_var_scope_.Enter(); }
    void ExitNativeVarScope() { native_var_scope_.Exit(); }
    void DeclareNativeVar(const std::string &name, InferredType native_type) {
        native_var_scope_.Declare(name, native_type);
    }
    [[nodiscard]] bool IsTypedNativeVar(const std::string &name) const { return native_var_scope_.IsTyped(name); }
    [[nodiscard]] InferredType GetNativeVarType(const std::string &name) const { return native_var_scope_.GetType(name); }

    // ==========================================
    // 第五部分：杂项辅助
    // ==========================================
    [[noreturn]] void ThrowError(const std::string &msg, const SyntaxTreeInterfacePtr &ptr);
    [[nodiscard]] std::string GenTab() const;

private:
    State *s_;
    std::string file_name_;

    std::stringstream headers_;
    std::stringstream globals_;
    std::stringstream decls_;
    std::stringstream impls_;

    bool in_global_init_ = false;
    std::unordered_map<std::string, InferredType> global_const_vars_;
    int tmp_var_counter_ = 0;

    std::unordered_map<std::string, int> local_func_names_;

    std::ostream *cur_output_ = nullptr;

    std::unordered_map<std::string, std::vector<int>> math_param_positions_;
    std::unordered_map<std::string, std::vector<EvalTypeSnapshot>> specialization_snapshots_;
    std::unordered_map<std::string, std::vector<InferredType>> specialization_return_types_;
    EvalTypeSnapshot main_eval_types_;

    NativeVarScope native_var_scope_;
    std::unordered_map<std::string, InferredType> spec_param_types_;
    std::string cur_spec_func_name_;
    int cur_spec_bitmask_ = -1;
    const EvalTypeSnapshot *cur_spec_snapshot_ = nullptr;

    std::stringstream func_temp_decls_;
    std::stringstream body_ss_;
    int cur_tab_ = 0;
};

}// namespace fakelua
