// fuzz_compile.cpp — Compilation-only fuzz target for fakelua
//
// Uses fuzz_bridge (compiled with GCC) to call fakelua from a
// clang+libFuzzer binary. Tests that CompileString doesn't crash.
//
// Build:  See fuzz/README.md
// Run:    ./fuzz_compile -max_len=4096 -runs=1000000

#include "fuzz_bridge.h"

#include <cstddef>
#include <cstdint>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    void *state = fuzz_fakelua_new_state();
    if (!state) return 0;

    // fuzz_bridge aborts on unexpected exceptions; returns 0/1 for expected cases
    fuzz_fakelua_compile_string(state, reinterpret_cast<const char *>(data),
                                static_cast<int>(size));

    fuzz_fakelua_delete_state(state);
    return 0;
}
