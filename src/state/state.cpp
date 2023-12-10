#include "state.h"
#include "compile/compiler.h"
#include "fakelua.h"
#include "util/common.h"

namespace fakelua {

state::state() {
}

state::~state() {
}

void state::compile_file(const std::string &filename, compile_config cfg) {
    LOG(INFO) << "start compile_file " << filename;
    compiler c;
    auto result = c.compile_file(shared_from_this(), filename, cfg);
    LOG(INFO) << "compile_file " << filename << " ok ";
}

void state::compile_string(const std::string &str, compile_config cfg) {
    LOG(INFO) << "start compile_string";
    compiler c;
    auto result = c.compile_string(shared_from_this(), str, cfg);
    LOG(INFO) << "compile_string ok " << result.file_name;
}

void *state::get_func_addr(const std::string &name) {
    auto func = vm_.get_function(name);
    if (func) {
        return func->get_addr();
    }
    return nullptr;
}

void state::set_var_interface_new_func(std::function<var_interface *()> func) {
    var_interface_new_func_ = func;
}


}// namespace fakelua
