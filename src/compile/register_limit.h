#pragma once

#include "compile/compile_common.h"
#include "compile/register_limits.h"
#include "fakelua.h"
#include "interp/func_proto.h"
#include "interp/opcode.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace fakelua {

class State;

// 共享的每函数寄存器分配遍历：用与解释器字节码生成相同的
// AllocLocal / AllocReg / FreeTo / EnsureStack 寿命规则计量高峰并发槽。
// Check() 只走分配、不创建字节码 / 原型 / 常量池、不访问 State。
// Build() 在同一套遍历上真正生成 Interp 字节码（由 InterpCodegen 使用）。
class FunctionRegisterUsage {
public:
    explicit FunctionRegisterUsage(State *s);

    // 只生成字节码并做寄存器高峰检查；不注册 VM、不跑 __fakelua_init。
    void Build(const ParseResult &pr, const AnalysisResult &ar);

    // 复用相同 AST/寄存器分配遍历；不创建字节码、原型、常量池，不访问 State。
    void Check(const ParseResult &pr, const AnalysisResult &ar);

protected:
    struct FuncInfo;
    struct VarDef;

    struct Scope {
        FuncInfo *func = nullptr;
        std::unordered_map<std::string, VarDef *> vars;
    };

    struct VarDef {
        std::string name;
        const SyntaxTreeInterface *def_node = nullptr;
        FuncInfo *defining_func = nullptr;
        bool is_captured = false;
        int reg = -1;
    };

    struct FuncInfo {
        const SyntaxTreeInterface *node = nullptr;
        SyntaxTreeInterfacePtr funcbody;
        FuncInfo *parent = nullptr;
        std::string name;
        std::string unique_c_name;
        std::vector<std::string> params;
        bool is_vararg = false;
        std::vector<VarDef *> captured_vars;
        std::unordered_set<VarDef *> captured_set;
        FuncProto *proto = nullptr;
        bool compiled = false;
    };

    struct PairHash {
        template<class T1, class T2>
        std::size_t operator()(const std::pair<T1, T2> &p) const {
            return std::hash<T1>{}(p.first) ^ (std::hash<T2>{}(p.second) << 1);
        }
    };

    struct LoopInfo {
        enum Kind { kWhile, kRepeat, kFor, kForIn } kind = kWhile;
        int continue_ip = -1;
        std::vector<int> break_jmps;
        std::vector<int> continue_jmps;
    };

    void ResolveScopes(const SyntaxTreeInterfacePtr &node, std::vector<Scope> &scopes, std::vector<FuncInfo *> &func_stack, FuncInfo *cur_func);
    std::string CompileFuncName(const SyntaxTreeInterfacePtr &ptr);

    void CompileFunction(FuncInfo *func);
    void CompileStmtBlock(const SyntaxTreeInterfacePtr &block);
    void CompileStmt(const SyntaxTreeInterfacePtr &stmt);
    void CompileStmtReturn(const SyntaxTreeInterfacePtr &stmt);
    void CompileStmtLocalVar(const SyntaxTreeInterfacePtr &stmt);
    void CompileStmtAssign(const SyntaxTreeInterfacePtr &stmt);
    void CompileStmtWhile(const SyntaxTreeInterfacePtr &stmt);
    void CompileStmtRepeat(const SyntaxTreeInterfacePtr &stmt);
    void CompileStmtIf(const SyntaxTreeInterfacePtr &stmt);
    void CompileStmtForLoop(const SyntaxTreeInterfacePtr &stmt);
    void CompileStmtForIn(const SyntaxTreeInterfacePtr &stmt);
    void CompileStmtLocalFunction(const SyntaxTreeInterfacePtr &stmt);
    void CompileStmtBreak();
    void CompileStmtContinue();
    void CompileStmtGoto(const SyntaxTreeInterfacePtr &stmt);
    void CompileStmtLabel(const SyntaxTreeInterfacePtr &stmt);

    int CompileExp(const SyntaxTreeInterfacePtr &exp, bool preserve_multi = false, int dest = -1);
    int CompilePrefixexp(const SyntaxTreeInterfacePtr &pe, bool preserve_multi, int dest = -1);
    int CompileVar(const SyntaxTreeInterfacePtr &v);
    int CompileFunctioncall(const SyntaxTreeInterfacePtr &functioncall, bool preserve_multi);
    int CompileTableconstructor(const SyntaxTreeInterfacePtr &tc);
    int CompileBinop(const SyntaxTreeInterfacePtr &exp, int dest = -1);
    int CompileUnop(const SyntaxTreeInterfacePtr &exp, int dest = -1);

