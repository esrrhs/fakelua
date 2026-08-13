// fuzz_bridge.cpp — Thin C wrapper implementation (compiled with GCC)
//
// Wraps fakelua C++ API behind simple C functions so fuzz targets
// compiled with clang+libFuzzer can call fakelua without needing its
// C++20 template headers.

#include "fuzz_bridge.h"
#include "fakelua.h"
#include "util/exception.h"

#include <cstdlib>
#include <stdexcept>
#include <string>

extern "C" {

void *fuzz_fakelua_new_state(void) {
    try {
        return fakelua::FakeluaNewState();
    } catch (...) {
        return nullptr;
    }
}

void fuzz_fakelua_delete_state(void *s) {
    if (!s) return;
    try {
        fakelua::FakeluaDeleteState(static_cast<fakelua::State *>(s));
    } catch (...) {
        // Destructor shouldn't throw, but be safe
    }
}

// debug_mode is off in both configs below: it dumps the generated C code and the
// type inference result to disk for every single input, which dominates runtime
// and fills up /tmp during a fuzz run.
static int CompileWith(void *s, const char *src, int len, const fakelua::CompileConfig &cfg) {
    if (!s || !src || len < 0) return 0;

    auto *state = static_cast<fakelua::State *>(s);
    std::string script(src, static_cast<size_t>(len));

    try {
        fakelua::CompileString(state, script, cfg);
        return 1; // success
    } catch (const fakelua::FakeluaException &) {
        return 0; // expected: invalid Lua / unsupported feature / runtime error in initializers
    } catch (const std::exception &) {
        // Unexpected exception type — likely a bug
        std::abort();
    } catch (...) {
        // Unknown exception — definitely a bug
        std::abort();
    }
}

int fuzz_fakelua_compile_string(void *s, const char *src, int len) {
    // Production path is GCC. Disable TCC so this target exercises the real
    // backend: C generation, GCC compile, and __fakelua_init execution.
    fakelua::CompileConfig cfg;
    cfg.debug_mode = false;
    cfg.disable_jit[fakelua::JIT_TCC] = true;
    return CompileWith(s, src, len, cfg);
}

int fuzz_fakelua_compile_string_executable(void *s, const char *src, int len) {
    // Same GCC-only config as fuzz_fakelua_compile_string; kept as a separate
    // entry point so differential fuzz documents that it needs a callable result.
    fakelua::CompileConfig cfg;
    cfg.debug_mode = false;
    cfg.disable_jit[fakelua::JIT_TCC] = true;
    return CompileWith(s, src, len, cfg);
}

int fuzz_fakelua_call_int(void *s, const char *name, int64_t *out) {
    if (!s || !name || !out) return 0;

    auto *state = static_cast<fakelua::State *>(s);

    try {
        fakelua::Call(state, fakelua::JIT_GCC, name, *out);
        return 1; // success
    } catch (const fakelua::FakeluaException &) {
        return 0; // expected runtime error
    } catch (const std::exception &) {
        std::abort();
    } catch (...) {
        std::abort();
    }
}

} // extern "C"
