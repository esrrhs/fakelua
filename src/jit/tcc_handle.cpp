#include "jit/tcc_handle.h"
#include "state/state.h"
#include "util/logging.h"
#include "jit/vm.h"
#include <libtcc.h>

namespace fakelua {

// 前向声明 CVar（完整定义在 fakelua.h）
struct CVar;

// 声明日志函数（定义在 util/logging.cpp）
extern "C" void FakeluaLogLua(int level, CVar msg, const char *file, int line, const char *fname);
extern "C" int GetLogLevel();

TCCHandle::TCCHandle(State *s, const CompileConfig &cfg) {
    const auto &config = s->GetStateConfig();
    tcc_state_ = tcc_new();
    if (!tcc_state_) {
        ThrowFakeluaException("tcc_new() failed to create TCC state");
    }
    if (!cfg.debug_mode) {
        tcc_set_options(tcc_state_, "-O2");
    }
    for (const auto &path: config.tcc_config.include_paths) {
        tcc_add_sysinclude_path(tcc_state_, path.c_str());
    }
    for (const auto &path: config.tcc_config.library_paths) {
        tcc_add_library_path(tcc_state_, path.c_str());
    }
    for (const auto &path: config.tcc_config.libraries) {
        tcc_add_library(tcc_state_, path.c_str());
    }

    tcc_set_output_type(tcc_state_, TCC_OUTPUT_MEMORY);

    // 添加 JIT 代码可能调用的符号
    tcc_add_symbol(tcc_state_, "FakeluaAlloc", reinterpret_cast<void *>(FakeluaAlloc));
    tcc_add_symbol(tcc_state_, "FakeluaThrowError", reinterpret_cast<void *>(FakeluaThrowError));
    tcc_add_symbol(tcc_state_, "FakeluaCallByName", reinterpret_cast<void *>(FakeluaCallByName));
    tcc_add_symbol(tcc_state_, "FlEvalLoadClosure", reinterpret_cast<void *>(FlEvalLoadClosure));
    tcc_add_symbol(tcc_state_, "FakeluaLogLua", reinterpret_cast<void *>(FakeluaLogLua));
    tcc_add_symbol(tcc_state_, "GetLogLevel", reinterpret_cast<void *>(GetLogLevel));
    tcc_define_symbol(tcc_state_, "FAKELUA_JIT_TYPE", std::to_string(static_cast<int>(JIT_TCC)).c_str());
}

TCCHandle::~TCCHandle() {
    if (tcc_state_) {
        tcc_delete(tcc_state_);
        tcc_state_ = nullptr;
    }
}

}// namespace fakelua
