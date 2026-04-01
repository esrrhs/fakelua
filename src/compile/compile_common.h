#pragma once

#include "syntax_tree.h"
#include <string>

namespace fakelua {

// 编译结果，包含文件名和生成的语法树
struct CompileResult {
    // 源代码文件名
    std::string file_name;
    // 语法树根节点（代码块）
    SyntaxTreeInterfacePtr chunk;
};

}// namespace fakelua