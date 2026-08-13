// fuzz_differential.cpp — Differential fuzzing: fakelua vs Lua 5.4
//
// Same Lua script compiled in both fakelua and Lua 5.4.
// Mismatches (compilation success/failure, return values) are potential bugs.
//
// Strategy:
//   1. Wrap fuzz bytes into a known-named function "fuzz_test()"
//   2. Compile in both engines
//   3. If compilation differs (fakelua-accepts + Lua-rejects), report
//   4. If both compile, call fuzz_test() and compare results

#include "fuzz_bridge.h"

#include <lua.hpp>

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

// Filter obviously binary inputs (too many null bytes or non-printable chars)
static bool LooksLikeText(const uint8_t *data, size_t size) {
    if (size == 0) return false;
    size_t nulls = 0;
    size_t bad = 0;
    const size_t check_len = (size < 256) ? size : 256;
    for (size_t i = 0; i < check_len; ++i) {
        if (data[i] == 0) nulls++;
        unsigned char c = data[i];
        if (c < 0x20 && c != '\n' && c != '\r' && c != '\t') bad++;
    }
    if (nulls > check_len / 10 || bad > check_len / 3) return false;
    return true;
}

// Sanitize raw bytes into a printable ASCII string for Lua
static std::string SanitizeForLua(const uint8_t *data, size_t size) {
    std::string out;
    out.reserve(size);
    for (size_t i = 0; i < size; ++i) {
        unsigned char c = data[i];
        if (c >= 0x20 && c <= 0x7E) {
            out += static_cast<char>(c);
        } else if (c == '\n' || c == '\r' || c == '\t') {
            out += static_cast<char>(c);
        } else {
            out += ' ';
        }
    }
    return out;
}

// Wrap arbitrary text as body of a known function
static std::string WrapAsFuzzFunction(const std::string &body) {
    return "function fuzz_test()\n" + body + "\nreturn 0\nend";
}

// ---------------------------------------------------------------------------
// Fuzz entry point
// ---------------------------------------------------------------------------

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (!LooksLikeText(data, size)) return 0;

    std::string body = SanitizeForLua(data, size);
    if (body.empty()) return 0;
    std::string script = WrapAsFuzzFunction(body);

    // ---- fakelua ----
    int flua_compiled = 0;
    void *flua = fuzz_fakelua_new_state();
    if (flua) {
        // Executable variant: fuzz_test() has to be callable below. Lua 5.4 also
        // runs the chunk via lua_pcall, so both sides execute file-level code.
        flua_compiled = fuzz_fakelua_compile_string_executable(flua, script.c_str(),
                                                              static_cast<int>(script.size()));
    }

    // ---- Lua 5.4 ----
    int lua_compiled = 0;
    lua_State *L = luaL_newstate();
    if (L) {
        luaL_openlibs(L);
        if (luaL_loadstring(L, script.c_str()) == LUA_OK) {
            if (lua_pcall(L, 0, 0, 0) == LUA_OK) {
                lua_compiled = 1;
            }
        }
    }

    // ---- Compare compilation results ----
    // Lua-accepted + fakelua-rejected → expected (fakelua is a subset)
    // fakelua-accepted + Lua-rejected → INTERESTING (potential bug)
    if (flua_compiled && !lua_compiled) {
        fprintf(stderr, "\n[DIFF FUZZ] fakelua compiled but Lua 5.4 rejected:\n%s\n",
                script.c_str());
        std::abort();
    }

    // ---- If both compiled, compare execution ----
    if (flua_compiled && lua_compiled) {
        // fakelua call
        int64_t flua_ret = -999;
        int flua_called = fuzz_fakelua_call_int(flua, "fuzz_test", &flua_ret);

        // Lua call
        int64_t lua_ret = -999;
        int lua_called = 0;
        lua_getglobal(L, "fuzz_test");
        if (lua_isfunction(L, -1)) {
            if (lua_pcall(L, 0, 1, 0) == LUA_OK) {
                if (lua_isinteger(L, -1)) {
                    lua_ret = static_cast<int64_t>(lua_tointeger(L, -1));
                    lua_called = 1;
                }
            }
        }

        if (flua_called != lua_called) {
            fprintf(stderr, "\n[DIFF FUZZ] call success mismatch: flua=%d lua=%d\n%s\n",
                    flua_called, lua_called, script.c_str());
            std::abort();
        }
        if (flua_called && lua_called && flua_ret != lua_ret) {
            fprintf(stderr, "\n[DIFF FUZZ] return mismatch: flua=%ld lua=%ld\n%s\n",
                    (long)flua_ret, (long)lua_ret, script.c_str());
            std::abort();
        }
    }

    // Cleanup
    if (L) lua_close(L);
    if (flua) fuzz_fakelua_delete_state(flua);

    return 0;
}