    int EmitClosure(FuncInfo *child);
    void EmitCallArgs(const std::shared_ptr<class SyntaxTreeArgs> &args_ptr, std::vector<int> &arg_regs, bool &last_expand);

    void Walk(const ParseResult &pr, const AnalysisResult &ar);
    int CodeSize() const;
    int64_t StringId(const std::string &s);

    int AllocReg();
    int AllocLocal();
    void FreeTo(int top);
    void EnsureStack(int n);
    int AddConst(const CVar &v);
    int AddStringConst(const std::string &s);
    int Emit(Op op, int a = 0, int b = 0, int c = 0, int sbx = 0);
    void PatchSbx(int ip, int dest);

    int LoadConstTo(int dest, const CVar &v);
    int MoveTo(int dest, int src);
    int CommitDest(int dest, int src);
    int LocalAssignDest(const SyntaxTreeVar *var);
    void StoreLocal(int dest, int src);
    int LoadLocal(VarDef *def);
    int ResolveSimpleName(const SyntaxTreeVar *var, const std::string &name, bool as_callee);
    void AssignToVar(const SyntaxTreeVar *var, int src);
    int EmitThrow(const std::string &msg);
    bool LastPreservesMulti(const SyntaxTreeInterfacePtr &exp) const;
    bool IsPackageHeaderStmt(const SyntaxTreeInterfacePtr &stmt) const;
    CVar LiteralFromExp(const SyntaxTreeInterfacePtr &exp);

    [[noreturn]] void ThrowError(const std::string &msg, const SyntaxTreeInterfacePtr &ptr);

    enum class PairsIpairsKind { kNone, kPairs, kIpairs };
    [[nodiscard]] PairsIpairsKind TryMatchPairsIpairs(const std::shared_ptr<class SyntaxTreeExplist> &explist_ptr, SyntaxTreeInterfacePtr &out_tbl_arg);

    static const std::unordered_set<std::string> kStringLibraryMethods;

    bool check_only_ = false;
    int check_ip_ = 0; // 控制流位置占位，无指令流
    State *s_ = nullptr;
    std::string file_name_;
    const AnalysisResult *ar_ = nullptr;
    InterpUnit *unit_ = nullptr;
    // Build 持有；RegisterAndInit 交给 VM handle 续命。校验-only 路径随本对象析构丢弃。
    std::shared_ptr<InterpUnit> unit_hold_;

    std::vector<std::unique_ptr<VarDef>> all_defs_;
    std::vector<std::unique_ptr<FuncInfo>> all_funcs_;
    std::unordered_map<const SyntaxTreeInterface *, FuncInfo *> func_map_;
    std::unordered_map<std::pair<const SyntaxTreeInterface *, std::string>, VarDef *, PairHash> stmt_var_to_def_;
    std::unordered_map<const SyntaxTreeInterface *, VarDef *> var_to_def_map_;
    std::unordered_map<std::string, FuncProto *> named_protos_;
    // 文件级函数的形参个数与是否变长（name / unique_c_name 都登记）。CGen 的 arity 检查
    // 靠它，解释器单独编译时（disable_jit 关掉两个 C 后端）也要靠它，否则少传参数会
    // 静默补 nil，三个后端语义就不一致了。
    std::unordered_map<std::string, std::pair<int, bool>> file_func_sigs_;
    std::string cur_package_name_;
    std::unordered_set<std::string> file_level_names_;

    FuncInfo *cur_func_ = nullptr;
    FuncProto *cur_proto_ = nullptr;
    // 最近一条语句/表达式在 Lua 源码里的行号，Emit 时盖到 lineinfo 上。
    int cur_line_ = 0;
    int stack_top_ = 0;
    int local_top_ = 0;
    std::vector<LoopInfo> loops_;

    struct LabelScope {
        std::unordered_map<std::string, int> defined;
        std::unordered_map<std::string, std::vector<int>> pending;
    };
    std::vector<LabelScope> label_scopes_;
};

// 语义分析阶段入口：对每个函数做与解释器相同的寄存器高峰检查，无需 State。
void CheckFunctionRegisterLimits(const ParseResult &pr, const AnalysisResult &ar);

}// namespace fakelua
