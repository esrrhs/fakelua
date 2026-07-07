#pragma once

#include "compile/inferred_type.h"
#include "compile/hm_type.h"
#include "jit/vm_function.h"
#include "syntax_tree.h"
#include "util/debug.h"
#include <format>
#include <string>
#include <unordered_set>
#include <vector>

namespace fakelua {

inline constexpr const char *kInitFunctionName = "__fakelua_init";

inline std::string InferredTypeToString(InferredType type) {
    switch (type) {
        case T_UNKNOWN:    return "T_UNKNOWN";
        case T_NIL:        return "T_NIL";
        case T_BOOL:       return "T_BOOL";
        case T_INT:        return "T_INT";
        case T_FLOAT:      return "T_FLOAT";
        case T_STRING:     return "T_STRING";
        case T_RECORD:     return "T_RECORD";
        case T_RECORD_OPEN:return "T_RECORD_OPEN";
        case T_DYNAMIC:    return "T_DYNAMIC";
        default:           return "T_UNKNOWN";
    }
}

// 将函数调用的 args 节点展开为原始参数节点数组，
// 覆盖三种语法形式：args ::= (explist) | tableconstructor | LiteralString。
inline std::vector<SyntaxTreeInterfacePtr> ExtractCallRawArgs(const std::shared_ptr<SyntaxTreeArgs> &args_ptr) {
    std::vector<SyntaxTreeInterfacePtr> raw_args;
    DEBUG_ASSERT(args_ptr);
    const auto args_kind = args_ptr->GetArgsKind();
    if (args_kind == ArgsKind::kExpList) {
        const auto explist_ptr = std::dynamic_pointer_cast<SyntaxTreeExplist>(args_ptr->Explist());
        DEBUG_ASSERT(explist_ptr);
        const auto &exps = explist_ptr->Exps();
        raw_args.insert(raw_args.end(), exps.begin(), exps.end());
        return raw_args;
    }
    if (args_kind == ArgsKind::kString) {
        const auto str_exp = args_ptr->String();
        DEBUG_ASSERT(str_exp);
        raw_args.push_back(str_exp);
        return raw_args;
    }
    if (args_kind == ArgsKind::kTableConstructor) {
        const auto table_arg = args_ptr->Tableconstructor();
        DEBUG_ASSERT(table_arg);
        raw_args.push_back(table_arg);
    }
    return raw_args;
}

// ---- vararg 辅助函数 ---------------------------------------------------------

// 判断一个表达式节点是否为 __fakelua_vararg_* 变量引用。
inline bool IsVarargExp(const SyntaxTreeInterfacePtr &exp_node) {
    if (!exp_node || exp_node->Type() != SyntaxTreeType::Exp) {
        return false;
    }
    const auto exp = std::dynamic_pointer_cast<SyntaxTreeExp>(exp_node);
    if (exp->GetExpKind() != ExpKind::kPrefixExp) {
        return false;
    }
    const auto pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(exp->Right());
    if (!pe || pe->GetPrefixKind() != PrefixExpKind::kVar) {
        return false;
    }
    const auto var = std::dynamic_pointer_cast<SyntaxTreeVar>(pe->GetValue());
    if (!var || var->GetVarKind() != VarKind::kSimple) {
        return false;
    }
    return var->GetName().rfind("__fakelua_vararg_", 0) == 0;
}

// ---- 阶段一：解析结果 -------------------------------------------------------
// Parser 的输出：源文件名和语法树根节点。
// 由 Compiler::Compile 在词法/语法解析阶段填充，
// 后续各阶段均以此作为只读输入。
struct ParseResult {
    // 源代码文件名
    std::string file_name;
    // 语法树根节点（代码块）
    SyntaxTreeInterfacePtr chunk;
};

// ---- 阶段三：语义与控制流分析结果 -----------------------------------------
// SemanticAnalysis 的输出。
// 由 SemanticAnalysis::Analyze 填充，供 CGen 使用。
struct AnalysisResult {
    // 函数名 -> 最大返回值数量（-1 代表动态，例如以函数调用结尾）
    std::unordered_map<std::string, int> function_max_returns;
    // 语法分析出的所有函数调用表达式节点集合，供 CGen 直接查询
    std::unordered_set<const SyntaxTreeInterface *> function_call_exps;
    // 语法分析出的所有函数调用到其被调用者名字的映射，供 CGen 直接查询
    std::unordered_map<const SyntaxTreeInterface *, std::string> callee_names;
    // 文件级/全局常量名称集合
    std::unordered_set<std::string> global_const_names;
};

// ── SSA 类型信息 ──────────────────────────────────────────────────────────
struct SSATypeInfo {
    InferredType type = T_UNKNOWN;
    int shape_id = -1;
    bool operator==(const SSATypeInfo &o) const { return type == o.type && shape_id == o.shape_id; }
    bool operator!=(const SSATypeInfo &o) const { return !(*this == o); }
};

// 函数摘要：用于过程间分析（规范 §7）
struct FuncSummary {
    std::string func_name;
    std::vector<SSATypeInfo> param_types;           // 每个参数的 SSA 类型
    SSATypeInfo ret_type{T_UNKNOWN, -1};             // 返回类型（所有 return 的 meet）
    std::vector<bool> param_escape;                  // 参数是否逃逸
    bool is_vararg = false;
    bool being_built = false;                        // 递归检测标记

