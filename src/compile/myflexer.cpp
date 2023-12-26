#include "compile/myflexer.h"
#include "util/common.h"
#include "util/exception.h"
#include "util/file_util.h"

namespace fakelua {

myflexer::myflexer() {
}

myflexer::~myflexer() {
}

void myflexer::input_file(const std::string &file) {
    file_.open(file.data(), std::ios::binary);
    if (file_.fail()) {
        throw_fakelua_exception("open file failed " + file);
    }
    filename_ = file;
    location_.initialize(&filename_);
    set_debug(0);
    switch_streams(&file_, nullptr);
}

void myflexer::input_string(const std::string &str) {
    filename_ = generate_tmp_file(str);
    location_.initialize(&filename_);
    set_debug(0);
    string_ = std::istringstream(str.data(), std::ios::binary);
    switch_streams(&string_, nullptr);
}

void myflexer::set_chunk(const syntax_tree_interface_ptr &chunk) {
    chunk_ = chunk;
}

syntax_tree_interface_ptr myflexer::get_chunk() const {
    return chunk_;
}

std::string myflexer::remove_quotes(const std::string &str) {
    DEBUG_ASSERT(str.size() >= 2);
    if (str[0] == '\'' && str[str.size() - 1] == '\'') {
        // we need to replace escape chars in string
        return replace_escape_chars(str.substr(1, str.size() - 2));
    } else if (str[0] == '"' && str[str.size() - 1] == '"') {
        // we need to replace escape chars in string
        return replace_escape_chars(str.substr(1, str.size() - 2));
    } else {
        DEBUG_ASSERT(str.size() >= 4);
        DEBUG_ASSERT(str[0] == '[' && str[1] == '[' && str[str.size() - 1] == ']' && str[str.size() - 2] == ']');
        // raw string not need to replace escape chars
        return str.substr(2, str.size() - 4);
    }
}

std::string myflexer::generate_tmp_file(const std::string &str) {
    // create tmp file in system temp dir
    std::string fileName = generate_tmp_filename("fakelua_myflexer_", ".lua");
    std::ofstream file(fileName);
    DEBUG_ASSERT(file.is_open());
    file << str;
    file.close();
    return fileName;
}

}// namespace fakelua
