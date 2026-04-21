#pragma once

#include "jit/vm_function.h"
#include "syntax_tree.h"
#include <string>

namespace fakelua {

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
};

}// namespace fakelua