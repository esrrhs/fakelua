#include "State.h"
#include "compile/Compiler.h"
#include "fakelua.h"
#include "util/common.h"

namespace fakelua {

State::State(StateConfig config) : compiler_(this), stack_(config) {
}

void State::CompileFile(const std::string &filename, const CompileConfig &cfg) {
    LOG_INFO("start CompileFile {}", filename);
    auto result = compiler_.CompileFile(filename, cfg);
    LOG_INFO("CompileFile {} ok ", filename);
}

void State::CompileString(const std::string &str, const CompileConfig &cfg) {
    LOG_INFO("start CompileString");
    if (str.empty()) {
        return;
    }
    auto [file_name, chunk] = compiler_.CompileString(str, cfg);
    LOG_INFO("CompileString ok {}", file_name);
}

}// namespace fakelua
