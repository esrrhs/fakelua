#include "compile/my_flexer.h"
#include "util/common.h"
#include "util/exception.h"
#include "util/file_util.h"

namespace fakelua {

// 输入文件进行解析
void MyFlexer::InputFile(const std::string &file) {
    is_string_ = false;
    file_.open(file.data(), std::ios::binary);
    if (file_.fail()) {
        ThrowFakeluaException("Read file failed, failed to open file " + file);
    }
    filename_ = file;
    location_.initialize(&filename_);
    set_debug(0);
    switch_streams(&file_, nullptr);
}

// 输入字符串进行解析
void MyFlexer::InputString(const std::string &str) {
    is_string_ = true;
    filename_ = GenerateTmpFile(str);
    location_.initialize(&filename_);
    set_debug(0);
    string_ = std::istringstream(str, std::ios::binary);
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

// 为字符串输入分配唯一文件名标识（不在磁盘上创建空临时文件）
std::string MyFlexer::GenerateTmpFile(const std::string &str) {
    (void) str;
    return GenerateTmpFilename("fakelua_myflexer_", ".lua");
}

}// namespace fakelua
