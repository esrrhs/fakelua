#pragma once

#include "compile/inferred_type.h"
#include "jit/vm_function.h"
#include "syntax_tree.h"
#include "util/debug.h"
#include <format>
#include <string>
#include <unordered_set>
#include <vector>

namespace fakelua {

// AST 节点类型快照：节点原始指针 → 推断类型。
// 每个特化 bitmask 对应一份快照，由 TypeInferencer::InferTypes 产生，
// 供 CGen 在生成特化体时查询任意节点的类型。
using EvalTypeSnapshot = std::unordered_map<SyntaxTreeInterface *, InferredType>;

// 特化参数类型位掩码编码：bitmask 的第 i 位为 0 表示第 i 个数学参数为 int64_t，
// 为 1 表示为 double。
enum MathParamKind : int {
    kMathParamInt = 0,  // 对应 int64_t
    kMathParamFloat = 1,// 对应 double
};

// 根据 bitmask 和参数位索引返回该参数的类型。
inline MathParamKind MathParamKindOf(int bitmask, int bit_index) {
    return ((bitmask >> bit_index) & 1) ? kMathParamFloat : kMathParamInt;
}

// 返回 MathParamKind 对应的 C 类型名称字符串。
inline const char *MathParamCTypeName(MathParamKind kind) {
    return kind == kMathParamFloat ? "double" : "int64_t";
}

// 返回 MathParamKind 对应的 bitmask 后缀字符（'0' 或 '1'）。
inline char MathParamSuffix(MathParamKind kind) {
    return kind == kMathParamFloat ? '1' : '0';
}

inline std::string InferredTypeToString(InferredType type) {
    switch (type) {
        case T_UNKNOWN:
            return "T_UNKNOWN";
        case T_INT:
            return "T_INT";
        case T_FLOAT:
            return "T_FLOAT";
        case T_DYNAMIC:
            return "T_DYNAMIC";
        default:
            return "T_UNKNOWN";
    }
}

inline bool IsNumericInferredType(const InferredType type) {
    return type == T_INT || type == T_FLOAT;
}

inline InferredType InferNumericBinopResultType(const BinOpKind op_kind, const InferredType left_type, const InferredType right_type) {
    if (!IsNumericInferredType(left_type) || !IsNumericInferredType(right_type)) {
        return T_DYNAMIC;
    }
    if (op_kind == BinOpKind::kSlash || op_kind == BinOpKind::kPow) {
        return T_FLOAT;
    }
    if (op_kind == BinOpKind::kPlus || op_kind == BinOpKind::kMinus || op_kind == BinOpKind::kStar || op_kind == BinOpKind::kMod || op_kind == BinOpKind::kDoubleSlash) {
        return (left_type == T_INT && right_type == T_INT) ? T_INT : T_FLOAT;
    }
    if (op_kind == BinOpKind::kBitAnd || op_kind == BinOpKind::kXor || op_kind == BinOpKind::kBitOr || op_kind == BinOpKind::kLeftShift || op_kind == BinOpKind::kRightShift) {
        return T_INT;
    }
    if (op_kind == BinOpKind::kAnd) {
        return right_type;
    }
    if (op_kind == BinOpKind::kOr) {
        return left_type;
    }
    return T_DYNAMIC;
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

// ---- 无状态代码生成帮助函数 -------------------------------------------------
// 这些函数不依赖任何实例状态，供 CGen 使用。

// 返回特化函数返回值对应的 C 类型名称字符串（"int64_t"、"double" 或 "CVar"）。
inline const char *SpecReturnCTypeName(InferredType ret_type) {
    if (ret_type == T_INT) {
        return "int64_t";
    }
    if (ret_type == T_FLOAT) {
        return "double";
    }
    return "CVar";
}

// 根据基础名称、数学参数下标列表和位掩码返回特化函数名。
// 例如 SpecFuncName("fib", {0}, 0) -> "fib_0"
//       SpecFuncName("test", {1,4}, 2) -> "test_0_1"
inline std::string SpecFuncName(const std::string &base_name, const std::vector<int> &math_param_indices, int bitmask) {
    std::string name = base_name;
    for (int i = 0; i < static_cast<int>(math_param_indices.size()); ++i) {
        name += '_';
        name += MathParamSuffix(MathParamKindOf(bitmask, i));
    }
    return name;
}

// 将原生 C 数值表达式（int64_t / double）装箱为 CVar 字面量。
// type 必须为 T_INT 或 T_FLOAT。
inline std::string BoxNativeValue(const std::string &expr, InferredType type) {
    if (type == T_INT) {
        return std::format("(CVar){{.type_ = VAR_INT, .data_.i = (int64_t)({})}}", expr);
    }
    DEBUG_ASSERT(type == T_FLOAT);
    return std::format("(CVar){{.type_ = VAR_FLOAT, .data_.f = (double)({})}}", expr);
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

// ---- 阶段四：类型推断结果 ---------------------------------------------------
// TypeInferencer 的输出。
// 由 TypeInferencer::InferTypes 填充，供 CGen 使用。
struct InferResult {
    // 数学参数位置：函数名 → 参数列表中参与算术运算的参数下标列表（最多8个）。
    // 由 TypeInferencer::InferTypes 填充，供 CGen 生成特化版本时使用。
    std::unordered_map<std::string, std::vector<int>> math_param_positions;
    // 特化快照：函数名 → 按 bitmask 索引 of EvalTypeSnapshot 数组（共 2^k 个）。
    // 每个快照记录在对应参数类型假设下整个函数体所有 AST 节点的推断类型，
    // 由 TypeInferencer::InferTypes 生成，供 CGen 在不依赖 EvalType()
    // 字段的情况下直接查询特化版本中任意节点的类型。
    std::unordered_map<std::string, std::vector<EvalTypeSnapshot>> specialization_snapshots;
    // 特化返回类型：函数名 → 按 bitmask 索引的返回类型数组（共 2^k 个）。
    // T_INT/T_FLOAT 表示该特化版本始终返回对应数值类型；T_DYNAMIC 表示未知或非数值。
    // 由 TypeInferencer::InferTypes 通过不动点迭代填充，
    // 供 CGen::InferArgTypeForSpec 在函数调用节点处查询被调用函数的实际返回类型。
    std::unordered_map<std::string, std::vector<InferredType>> specialization_return_types;
    // 全局类型推断结果：节点指针 → 推断类型。
    // 由 TypeInferencer::InferTypes 在全局（非试推断）推断完成后填充，
    // 供 CGen 在非特化编译路径中查询任意节点的类型，替代原先内嵌在 AST 节点的 eval_type_ 字段。
    EvalTypeSnapshot main_eval_types;
    // 文件级/全局数值常量及其推断类型映射
    std::unordered_map<std::string, InferredType> global_const_vars;
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
