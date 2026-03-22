#include "State.h"
#include "compile/Compiler.h"
#include "fakelua.h"
#include "util/common.h"

namespace fakelua {

State::State(StateConfig config) : FakeluaState(config), stack_(config) {
}

void State::CompileFile(const std::string &filename, const CompileConfig &cfg) {
    LOG_INFO("start CompileFile {}", filename);
    Compiler c;
    auto result = c.CompileFile(shared_from_this(), filename, cfg);
    LOG_INFO("CompileFile {} ok ", filename);
}

void State::CompileString(const std::string &str, const CompileConfig &cfg) {
    LOG_INFO("start CompileString");
    if (str.empty()) {
        return;
    }
    Compiler c;
    auto [file_name, chunk] = c.CompileString(shared_from_this(), str, cfg);
    LOG_INFO("CompileString ok {}", file_name);
}

}// namespace fakelua
