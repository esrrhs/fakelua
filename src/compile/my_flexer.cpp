#include "compile/my_flexer.h"
#include "util/common.h"
#include "util/exception.h"
#include "util/file_util.h"

namespace fakelua {

// 输入文件进行解析
void MyFlexer::InputFile(const std::string &file) {
    file_.open(file.data(), std::ios::binary);
    if (file_.fail()) {
        ThrowFakeluaException("failed to open file " + file);
    }
    filename_ = file;
    location_.initialize(&filename_);
    set_debug(0);
    switch_streams(&file_, nullptr);
}

// 输入字符串进行解析，通过生成临时文件来实现
void MyFlexer::InputString(const std::string &str) {
    filename_ = GenerateTmpFile(str);
    location_.initialize(&filename_);
    set_debug(0);
    string_ = std::istringstream(str.data(), std::ios::binary);
    switch_streams(&string_, nullptr);
}

// 设置解析生成的主语法树
void MyFlexer::SetChunk(const SyntaxTreeInterfacePtr &chunk) {
    chunk_ = chunk;
}

// 获取生成的语法树
SyntaxTreeInterfacePtr MyFlexer::GetChunk() const {
    return chunk_;
}

// 移除字符串两端的引号，并对内部转义字符进行处理
std::string MyFlexer::RemoveQuotes(const std::string &str) {
    DEBUG_ASSERT(str.size() >= 2);
    // 处理单引号或双引号包裹的字符串
    if ((str[0] == '\'' && str[str.size() - 1] == '\'') || (str[0] == '"' && str[str.size() - 1] == '"')) {
        // 需要替换字符串中的转义字符
        return ReplaceEscapeChars(str.substr(1, str.size() - 2));
    } else {
        // 处理 [[...]] 形式的长字符串（Raw String）
        DEBUG_ASSERT(str.size() >= 4);
        DEBUG_ASSERT(str[0] == '[' && str[1] == '[' && str[str.size() - 1] == ']' && str[str.size() - 2] == ']');
        // 长字符串不需要处理转义字符
        return str.substr(2, str.size() - 4);
    }
}

// 将输入的字符串写入系统临时文件，便于后续 Flex 读取
std::string MyFlexer::GenerateTmpFile(const std::string &str) {
    std::string fileName = GenerateTmpFilename("fakelua_myflexer_", ".lua");
    std::ofstream file(fileName);
    DEBUG_ASSERT(file.is_open());
    file << str;
    file.close();
    return fileName;
}

}// namespace fakelua
