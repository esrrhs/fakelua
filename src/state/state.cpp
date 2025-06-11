#include "state.h"
#include "compile/compiler.h"
#include "fakelua.h"
#include "util/common.h"

namespace fakelua {

void state::compile_file(const std::string &filename, compile_config cfg) {
    LOG_INFO("start compile_file {}", filename);
    compiler c;
    auto result = c.compile_file(shared_from_this(), filename, cfg);
    LOG_INFO("compile_file {} ok ", filename);
}

void state::compile_string(const std::string &str, compile_config cfg) {
    LOG_INFO("start compile_string");
    if (str.empty()) {
        return;
    }
    compiler c;
    auto [file_name, chunk] = c.compile_string(shared_from_this(), str, cfg);
    LOG_INFO("compile_string ok {}", file_name);
}

}// namespace fakelua