    // ── Hindley-Milner 多态签名（可选） ───────────────────────────────
    // 当函数参数未绑定时（默认 T_DYNAMIC），我们基于 HM 推导一个多态
    // 签名 param_hm_types → ret_hm_type，使得调用点可以得到更精确的
    // 多态实例化。
    // 注：仅当 must_use_hm 为 true 且 param_hm_types 非空时有效。
    bool must_use_hm = false;
    std::vector<Type *> param_hm_types;  // 每个参数的 HM 类型变量/表达式
    Type *ret_hm_type = nullptr;         // 返回值的 HM 类型表达式
};

// TypeInferencer 的输出 — SSA/CFG/Shape 管线的真相源。
struct InferResult {
    // SSA 版本 → 类型信息
    std::unordered_map<int, SSATypeInfo> ssa_version_types;
    // AST 节点 → SSA 版本号
    std::unordered_map<const SyntaxTreeInterface *, int> node_ssa_version;
    // AST 节点 → 推导类型（主分析路径）
    std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> main_ssa_types;
    // 全局 shape 注册表
    std::shared_ptr<struct ShapeRegistry> shape_registry;
    // 变量名 → 最终 shape_id（所有版本的 meet）
    std::unordered_map<std::string, int> var_final_shapes;
    // table constructor AST 节点 → shape_id
    std::unordered_map<const SyntaxTreeInterface *, int> ctor_target_shapes;
    // 逃逸分析结果：函数名 → (变量名 → 是否逃逸)
    std::unordered_map<std::string, std::unordered_map<std::string, bool>> escape_vars;
    // 函数摘要：函数名 → 摘要
    std::unordered_map<std::string, FuncSummary> func_summaries;
};

struct JitFunctionInfo {
    int params_count = 0;
    bool is_vararg = false;
};

// ---- 阶段五：代码生成结果 ---------------------------------------------------
// CGen 的输出。
// 由 CGen::Generate 填充，供 JIT 编译器使用。
struct GenResult {
    // 生成的C代码字符串
    std::string c_code;
    // 记录的C代码（全局变量、函数声明、函数实现，不含公共头部）。
    // 仅当 CompileConfig::record_c_code 为 true 时由 CGen 填充。
    std::string recorded_c_code;
    // 函数名->函数元信息
    std::unordered_map<std::string, JitFunctionInfo> function_names;
};


}// namespace fakelua
