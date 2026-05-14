#pragma once

#include "compile/compile_common.h"
#include "compile/native_var_scope.h"
#include "compile/syntax_tree.h"
#include <ostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace fakelua {

class State;

// FuncBodyCompiler —— 函数体与表达式的代码生成器。
//
// 单一职责：将 AST 中的语句和表达式节点编译为 C 代码字符串。
// 自身维护：
//   • NativeVarScope       —— 追踪局部原生类型变量
//   • 特化上下文            —— 当前数学参数类型与快照
//   • 每次函数体的输出缓冲区 —— func_temp_decls_ + body_ss_
//
// 对以下数据持非拥有指针（生命周期由调用方 CGen 保证）：
//   • file_name_, local_func_names_, global_const_vars_, in_global_init_
//   • tmp_var_counter_ （跨函数持久累加）
//   • InferResult 中的特化快照与数学参数信息
class FuncBodyCompiler {
public:
    explicit FuncBodyCompiler(State *s);

    // 在 CGen::Build() 中调用一次，注入所有外部上下文指针。
    // 所有指针必须在 FuncBodyCompiler 使用期间持续有效。
    void SetContext(const std::string *file_name,
                    const std::unordered_map<std::string, int> *local_func_names,
                    const std::unordered_set<std::string> *global_const_vars,
                    bool *in_global_init,
                    int *tmp_var_counter,
                    const std::unordered_map<std::string, std::vector<int>> *math_param_positions,
                    const std::unordered_map<std::string, std::vector<EvalTypeSnapshot>> *specialization_snapshots,
                    const std::unordered_map<std::string, std::vector<InferredType>> *specialization_return_types,
                    const EvalTypeSnapshot *main_eval_types);

    // 编译一个表达式并返回其 C 表达式字符串。
    // 在 global-constant-init 上下文（*in_global_init_ == true）中，
    // 只有字面量表达式（nil / bool / number / string）是合法的，
    // 此时不会向任何输出流写入任何代码（无副作用）。
    [[nodiscard]] std::string CompileExp(const SyntaxTreeInterfacePtr &exp);

    // 编译完整的函数体并将结果（临时变量声明 + 函数体）追加到 out 流中。
    // spec_bitmask < 0  → 普通（非特化）模式。
    // spec_bitmask >= 0 → 数值特化模式：数学参数按 bitmask 映射到 int64_t / double。
    void CompileFuncBody(const std::string &func_name,
                         const std::vector<std::string> &func_params,
                         const SyntaxTreeInterfacePtr &func_block,
                         int spec_bitmask,
                         std::ostream &out);

    // 查询函数特化版本的实际返回类型（T_INT、T_FLOAT 或 T_DYNAMIC）。
    [[nodiscard]] InferredType GetSpecReturnType(const std::string &func_name, int bitmask) const;

    // 以带有源码位置的错误信息抛出代码生成异常。
    [[noreturn]] void ThrowError(const std::string &msg, const SyntaxTreeInterfacePtr &ptr) const;

private:
    // ---- 语句编译 ---------------------------------------------------------

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

    // ---- 表达式编译 -------------------------------------------------------

    std::string CompilePrefixexp(const SyntaxTreeInterfacePtr &pe);
    std::string CompileVar(const SyntaxTreeInterfacePtr &v);
    std::string CompileFunctioncall(const SyntaxTreeInterfacePtr &functioncall);
    std::string CompileTableconstructor(const SyntaxTreeInterfacePtr &tc);
    std::string CompileBinop(const SyntaxTreeInterfacePtr &left,
                              const SyntaxTreeInterfacePtr &right,
                              const SyntaxTreeInterfacePtr &op);
    std::string CompileUnop(const SyntaxTreeInterfacePtr &right,
                             const SyntaxTreeInterfacePtr &op);

    // ---- 数值特化编译 -----------------------------------------------------

    // 将表达式编译为原生 C 数值表达式字符串（int64_t / double）。
    // 失败时抛出异常，调用方应通过 TryCompileNativeExpr 捕获。
    std::string CompileNumericExp(const SyntaxTreeInterfacePtr &exp);

    // 尝试将表达式编译为原生数值字符串；失败时返回空字符串（不抛异常）。
    std::string TryCompileNativeExpr(const SyntaxTreeInterfacePtr &exp);

