#pragma once

#include "compile/inferred_type.h"
#include "jit/vm_function.h"
#include "syntax_tree.h"
#include "util/debug.h"
#include <algorithm>
#include <format>
#include <string>
#include <unordered_set>
#include <vector>

namespace fakelua {

inline constexpr const char *kInitFunctionName = "__fakelua_init";

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
enum class TableKeyKind {
    kString,
    kInt,
    kBool,
    kFloat
};

// table 特化信息：描述一个 table 的字段结构
struct TableFieldInfo {
    std::string key;            // 字段名（静态字符串 key）
    InferredType type;          // 值的推断类型
    TableKeyKind key_kind = TableKeyKind::kString;
    std::string c_field_name;
    int64_t int_value = 0;
    bool bool_value = false;
    double float_value = 0.0;
    bool optional = false;      // Phase 2: 该字段在当前 constructor 字面量中不存在（由合并产生），emit 时需 nil 初始化
};

// table 特化信息：table constructor 节点对应的特化描述
struct TableSpecInfo {
    std::vector<TableFieldInfo> fields;
    bool can_specialize;  // 所有访问是否已知（可特化）
};

// per-spec-type 字段布局元数据：描述一个 spec 结构体类型（flua_spec_<hex>）的字段布局。
// 由 TypeInferencer 在 AnalyzeTableShapes 之后预计算，供 CGen 发射 typedef/getter/setter 以及
// CompileVar 解析字段时按名查询。按 spec 类型名（字段签名哈希）去重——同名类型共享同一布局。
struct SpecTypeMetadata {
    std::string name;                         // spec 类型名（flua_spec_<hex>）
    std::vector<TableFieldInfo> fields;       // 排序后的字段列表（含 c_field_name / key / key_kind / type / optional）
    bool has_string_keys = false;
    bool has_int_keys = false;
    bool has_float_keys = false;
    bool has_bool_keys = false;
    // 派生索引：字段 key 描述符 → 各类查询（由 ComputeSpecTypeMetadata 一次性建好）。
    std::unordered_set<std::string> field_key_descs;                 // IsSpecField 用
    std::unordered_map<std::string, std::string> c_field_names;      // GetSpecFieldCName 用
    std::unordered_map<std::string, int> field_indices;              // GetSpecFieldIndex 用
    std::unordered_map<std::string, InferredType> field_types;       // GetSpecFieldType 用
};

// 数学参数特化上下文：每个 (函数名, bitmask) 对应一个，描述该特化版本的参数类型假设
// 与节点类型快照。由 TypeInferencer 预计算，供 CGen 的 CompileFuncBody 消费——
// CGen 不再自行做 MathParamKindOf 推导、snapshot 选择或 param_types 初始填充。
struct SpecFuncContext {
    std::string func_name;                                          // 函数名
    int bitmask = -1;                                               // 特化位掩码
    const EvalTypeSnapshot *snapshot = nullptr;                     // 该版本所有 AST 节点的推断类型快照
    std::unordered_map<std::string, InferredType> param_types;      // 参数名 → 初始特化类型（int/float）
    std::vector<std::string> param_names;                           // 所有参数名（按位置顺序），供 CGen 初始化 spec_param_types_ 使用
};

// 字段 key 描述符（与 CGen::GetKeyDescriptor / TypeInferencer::FieldKeyDescriptor 一致）。
// 单一事实来源，保证 spec 类型名哈希与 TypeInferencer 去重/并集逻辑使用完全相同的描述符。
inline std::string TableFieldDescriptor(const TableFieldInfo &f) {
    switch (f.key_kind) {
        case TableKeyKind::kString: return "S_" + f.key;
        case TableKeyKind::kInt:    return "I_" + f.key;
        case TableKeyKind::kBool:   return "B_" + f.key;
        case TableKeyKind::kFloat:  return "F_" + f.key;
    }
    return "";
}

// 根据字段布局计算 spec 结构体类型名：flua_spec_<hex(签名)>。
// 签名 = 排序后的字段 key 描述符拼接，确保相同布局（无论字段顺序）产生相同类型名，
// 使不同 constructor 字面量在合并后共享同一 typedef + get/set，并让 CGen 侧的
// 字符串比较 join 能识别两分支构造的同 shape table 为一致。
// 共享给 TypeInferencer（前向流分析时预算 spec 名）和 CGen（emit 时复用）。
inline std::string ComputeTableSpecName(const std::vector<TableFieldInfo> &fields) {
    std::vector<std::string> descs;
    descs.reserve(fields.size());
    for (const auto &f: fields) {
        descs.push_back(TableFieldDescriptor(f));
    }
    std::sort(descs.begin(), descs.end());
    std::string sig;
    for (size_t i = 0; i < descs.size(); ++i) {
        if (i) sig += '|';
        sig += descs[i];
    }
    // FNV-1a 64bit 哈希，输出 16 进制，保持类型名简短且确定。
    uint64_t h = 14695981039346656037ULL;
    for (const char c: sig) {
        h ^= static_cast<uint64_t>(static_cast<unsigned char>(c));
        h *= 1099511628211ULL;
    }
    return std::format("flua_spec_{:016x}", h);
}

// 从 Var/PrefixExp/Exp 节点中抽取底层简单变量名。
// 共享给 TypeInferencer（前向流分析时解析 LHS/RHS 变量名）和 CGen。
inline std::string GetSimpleVarName(const SyntaxTreeInterfacePtr &pe) {
    if (!pe) return "";
    if (pe->Type() == SyntaxTreeType::Var) {
        auto var = std::dynamic_pointer_cast<SyntaxTreeVar>(pe);
        if (var->GetVarKind() == VarKind::kSimple) return var->GetName();
    } else if (pe->Type() == SyntaxTreeType::PrefixExp) {
        auto pf = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(pe);
        if (pf->GetPrefixKind() == PrefixExpKind::kVar) return GetSimpleVarName(pf->GetValue());
    } else if (pe->Type() == SyntaxTreeType::Exp) {
        auto exp = std::dynamic_pointer_cast<SyntaxTreeExp>(pe);
        if (exp->GetExpKind() == ExpKind::kPrefixExp) return GetSimpleVarName(exp->Right());
    }
    return "";
}

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
    // 供 CGen::GetType 在函数调用节点处查询被调用函数的实际返回类型。
    std::unordered_map<std::string, std::vector<InferredType>> specialization_return_types;
    // 全局类型推断结果：节点指针 → 推断类型。
    // 由 TypeInferencer::InferTypes 在全局（非试推断）推断完成后填充，
    // 供 CGen 在非特化编译路径中查询任意节点的类型，替代原先内嵌在 AST 节点的 eval_type_ 字段。
    EvalTypeSnapshot main_eval_types;
    // 文件级/全局数值常量及其推断类型映射
    std::unordered_map<std::string, InferredType> global_const_vars;
    // table 特化信息：table constructor 节点 → 特化信息
    std::unordered_map<const SyntaxTreeInterface *, struct TableSpecInfo> table_spec_infos;
    // 流敏感 table 特化标注：Var 引用节点（kDot/kSquare 的 prefixexp 所指 Var 节点）→ 该程序点该变量的 spec 类型名。
    // 空串表示 dynamic（走哈希路径）。由 TypeInferencer 的前向流分析填充，供 CGen 纯读取。
    std::unordered_map<const SyntaxTreeInterface *, std::string> var_spec_annotations;
    // per-spec-type 字段布局元数据（spec 类型名 → metadata）。
    // 由 TypeInferencer 预计算（按 spec 类型名去重），供 CGen 发射 typedef/getter/setter 以及
    // 字段名/C 字段名/索引/类型查询——CGen 不再自行计算字段布局。
    std::unordered_map<std::string, struct SpecTypeMetadata> spec_type_metadata;
    // 数学参数特化上下文（函数名 → per-bitmask 数组，共 2^k 个）。
    // 由 TypeInferencer 预计算，描述每个特化版本的参数类型假设与节点类型快照。
    // CGen::CompileFuncBody 据此初始化发射上下文，不再自行做 MathParamKindOf 推导与 snapshot 选择。
    std::unordered_map<std::string, std::vector<struct SpecFuncContext>> spec_func_context;
    // 变量定义关联表：从语法树变量引用节点 (SyntaxTreeVar) 指针 → 对应的初始化声明节点 (init_node)。
    // 由 TypeInferencer 填充，供 CGen 直接利用声明节点快照来查询当前物理变量是 native 还是 CVar。
    std::unordered_map<const SyntaxTreeInterface*, const SyntaxTreeInterface*> var_define_nodes;
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
