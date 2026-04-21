#pragma once

#include "jit/vm_function.h"
#include "syntax_tree.h"
#include <string>

namespace fakelua {

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
    // 由 ParamNumericAnalyzer 填充，供 CGen 生成特化版本时使用。
    std::unordered_map<std::string, std::vector<int>> math_param_positions;
};

}// namespace fakelua