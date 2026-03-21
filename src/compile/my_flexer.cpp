#include "compile/my_flexer.h"
#include "util/common.h"
#include "util/exception.h"
#include "util/file_util.h"

namespace fakelua {

void MyFlexer::InputFile(const std::string &file) {
    file_.open(file.data(), std::ios::binary);
    if (file_.fail()) {
        ThrowFakeluaException("open file failed " + file);
    }
    filename_ = file;
    location_.initialize(&filename_);
    SetDebug(0);
    SwitchStreams(&file_, nullptr);
}

void MyFlexer::InputString(const std::string &str) {
    filename_ = GenerateTmpFile(str);
    location_.initialize(&filename_);
    SetDebug(0);
    string_ = std::istringstream(str.data(), std::ios::binary);
    SwitchStreams(&string_, nullptr);
}

void MyFlexer::SetChunk(const SyntaxTreeInterfacePtr &chunk) {
    chunk_ = chunk;
}

SyntaxTreeInterfacePtr MyFlexer::GetChunk() const {
    return chunk_;
}

std::string MyFlexer::RemoveQuotes(const std::string &str) {
    DEBUG_ASSERT(str.size() >= 2);
    if (str[0] == '\'' && str[str.size() - 1] == '\'') {
        // we need to replace escape chars in string
        return ReplaceEscapeChars(str.substr(1, str.size() - 2));
    } else if (str[0] == '"' && str[str.size() - 1] == '"') {
        // we need to replace escape chars in string
        return ReplaceEscapeChars(str.substr(1, str.size() - 2));
    } else {
        DEBUG_ASSERT(str.size() >= 4);
        DEBUG_ASSERT(str[0] == '[' && str[1] == '[' && str[str.size() - 1] == ']' && str[str.size() - 2] == ']');
        // raw string doesn't need to replace escape chars
        return str.substr(2, str.size() - 4);
    }
}

std::string MyFlexer::GenerateTmpFile(const std::string &str) {
    // create tmp file in system temp dir
    std::string fileName = GenerateTmpFilename("fakelua_myflexer_", ".lua");
    std::ofstream file(fileName);
    DEBUG_ASSERT(file.is_open());
    file << str;
    file.close();
    return fileName;
}

}// namespace fakelua
