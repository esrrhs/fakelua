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

int fuzz_fakelua_compile_string(void *s, const char *src, int len) {
    if (!s || !src || len < 0) return 0;

    auto *state = static_cast<fakelua::State *>(s);
    std::string script(src, static_cast<size_t>(len));

    try {
        fakelua::CompileString(state, script, {.debug_mode = false});
        return 1; // success
    } catch (const fakelua::FakeluaException &) {
        return 0; // expected: invalid Lua / unsupported feature
    } catch (const std::exception &) {
        // Unexpected exception type — likely a bug
        std::abort();
    } catch (...) {
        // Unknown exception — definitely a bug
        std::abort();
    }
}

int fuzz_fakelua_call_int(void *s, const char *name, int64_t *out) {
    if (!s || !name || !out) return 0;

    auto *state = static_cast<fakelua::State *>(s);

    try {
        fakelua::Call(state, fakelua::JIT_TCC, name, *out);
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
