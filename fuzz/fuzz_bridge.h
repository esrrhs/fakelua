// fuzz_bridge.h — Thin C wrapper around fakelua C++ API
//
// Compiled with GCC (supports <format> and C++20 features).
// Fuzz targets compiled with clang + libFuzzer only include this header,
// avoiding the need for clang to handle fakelua's C++ template headers.
#pragma once

#include <cstddef>
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

// State management
void *fuzz_fakelua_new_state(void);
void fuzz_fakelua_delete_state(void *s);

// Compile Lua source code.
// Returns 1 on success, 0 on expected failure (invalid syntax/unsupported feature),
// aborts on unexpected failure (crash / unknown exception).
int fuzz_fakelua_compile_string(void *s, const char *src, int len);

// Call a named function with no arguments, expecting an int64_t return.
// Returns 1 on success, 0 on expected failure, aborts on unexpected failure.
int fuzz_fakelua_call_int(void *s, const char *name, int64_t *out);

#ifdef __cplusplus
}
#endif
