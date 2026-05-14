#pragma once

#include "compile/compile_common.h"
#include "compile/syntax_tree.h"
#include "compile/type_inferencer.h"
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

    // 将函数体编译到当前输出流。
    // `spec_bitmask` >= 0 时启用特化模式：对数学参数 i，
    // MathParamKindOf(spec_bitmask, i) == kMathParamInt → int64_t，
    //                                    == kMathParamFloat → double。
    // `spec_bitmask` < 0 表示普通（非特化）模式。
    void CompileFuncBody(const std::string &func_name,
                          const std::vector<std::string> &func_params,
                          const SyntaxTreeInterfacePtr &func_block,
                          int spec_bitmask);

    // 为含有数学参数的函数生成入口分发器。
    void GenerateEntryDispatcher(const std::string &func_name,
                                  const std::vector<std::string> &func_params,
                                  const std::vector<int> &math_param_indices);

    // 根据基础名称和位掩码返回特化函数名。
    // 例如 SpecFuncName("fib", {0}, 0) -> "fib_0"
    //       SpecFuncName("test", {1,4}, 2) -> "test_0_1"
    static std::string SpecFuncName(const std::string &base_name,
                                     const std::vector<int> &math_param_indices, int bitmask);

    // 在当前特化上下文中推断表达式的原生类型。
    // 返回 T_INT、T_FLOAT 或 T_DYNAMIC。
    [[nodiscard]] InferredType InferArgTypeForSpec(const SyntaxTreeInterfacePtr &exp) const;

    // 尝试将表达式编译为原始数值（int64_t / double 表达式）。
    // 返回 C 表达式字符串，失败时返回空字符串。
    // 该函数是纯函数式的——不向 cur_output_ 写入任何代码。
    std::string TryCompileNativeExpr(const SyntaxTreeInterfacePtr &exp);

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

    void CompileTypedIntForLoop(const std::shared_ptr<SyntaxTreeForLoop> &for_stmt);

    void CompileTypedFloatForLoop(const std::shared_ptr<SyntaxTreeForLoop> &for_stmt);

    void CompileDynamicForLoop(const std::shared_ptr<SyntaxTreeForLoop> &for_stmt);

    void CompileStmtForIn(const SyntaxTreeInterfacePtr &stmt);

    std::string CompileExp(const SyntaxTreeInterfacePtr &exp);

    std::string CompilePrefixexp(const SyntaxTreeInterfacePtr &pe);

    std::string CompileVar(const SyntaxTreeInterfacePtr &v);

    std::string CompileFunctioncall(const SyntaxTreeInterfacePtr &functioncall);

    std::string CompileTableconstructor(const SyntaxTreeInterfacePtr &tc);

    std::string CompileBinop(const SyntaxTreeInterfacePtr &left, const SyntaxTreeInterfacePtr &right, const SyntaxTreeInterfacePtr &op);

    std::string CompileUnop(const SyntaxTreeInterfacePtr &right, const SyntaxTreeInterfacePtr &op);

    std::string CompileNumericExp(const SyntaxTreeInterfacePtr &exp);

    // 尝试将比较表达式编译为原生 C bool 表达式（不经过 CVar/OpXx/IsTrue 机制）。
    // 当两个操作数均为静态已知的数值类型时返回非空字符串，否则返回空字符串。
    // 该方法是纯函数（不写入 cur_output_）。
    std::string TryCompileNativeBoolExpr(const SyntaxTreeInterfacePtr &exp);

    std::string BoxNativeValue(const std::string &expr, InferredType type) const;

    // 返回特化函数返回值对应的 C 类型名称字符串（"int64_t"、"double" 或 "CVar"）。
    [[nodiscard]] static const char *SpecReturnCTypeName(InferredType ret_type);

    // 检查 block 的最后一条语句是否为 return。
    [[nodiscard]] static bool BlockEndsWithReturn(const SyntaxTreeInterfacePtr &block);

    // 查询函数特化版本的实际返回类型（T_INT、T_FLOAT 或 T_DYNAMIC）。
    // 由 DiscoverMathParams 的不动点迭代填充，不存在时返回 T_DYNAMIC。
    [[nodiscard]] InferredType GetSpecReturnType(const std::string &func_name, int bitmask) const;

    // 若 functioncall_node 指向一个返回原生数值类型的特化函数，
    // 则直接将调用编译为原生表达式（int64_t / double 临时变量），并返回变量名。
    // 避免调用方在 CompileNumericExp 中装箱再拆箱。
    // 不适用时（被调函数 CVar 返回或无法推断）返回空字符串。
    std::string TryCompileNativeSpecCallExpr(const SyntaxTreeInterfacePtr &functioncall_node);

    std::string CompileCondBoolExpr(const SyntaxTreeInterfacePtr &exp, const std::string &tmp_prefix);

    void CompileScopedBlock(const SyntaxTreeInterfacePtr &block);

    bool TryInferMathCallBitmask(const std::string &callee_name,
                                 const std::vector<SyntaxTreeInterfacePtr> &raw_args,
                                 int &bitmask) const;

    bool TryInferMathCallSpec(const std::string &callee_name,
                              const std::vector<SyntaxTreeInterfacePtr> &raw_args,
                              int &bitmask,
                              InferredType &spec_ret) const;

    void EnterNativeVarScope();

    void ExitNativeVarScope();

    void DeclareNativeVar(const std::string &name, InferredType native_type);

    [[nodiscard]] bool IsTypedNativeVar(const std::string &name) const;

    // 返回变量在当前作用域中声明的原生 C 类型：
    //   T_INT   → int64_t
    //   T_FLOAT → double
    //   T_DYNAMIC → CVar（或不在当前作用域中）
    [[nodiscard]] InferredType GetNativeVarType(const std::string &name) const;

    // 查询 AST 节点的推断类型。特化模式下优先使用当前特化快照，
    // 否则回退到全局主推断结果（main_eval_types_）。未找到时返回 T_UNKNOWN。
    [[nodiscard]] InferredType LookupNodeType(SyntaxTreeInterface *node) const;

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
    // 作用域化的本地变量声明信息：name -> 声明的原生 C 类型。
    //   T_INT    : int64_t 变量
    //   T_FLOAT  : double 变量
    //   T_DYNAMIC: CVar（即使外层同名变量是原生类型，遮蔽后仍为 CVar）
    std::vector<std::unordered_map<std::string, InferredType>> native_var_scopes_;

    // 来自 TypeInferencer::DiscoverMathParams 的数学参数分析结果。
    // func_name -> 按升序排列的数学参数索引列表。
    std::unordered_map<std::string, std::vector<int>> math_param_positions_;

    // 所有含数学参数的函数的按位掩码分组的 AST 类型快照。
    // 指向 CompileResult 中在 Build() 调用期间始终有效的数据。
    const std::unordered_map<std::string, std::vector<EvalTypeSnapshot>> *specialization_snapshots_ = nullptr;

    // 每个数学参数函数每个 bitmask 特化版本的实际返回类型。
    // 由 TypeInferencer::DiscoverMathParams 通过不动点迭代填充；
    // 指向 CompileResult 中在 Build() 调用期间始终有效的数据。
    const std::unordered_map<std::string, std::vector<InferredType>> *specialization_return_types_ = nullptr;

    // 全局主类型推断结果，由 TypeInferencer::Process 写入 CompileResult::main_eval_types。
    // 在非特化路径下用于查询任意节点的推断类型（替代原先的 node->EvalType() 调用）。
    const EvalTypeSnapshot *main_eval_types_ = nullptr;

    // 特化上下文——在特化函数体编译期间填充。
    // 将数学参数名称映射到其原生类型（T_INT 或 T_FLOAT）。
    // 不在特化中时为空。
    std::unordered_map<std::string, InferredType> spec_param_types_;

    // 当前正在特化的函数基础名称（不在特化中时为空）。
    std::string cur_spec_func_name_;

    // 当前特化的位掩码（-1 表示不在特化中）。
    // 第 i 位为 0 → math_param[i] 为 int64_t；第 i 位为 1 → double。
    int cur_spec_bitmask_ = -1;

    // 当前特化位掩码对应的快照，不在特化中时为 nullptr。
    // 指向 specialization_snapshots_ 中的数据。
    const EvalTypeSnapshot *cur_spec_snapshot_ = nullptr;
};

}// namespace fakelua