    // 尝试将比较/逻辑表达式编译为原生 C bool 表达式字符串；失败时返回空字符串。
    // 该方法是纯函数（不写入 cur_output_）。
    std::string TryCompileNativeBoolExpr(const SyntaxTreeInterfacePtr &exp);

    // 若 functioncall_node 指向一个返回原生数值类型的特化函数，
    // 则直接将调用编译为原生表达式，并返回变量名；否则返回空字符串。
    std::string TryCompileNativeSpecCallExpr(const SyntaxTreeInterfacePtr &functioncall_node);

    // ---- 类型推断帮助 -----------------------------------------------------

    // 在当前特化上下文中推断表达式的原生类型。
    [[nodiscard]] InferredType InferArgTypeForSpec(const SyntaxTreeInterfacePtr &exp) const;

    // 若 callee 是含数学参数的本地函数且所有数学参数类型已知，
    // 则填充 bitmask 并返回 true；否则返回 false。
    [[nodiscard]] bool TryInferMathCallBitmask(const std::string &callee_name,
                                               const std::vector<SyntaxTreeInterfacePtr> &raw_args,
                                               int &bitmask) const;

    // TryInferMathCallBitmask 的扩展版本，同时填充 spec_ret。
    [[nodiscard]] bool TryInferMathCallSpec(const std::string &callee_name,
                                            const std::vector<SyntaxTreeInterfacePtr> &raw_args,
                                            int &bitmask,
                                            InferredType &spec_ret) const;

    // 查询 AST 节点的推断类型：特化模式下优先使用快照，否则使用主推断结果。
    [[nodiscard]] InferredType LookupNodeType(SyntaxTreeInterface *node) const;

    // ---- 原生变量作用域 ---------------------------------------------------

    void EnterNativeVarScope() {
        native_var_scope_.Enter();
    }

    void ExitNativeVarScope() {
        native_var_scope_.Exit();
    }

    void DeclareNativeVar(const std::string &name, InferredType native_type) {
        native_var_scope_.Declare(name, native_type);
    }

    [[nodiscard]] bool IsTypedNativeVar(const std::string &name) const {
        return native_var_scope_.IsTyped(name);
    }

    [[nodiscard]] InferredType GetNativeVarType(const std::string &name) const {
        return native_var_scope_.GetType(name);
    }

    // ---- 输出帮助 ---------------------------------------------------------

    // 返回当前缩进字符串（每级 4 个空格）。
    [[nodiscard]] std::string GenTab() const;

    // ---- 外部上下文（非拥有指针，生命周期由 CGen 保证）-------------------

    State *s_ = nullptr;
    const std::string *file_name_ = nullptr;
    const std::unordered_map<std::string, int> *local_func_names_ = nullptr;
    const std::unordered_set<std::string> *global_const_vars_ = nullptr;
    bool *in_global_init_ = nullptr;
    int *tmp_var_counter_ = nullptr;

    // InferResult 数据（非拥有）
    const std::unordered_map<std::string, std::vector<int>> *math_param_positions_ = nullptr;
    const std::unordered_map<std::string, std::vector<EvalTypeSnapshot>> *specialization_snapshots_ = nullptr;
    const std::unordered_map<std::string, std::vector<InferredType>> *specialization_return_types_ = nullptr;
    const EvalTypeSnapshot *main_eval_types_ = nullptr;

    // ---- 每次函数体编译的状态（拥有）--------------------------------------

    NativeVarScope native_var_scope_;

    // 特化上下文（在 CompileFuncBody 开始和结束时清零）
    std::unordered_map<std::string, InferredType> spec_param_types_;
    std::string cur_spec_func_name_;
    int cur_spec_bitmask_ = -1;
    const EvalTypeSnapshot *cur_spec_snapshot_ = nullptr;

    // 输出缓冲区：CompileFuncBody 将函数体编译到 body_ss_，
    // 将临时变量声明写到 func_temp_decls_，最后一起追加到调用方提供的 out 流。
    std::ostream *cur_output_ = nullptr;
    std::stringstream func_temp_decls_;
    std::stringstream body_ss_;
    int cur_tab_ = 0;
};

}// namespace fakelua
