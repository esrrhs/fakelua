#include "state.h"
#include "compile/compiler.h"
#include "fakelua.h"
#include "util/common.h"

namespace fakelua {

state::state() {
}

state::~state() {
}

void state::compile_file(const std::string_view &filename) {
    LOG(INFO) << "start compile_file " << filename;
    compiler c;
    auto result = c.compile_file(shared_from_this(), filename, {});
    LOG(INFO) << "compile_file " << filename << " ok ";
}

void state::compile_string(const std::string_view &str) {
    LOG(INFO) << "start compile_string";
    compiler c;
    auto result = c.compile_string(shared_from_this(), str, {});
    LOG(INFO) << "compile_string ok " << result.file_name;
}

void *state::get_func_addr(const std::string_view &name) {
    return nullptr;
}

}// namespace fakelua
