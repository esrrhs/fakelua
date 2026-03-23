#pragma once

#include "bison/location.hh"
#include "bison/parser.h"
#include "compile/syntax_tree.h"
#include <FlexLexer.h>
#include <fstream>
#include <sstream>

namespace fakelua {

// 自定义词法分析器，集成 Flex 与语法树构建功能
class MyFlexer : public yyFlexLexer {
public:
    MyFlexer() = default;

    ~MyFlexer() override = default;

    // Bison 调用的词法分析接口，实现在 scanner.cpp 中
    virtual int MyYylex(yy::parser::semantic_type *yylval, yy::parser::location_type *yylloc);

    // 设置输入为 Lua 源代码文件
    void InputFile(const std::string &file);

    // 设置输入为 Lua 源代码字符串
    void InputString(const std::string &str);

    // 设置解析生成的主语法树根节点
    void SetChunk(const SyntaxTreeInterfacePtr &chunk);

    // 获取当前生成的语法树
    [[nodiscard]] SyntaxTreeInterfacePtr GetChunk() const;

    // 移除字符串两侧的引号并处理转义字符
    std::string RemoveQuotes(const std::string &str);

    // 获取当前正在解析的文件名
    std::string GetFilename() const {
        return filename_;
    }

private:
    // 生成临时文件以供词法分析使用
    std::string GenerateTmpFile(const std::string &str);

private:
    yy::location location_;       // 记录当前 token 的行列位置
    std::string filename_;        // 当前解析的文件名或源标识
    std::ifstream file_;          // 文件输入流
    std::istringstream string_;   // 字符串输入流
    SyntaxTreeInterfacePtr chunk_;// 语法树根节点
};

}// namespace fakelua
