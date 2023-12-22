#include "state.h"
#include "compile/compiler.h"
#include "fakelua.h"
#include "util/common.h"

namespace fakelua {

state::state() : var_string_heap_(this) {
}

state::~state() {
}

void state::compile_file(const std::string &filename, compile_config cfg) {
    LOG_INFO("start compile_file {}", filename);
    compiler c;
    auto result = c.compile_file(shared_from_this(), filename, cfg);
    LOG_INFO("compile_file {} ok ", filename);
}

void state::compile_string(const std::string &str, compile_config cfg) {
    LOG_INFO("start compile_string");
    compiler c;
    auto result = c.compile_string(shared_from_this(), str, cfg);
    LOG_INFO("compile_string ok {}", result.file_name);
}

void state::set_var_interface_new_func(std::function<var_interface *()> func) {
    var_interface_new_func_ = func;
}

}// namespace fakelua
