#pragma once

#include "jit/vm_function.h"
#include "syntax_tree.h"
#include <string>

namespace fakelua {

// AST 节点类型快照：节点原始指针 → 推断类型。
// 每个特化 bitmask 对应一份快照，由 TypeInferencer::DiscoverMathParams 生成，
// 供 CGen 在生成特化体时查询任意节点的类型。
using EvalTypeSnapshot = std::unordered_map<SyntaxTreeInterface *, InferredType>;

// 特化参数类型位掩码编码：bitmask 的第 i 位为 0 表示第 i 个数学参数为 int64_t，
// 为 1 表示为 double。
enum MathParamKind : int {
    kMathParamInt = 0,   // 对应 int64_t
    kMathParamFloat = 1, // 对应 double
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

inline bool IsNumericInferredType(const InferredType type) {
    return type == T_INT || type == T_FLOAT;
}

inline InferredType InferNumericBinopResultType(const BinOpKind op_kind,
                                                const InferredType left_type,
                                                const InferredType right_type) {
    if (!IsNumericInferredType(left_type) || !IsNumericInferredType(right_type)) {
        return T_DYNAMIC;
    }
    if (op_kind == BinOpKind::kSlash || op_kind == BinOpKind::kPow) {
        return T_FLOAT;
    }
    if (op_kind == BinOpKind::kPlus || op_kind == BinOpKind::kMinus || op_kind == BinOpKind::kStar ||
        op_kind == BinOpKind::kMod || op_kind == BinOpKind::kDoubleSlash) {
        return (left_type == T_INT && right_type == T_INT) ? T_INT : T_FLOAT;
    }
    if (op_kind == BinOpKind::kBitAnd || op_kind == BinOpKind::kXor || op_kind == BinOpKind::kBitOr ||
        op_kind == BinOpKind::kLeftShift || op_kind == BinOpKind::kRightShift) {
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

// Extracts a function-call args node into a raw argument node array, covering
// the three syntax forms: args ::= (explist) | tableconstructor | LiteralString.
inline std::vector<SyntaxTreeInterfacePtr> ExtractCallRawArgs(const std::shared_ptr<SyntaxTreeArgs> &args_ptr) {
    std::vector<SyntaxTreeInterfacePtr> raw_args;
    if (!args_ptr) {
        return raw_args;
    }
    const auto args_kind = args_ptr->GetArgsKind();
    if (args_kind == ArgsKind::kExpList) {
        const auto explist_ptr = std::dynamic_pointer_cast<SyntaxTreeExplist>(args_ptr->Explist());
        if (!explist_ptr) {
            return raw_args;
        }
        const auto &exps = explist_ptr->Exps();
        raw_args.insert(raw_args.end(), exps.begin(), exps.end());
        return raw_args;
    }
    if (args_kind == ArgsKind::kString) {
        if (const auto str_exp = args_ptr->String()) {
            raw_args.push_back(str_exp);
        }
        return raw_args;
    }
    if (args_kind == ArgsKind::kTableConstructor) {
        if (const auto table_arg = args_ptr->Tableconstructor()) {
            raw_args.push_back(table_arg);
        }
    }
    return raw_args;
}

// 编译结果，包含文件名和生成的语法树
struct CompileResult {
    // 源代码文件名
    std::string file_name;
    // 语法树根节点（代码块）
    SyntaxTreeInterfacePtr chunk;
    // 生成的C代码字符串
    std::string c_code;
    // 记录的C代码（全局变量、函数声明、函数实现，不含公共头部）。
    // 仅当 CompileConfig::record_c_code 为 true 时由 CGen 填充。
    std::string recorded_c_code;
    // 入口函数名->参数个数
    std::unordered_map<std::string, int> function_names;
    // 数学参数位置：函数名 → 参数列表中参与算术运算的参数下标列表（最多8个）。
    // 由 TypeInferencer::DiscoverMathParams 填充，供 CGen 生成特化版本时使用。
    std::unordered_map<std::string, std::vector<int>> math_param_positions;
    // 特化快照：函数名 → 按 bitmask 索引的 EvalTypeSnapshot 数组（共 2^k 个）。
    // 每个快照记录在对应参数类型假设下整个函数体所有 AST 节点的推断类型，
    // 由 TypeInferencer::DiscoverMathParams 生成，供 CGen 在不依赖 EvalType()
    // 字段的情况下直接查询特化版本中任意节点的类型。
    std::unordered_map<std::string, std::vector<EvalTypeSnapshot>> specialization_snapshots;
    // 特化返回类型：函数名 → 按 bitmask 索引的返回类型数组（共 2^k 个）。
    // T_INT/T_FLOAT 表示该特化版本始终返回对应数值类型；T_DYNAMIC 表示未知或非数值。
    // 由 TypeInferencer::DiscoverMathParams 通过不动点迭代填充，
    // 供 CGen::InferArgTypeForSpec 在函数调用节点处查询被调用函数的实际返回类型。
    std::unordered_map<std::string, std::vector<InferredType>> specialization_return_types;
    // 全局类型推断结果：节点指针 → 推断类型。
    // 由 TypeInferencer::Process 在全局（非试推断）推断完成后填充，
    // 供 CGen 在非特化编译路径中查询任意节点的类型，替代原先内嵌在 AST 节点的 eval_type_ 字段。
    EvalTypeSnapshot main_eval_types;
};

}// namespace fakelua
