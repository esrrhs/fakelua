#include "state.h"
#include "compile/compiler.h"
#include "fakelua.h"
#include "util/common.h"

namespace fakelua {

fakelua_state_impl::fakelua_state_impl() {
}

fakelua_state_impl::~fakelua_state_impl() {
}

void fakelua_state_impl::compile_file(const std::string &filename) {
    LOG(INFO) << "start compile_file " << filename;
    compiler c;
    auto result = c.compile_file(filename, {});
    LOG(INFO) << "compile_file " << filename << " ok ";
}

void fakelua_state_impl::compile_string(const std::string &str) {
    LOG(INFO) << "start compile_string";
    compiler c;
    auto result = c.compile_string(str, {});
    LOG(INFO) << "compile_string ok ";
}

}// namespace fakelua
