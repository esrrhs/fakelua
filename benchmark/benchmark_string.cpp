#include "benchmark/benchmark.h"
#include "fakelua.h"

#include <lua.hpp>

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <string>

using namespace fakelua;

namespace {

// ---------------------------------------------------------------------------
// Scripts
// ---------------------------------------------------------------------------

constexpr const char *kStringLenScript = R"(
function bench_string_len(s)
    return #s
end
)";

constexpr const char *kStringSubScript = R"(
function bench_string_sub(s, i, j)
    return string.sub(s, i, j)
end
)";

constexpr const char *kStringRepScript = R"(
function bench_string_rep(s, n)
    return string.rep(s, n)
end
)";

constexpr const char *kStringReverseScript = R"(
function bench_string_reverse(s)
    return string.reverse(s)
end
)";

constexpr const char *kStringLowerScript = R"(
function bench_string_lower(s)
    return string.lower(s)
end
)";

constexpr const char *kStringUpperScript = R"(
function bench_string_upper(s)
    return string.upper(s)
end
)";

constexpr const char *kStringByteScript = R"(
function bench_string_byte(s, i)
    return string.byte(s, i)
end
)";

constexpr const char *kStringCharScript = R"(
function bench_string_char(n)
    local t = {}
    for i = 1, n do
        t[i] = string.char((i % 95) + 32)
    end
    return table.concat(t)
end
)";

constexpr const char *kStringFormatScript = R"(
function bench_string_format(n)
    local s = ""
    for i = 1, n do
        s = string.format("%d", i)
    end
    return #s
end
)";

constexpr const char *kStringFindScript = R"(
function bench_string_find(s, pat)
    local pos = string.find(s, pat, 1, true)
    if pos then
        return pos
    end
    return 0
end
)";

constexpr const char *kStringGsubScript = R"(
function bench_string_gsub(s, from, to)
    local result, count = string.gsub(s, from, to)
    return count
end
)";

constexpr const char *kToNumberScript = R"(
function bench_tonumber(s)
    return tonumber(s) or 0
end
)";

constexpr const char *kToStringScript = R"(
function bench_tostring(n)
    return tostring(n)
end
)";

// ---------------------------------------------------------------------------
// C++ reference implementations
// ---------------------------------------------------------------------------

int64_t CppStringLen(const std::string &s) {
    return static_cast<int64_t>(s.size());
}

std::string CppStringSub(const std::string &s, int64_t i, int64_t j) {
    // Lua uses 1-based indexing, inclusive
    return s.substr(static_cast<size_t>(i - 1), static_cast<size_t>(j - i + 1));
}

std::string CppStringRep(const std::string &s, int64_t n) {
    std::string result;
    result.reserve(s.size() * static_cast<size_t>(n));
    for (int64_t k = 0; k < n; ++k) {
        result += s;
    }
    return result;
}

std::string CppStringReverse(const std::string &s) {
    std::string result(s);
    std::reverse(result.begin(), result.end());
    return result;
}

std::string CppStringLower(const std::string &s) {
    std::string result(s);
    for (char &c: result) {
        if (c >= 'A' && c <= 'Z') {
            c = static_cast<char>(c + ('a' - 'A'));
        }
    }
    return result;
}

std::string CppStringUpper(const std::string &s) {
    std::string result(s);
    for (char &c: result) {
        if (c >= 'a' && c <= 'z') {
            c = static_cast<char>(c - ('a' - 'A'));
        }
    }
    return result;
}

int64_t CppStringByte(const std::string &s, int64_t i) {
    return static_cast<int64_t>(static_cast<unsigned char>(s[static_cast<size_t>(i - 1)]));
}

std::string CppStringChar(int64_t n) {
    std::string result;
    result.reserve(static_cast<size_t>(n));
    for (int64_t k = 0; k < n; ++k) {
        result.push_back(static_cast<char>((k % 95) + 32));
    }
    return result;
}

int64_t CppStringFind(const std::string &s, const std::string &pat) {
    auto pos = s.find(pat);
    if (pos != std::string::npos) {
        return static_cast<int64_t>(pos) + 1; // 1-based
    }
    return 0;
}

int64_t CppStringGsub(const std::string &s, const std::string &from, const std::string &to) {
    int64_t count = 0;
    std::string result;
    size_t pos = 0;
    while (pos < s.size()) {
        auto found = s.find(from, pos);
        if (found == std::string::npos) {
            break;
        }
        ++count;
        pos = found + from.size();
    }
    return count;
}

// ---------------------------------------------------------------------------
// Lua helpers (extended for string args/returns)
// ---------------------------------------------------------------------------

void PushLuaArg(lua_State *L, int64_t value) {
    lua_pushinteger(L, static_cast<lua_Integer>(value));
}

void PushLuaArg(lua_State *L, const std::string &value) {
    lua_pushlstring(L, value.c_str(), value.size());
}

void PushLuaArgs(lua_State *) {}

template<typename T, typename... Args>
void PushLuaArgs(lua_State *L, T first, Args... args) {
    PushLuaArg(L, first);
    PushLuaArgs(L, args...);
}

template<typename... Args>
int64_t CallLuaInt(lua_State *L, const char *func_name, Args... args) {
    const int top = lua_gettop(L);
    lua_getglobal(L, func_name);
    if (!lua_isfunction(L, -1)) {
        lua_settop(L, top);
        throw std::runtime_error(std::string("Lua function not found: ") + func_name);
    }
    PushLuaArgs(L, std::forward<Args>(args)...);
    constexpr int nargs = sizeof...(Args);
    if (const int code = lua_pcall(L, nargs, 1, 0); code != LUA_OK) {
        const char *err = lua_tostring(L, -1);
        std::string msg = err ? err : "unknown lua error";
        lua_settop(L, top);
        throw std::runtime_error("Lua call failed: " + msg);
    }
    const auto ret = static_cast<int64_t>(lua_tointeger(L, -1));
    lua_settop(L, top);
    return ret;
}

template<typename... Args>
std::string CallLuaString(lua_State *L, const char *func_name, Args... args) {
    const int top = lua_gettop(L);
    lua_getglobal(L, func_name);
    if (!lua_isfunction(L, -1)) {
        lua_settop(L, top);
        throw std::runtime_error(std::string("Lua function not found: ") + func_name);
    }
    PushLuaArgs(L, std::forward<Args>(args)...);
    constexpr int nargs = sizeof...(Args);
    if (const int code = lua_pcall(L, nargs, 1, 0); code != LUA_OK) {
        const char *err = lua_tostring(L, -1);
        std::string msg = err ? err : "unknown lua error";
        lua_settop(L, top);
        throw std::runtime_error("Lua call failed: " + msg);
    }
    size_t len = 0;
    const char *s = lua_tolstring(L, -1, &len);
    std::string result(s, len);
    lua_settop(L, top);
    return result;
}

// ---------------------------------------------------------------------------
// RuntimeContext — one Lua + one FakeLua state for string benchmarks
// ---------------------------------------------------------------------------

struct RuntimeContext {
    RuntimeContext() {
        lua = luaL_newstate();
        luaL_openlibs(lua);

        const char *lua_scripts[] = {
            kStringLenScript,     kStringSubScript,   kStringRepScript,
            kStringReverseScript, kStringLowerScript, kStringUpperScript,
            kStringByteScript,    kStringCharScript,  kStringFormatScript,
            kStringFindScript,    kStringGsubScript,  kToNumberScript,
            kToStringScript,
        };
        for (const char *script: lua_scripts) {
            if (luaL_dostring(lua, script) != LUA_OK) {
                const char *err = lua_tostring(lua, -1);
                throw std::runtime_error(std::string("init lua scripts failed: ") + (err ? err : "unknown"));
            }
        }

        flua = FakeluaNewState();
        for (const char *script: lua_scripts) {
            CompileString(flua, script, {.debug_mode = false});
        }

        // Warmup: call each function once (TCC only — GCC warmup is too slow)
        int64_t warmup_int = 0;
        std::string warmup_str;
        std::string test_s = std::string(100, 'a');
        Call(flua, JIT_TCC, "bench_string_len", warmup_int, test_s);
        Call(flua, JIT_TCC, "bench_string_sub", warmup_str, test_s, 26, 75);
        Call(flua, JIT_TCC, "bench_string_rep", warmup_str, std::string("x"), 10);
        Call(flua, JIT_TCC, "bench_string_reverse", warmup_str, test_s);
        Call(flua, JIT_TCC, "bench_string_lower", warmup_str, test_s);
        Call(flua, JIT_TCC, "bench_string_upper", warmup_str, test_s);
        Call(flua, JIT_TCC, "bench_string_byte", warmup_int, test_s, 50);
        Call(flua, JIT_TCC, "bench_string_format", warmup_int, 10);
        Call(flua, JIT_TCC, "bench_string_find", warmup_int, test_s, std::string("aaa"));
        Call(flua, JIT_TCC, "bench_string_gsub", warmup_int, test_s, std::string("a"), std::string("b"));
        Call(flua, JIT_TCC, "bench_tonumber", warmup_int, std::string("12345"));
        Call(flua, JIT_TCC, "bench_tostring", warmup_str, 12345);
    }

    ~RuntimeContext() {
        if (lua) {
            lua_close(lua);
            lua = nullptr;
        }
        if (flua) {
            FakeluaDeleteState(flua);
            flua = nullptr;
        }
    }

    lua_State *lua = nullptr;
    State *flua = nullptr;
};

RuntimeContext g_ctx;

void VerifyEqual(int64_t got, int64_t expected, const char *name) {
    if (got != expected) {
        throw std::runtime_error(std::string(name) + " wrong result: got " + std::to_string(got) + ", expected " + std::to_string(expected));
    }
}

void VerifyEqual(const std::string &got, const std::string &expected, const char *name) {
    if (got != expected) {
        throw std::runtime_error(std::string(name) + " wrong result: got \"" + got + "\", expected \"" + expected + "\"");
    }
}

// ---------------------------------------------------------------------------
// Benchmarks: string.len
// ---------------------------------------------------------------------------

static void BM_CPP_StringLen(benchmark::State &state) {
    const int64_t len = state.range(0);
    std::string s(static_cast<size_t>(len), 'a');
    const int64_t expected = len;
    for (auto _: state) {
        int64_t ret = static_cast<int64_t>(s.size());
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "C++ string.len");
    }
}

static void BM_Lua_StringLen(benchmark::State &state) {
    const int64_t len = state.range(0);
    std::string s(static_cast<size_t>(len), 'a');
    for (auto _: state) {
        int64_t ret = CallLuaInt(g_ctx.lua, "bench_string_len", s);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_StringLen_TCC(benchmark::State &state) {
    const int64_t len = state.range(0);
    std::string s(static_cast<size_t>(len), 'a');
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_string_len", ret, s);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_StringLen_GCC(benchmark::State &state) {
    const int64_t len = state.range(0);
    std::string s(static_cast<size_t>(len), 'a');
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_string_len", ret, s);
        benchmark::DoNotOptimize(ret);
    }
}

// ---------------------------------------------------------------------------
// Benchmarks: string.sub
// ---------------------------------------------------------------------------

static void BM_CPP_StringSub(benchmark::State &state) {
    const int64_t len = state.range(0);
    std::string s(static_cast<size_t>(len), 'a');
    const int64_t i = len / 4 + 1;
    const int64_t j = len / 4 + len / 2;
    const std::string expected = CppStringSub(s, i, j);
    for (auto _: state) {
        std::string ret = CppStringSub(s, i, j);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "C++ string.sub");
    }
}

static void BM_Lua_StringSub(benchmark::State &state) {
    const int64_t len = state.range(0);
    std::string s(static_cast<size_t>(len), 'a');
    const int64_t i = len / 4 + 1;
    const int64_t j = len / 4 + len / 2;
    for (auto _: state) {
        std::string ret = CallLuaString(g_ctx.lua, "bench_string_sub", s, i, j);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_StringSub_TCC(benchmark::State &state) {
    const int64_t len = state.range(0);
    std::string s(static_cast<size_t>(len), 'a');
    const int64_t i = len / 4 + 1;
    const int64_t j = len / 4 + len / 2;
    for (auto _: state) {
        std::string ret;
        Call(g_ctx.flua, JIT_TCC, "bench_string_sub", ret, s, i, j);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_StringSub_GCC(benchmark::State &state) {
    const int64_t len = state.range(0);
    std::string s(static_cast<size_t>(len), 'a');
    const int64_t i = len / 4 + 1;
    const int64_t j = len / 4 + len / 2;
    for (auto _: state) {
        std::string ret;
        Call(g_ctx.flua, JIT_GCC, "bench_string_sub", ret, s, i, j);
        benchmark::DoNotOptimize(ret);
    }
}

// ---------------------------------------------------------------------------
// Benchmarks: string.rep
// ---------------------------------------------------------------------------

static void BM_CPP_StringRep(benchmark::State &state) {
    const int64_t n = state.range(0);
    std::string s("c");
    const std::string expected = CppStringRep(s, n);
    for (auto _: state) {
        std::string ret = CppStringRep(s, n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "C++ string.rep");
    }
}

static void BM_Lua_StringRep(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        std::string ret = CallLuaString(g_ctx.lua, "bench_string_rep", std::string("c"), n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_StringRep_TCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        std::string ret;
        Call(g_ctx.flua, JIT_TCC, "bench_string_rep", ret, std::string("c"), n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_StringRep_GCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        std::string ret;
        Call(g_ctx.flua, JIT_GCC, "bench_string_rep", ret, std::string("c"), n);
        benchmark::DoNotOptimize(ret);
    }
}

// ---------------------------------------------------------------------------
// Benchmarks: string.reverse
// ---------------------------------------------------------------------------

static void BM_CPP_StringReverse(benchmark::State &state) {
    const int64_t len = state.range(0);
    std::string s(static_cast<size_t>(len), 'a');
    const std::string expected = CppStringReverse(s);
    for (auto _: state) {
        std::string ret = CppStringReverse(s);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "C++ string.reverse");
    }
}

static void BM_Lua_StringReverse(benchmark::State &state) {
    const int64_t len = state.range(0);
    std::string s(static_cast<size_t>(len), 'a');
    for (auto _: state) {
        std::string ret = CallLuaString(g_ctx.lua, "bench_string_reverse", s);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_StringReverse_TCC(benchmark::State &state) {
    const int64_t len = state.range(0);
    std::string s(static_cast<size_t>(len), 'a');
    for (auto _: state) {
        std::string ret;
        Call(g_ctx.flua, JIT_TCC, "bench_string_reverse", ret, s);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_StringReverse_GCC(benchmark::State &state) {
    const int64_t len = state.range(0);
    std::string s(static_cast<size_t>(len), 'a');
    for (auto _: state) {
        std::string ret;
        Call(g_ctx.flua, JIT_GCC, "bench_string_reverse", ret, s);
        benchmark::DoNotOptimize(ret);
    }
}

// ---------------------------------------------------------------------------
// Benchmarks: string.lower
// ---------------------------------------------------------------------------

static void BM_CPP_StringLower(benchmark::State &state) {
    const int64_t len = state.range(0);
    std::string s(static_cast<size_t>(len), 'A');
    const std::string expected(len, 'a');
    for (auto _: state) {
        std::string ret = CppStringLower(s);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "C++ string.lower");
    }
}

static void BM_Lua_StringLower(benchmark::State &state) {
    const int64_t len = state.range(0);
    std::string s(static_cast<size_t>(len), 'A');
    for (auto _: state) {
        std::string ret = CallLuaString(g_ctx.lua, "bench_string_lower", s);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_StringLower_TCC(benchmark::State &state) {
    const int64_t len = state.range(0);
    std::string s(static_cast<size_t>(len), 'A');
    for (auto _: state) {
        std::string ret;
        Call(g_ctx.flua, JIT_TCC, "bench_string_lower", ret, s);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_StringLower_GCC(benchmark::State &state) {
    const int64_t len = state.range(0);
    std::string s(static_cast<size_t>(len), 'A');
    for (auto _: state) {
        std::string ret;
        Call(g_ctx.flua, JIT_GCC, "bench_string_lower", ret, s);
        benchmark::DoNotOptimize(ret);
    }
}

// ---------------------------------------------------------------------------
// Benchmarks: string.upper
// ---------------------------------------------------------------------------

static void BM_CPP_StringUpper(benchmark::State &state) {
    const int64_t len = state.range(0);
    std::string s(static_cast<size_t>(len), 'a');
    const std::string expected(len, 'A');
    for (auto _: state) {
        std::string ret = CppStringUpper(s);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "C++ string.upper");
    }
}

static void BM_Lua_StringUpper(benchmark::State &state) {
    const int64_t len = state.range(0);
    std::string s(static_cast<size_t>(len), 'a');
    for (auto _: state) {
        std::string ret = CallLuaString(g_ctx.lua, "bench_string_upper", s);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_StringUpper_TCC(benchmark::State &state) {
    const int64_t len = state.range(0);
    std::string s(static_cast<size_t>(len), 'a');
    for (auto _: state) {
        std::string ret;
        Call(g_ctx.flua, JIT_TCC, "bench_string_upper", ret, s);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_StringUpper_GCC(benchmark::State &state) {
    const int64_t len = state.range(0);
    std::string s(static_cast<size_t>(len), 'a');
    for (auto _: state) {
        std::string ret;
        Call(g_ctx.flua, JIT_GCC, "bench_string_upper", ret, s);
        benchmark::DoNotOptimize(ret);
    }
}

// ---------------------------------------------------------------------------
// Benchmarks: string.byte
// ---------------------------------------------------------------------------

static void BM_CPP_StringByte(benchmark::State &state) {
    const int64_t len = state.range(0);
    std::string s(static_cast<size_t>(len), 'a');
    const int64_t expected = 'a';
    for (auto _: state) {
        int64_t ret = CppStringByte(s, len / 2);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "C++ string.byte");
    }
}

static void BM_Lua_StringByte(benchmark::State &state) {
    const int64_t len = state.range(0);
    std::string s(static_cast<size_t>(len), 'a');
    for (auto _: state) {
        int64_t ret = CallLuaInt(g_ctx.lua, "bench_string_byte", s, len / 2);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_StringByte_TCC(benchmark::State &state) {
    const int64_t len = state.range(0);
    std::string s(static_cast<size_t>(len), 'a');
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_string_byte", ret, s, len / 2);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_StringByte_GCC(benchmark::State &state) {
    const int64_t len = state.range(0);
    std::string s(static_cast<size_t>(len), 'a');
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_string_byte", ret, s, len / 2);
        benchmark::DoNotOptimize(ret);
    }
}

// ---------------------------------------------------------------------------
// Benchmarks: string.char
// ---------------------------------------------------------------------------

static void BM_CPP_StringChar(benchmark::State &state) {
    const int64_t n = state.range(0);
    const std::string expected = CppStringChar(n);
    for (auto _: state) {
        std::string ret = CppStringChar(n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "C++ string.char");
    }
}

// string.char is variadic — use a loop in Lua
static void BM_Lua_StringChar(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        // Build a call string since Lua string.char is variadic
        std::string ret = CallLuaString(g_ctx.lua, "bench_string_char", n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_StringChar_TCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        std::string ret;
        Call(g_ctx.flua, JIT_TCC, "bench_string_char", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_StringChar_GCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        std::string ret;
        Call(g_ctx.flua, JIT_GCC, "bench_string_char", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

// ---------------------------------------------------------------------------
// Benchmarks: string.format
// ---------------------------------------------------------------------------

static void BM_CPP_StringFormat(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t total = 0;
        for (int64_t i = 1; i <= n; ++i) {
            total += static_cast<int64_t>(std::to_string(i).size());
        }
        benchmark::DoNotOptimize(total);
    }
}

static void BM_Lua_StringFormat(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = CallLuaInt(g_ctx.lua, "bench_string_format", n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_StringFormat_TCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_string_format", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_StringFormat_GCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_string_format", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

// ---------------------------------------------------------------------------
// Benchmarks: string.find (plain match)
// ---------------------------------------------------------------------------

static void BM_CPP_StringFind(benchmark::State &state) {
    const int64_t len = state.range(0);
    std::string s(static_cast<size_t>(len/2), 'a');
    s += "needle";
    s += std::string(static_cast<size_t>(len/2), 'a');
    for (auto _: state) {
        int64_t ret = CppStringFind(s, std::string("needle"));
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_Lua_StringFind(benchmark::State &state) {
    const int64_t len = state.range(0);
    std::string s(static_cast<size_t>(len/2), 'a');
    s += "needle";
    s += std::string(static_cast<size_t>(len/2), 'a');
    for (auto _: state) {
        int64_t ret = CallLuaInt(g_ctx.lua, "bench_string_find", s, std::string("needle"));
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_StringFind_TCC(benchmark::State &state) {
    const int64_t len = state.range(0);
    std::string s(static_cast<size_t>(len/2), 'a');
    s += "needle";
    s += std::string(static_cast<size_t>(len/2), 'a');
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_string_find", ret, s, std::string("needle"));
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_StringFind_GCC(benchmark::State &state) {
    const int64_t len = state.range(0);
    std::string s(static_cast<size_t>(len/2), 'a');
    s += "needle";
    s += std::string(static_cast<size_t>(len/2), 'a');
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_string_find", ret, s, std::string("needle"));
        benchmark::DoNotOptimize(ret);
    }
}

// ---------------------------------------------------------------------------
// Benchmarks: string.gsub
// ---------------------------------------------------------------------------

static void BM_CPP_StringGsub(benchmark::State &state) {
    const int64_t n = state.range(0);
    std::string s(static_cast<size_t>(n), 'a');
    const std::string from("a");
    const std::string to("b");
    for (auto _: state) {
        int64_t ret = CppStringGsub(s, from, to);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_Lua_StringGsub(benchmark::State &state) {
    const int64_t n = state.range(0);
    std::string s(static_cast<size_t>(n), 'a');
    for (auto _: state) {
        int64_t ret = CallLuaInt(g_ctx.lua, "bench_string_gsub", s, std::string("a"), std::string("b"));
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_StringGsub_TCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    std::string s(static_cast<size_t>(n), 'a');
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_string_gsub", ret, s, std::string("a"), std::string("b"));
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_StringGsub_GCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    std::string s(static_cast<size_t>(n), 'a');
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_string_gsub", ret, s, std::string("a"), std::string("b"));
        benchmark::DoNotOptimize(ret);
    }
}

// ---------------------------------------------------------------------------
// Benchmarks: tonumber
// ---------------------------------------------------------------------------

static void BM_CPP_ToNumber(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t total = 0;
        for (int64_t i = 1; i <= n; ++i) {
            total += std::stoll(std::to_string(i));
        }
        benchmark::DoNotOptimize(total);
    }
}

static void BM_Lua_ToNumber(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        // Call tonumber on a string representation of n
        std::string s = std::to_string(n);
        int64_t ret = CallLuaInt(g_ctx.lua, "bench_tonumber", s);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_ToNumber_TCC(benchmark::State &state) {
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_tonumber", ret, std::string("1234567890"));
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_ToNumber_GCC(benchmark::State &state) {
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_tonumber", ret, std::string("1234567890"));
        benchmark::DoNotOptimize(ret);
    }
}

// ---------------------------------------------------------------------------
// Benchmarks: tostring
// ---------------------------------------------------------------------------

static void BM_CPP_ToString(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        std::string total;
        for (int64_t i = 1; i <= n; ++i) {
            total += std::to_string(i);
        }
        benchmark::DoNotOptimize(total);
    }
}

static void BM_Lua_ToString(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = CallLuaInt(g_ctx.lua, "bench_tostring", n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_ToString_TCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_tostring", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

static void BM_FakeLua_ToString_GCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for (auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_tostring", ret, n);
        benchmark::DoNotOptimize(ret);
    }
}

} // namespace

// ---------------------------------------------------------------------------
// Benchmark registrations
// ---------------------------------------------------------------------------

#define STRING_LEN_ARGS ->Arg(10)->Arg(100)->Arg(1000)->Arg(10000)
#define STRING_SUB_ARGS  ->Arg(10)->Arg(100)->Arg(1000)->Arg(10000)
#define STRING_REP_ARGS  ->Arg(10)->Arg(100)->Arg(1000)
#define STRING_REVERSE_ARGS ->Arg(10)->Arg(100)->Arg(1000)->Arg(10000)
#define STRING_LOWER_ARGS ->Arg(10)->Arg(100)->Arg(1000)->Arg(10000)
#define STRING_UPPER_ARGS ->Arg(10)->Arg(100)->Arg(1000)->Arg(10000)
#define STRING_BYTE_ARGS ->Arg(10)->Arg(100)->Arg(1000)
#define STRING_CHAR_ARGS ->Arg(10)->Arg(100)->Arg(500)
#define STRING_FORMAT_ARGS ->Arg(10)->Arg(100)->Arg(500)
#define STRING_FIND_ARGS ->Arg(10)->Arg(100)->Arg(1000)->Arg(10000)
#define STRING_GSUB_ARGS ->Arg(10)->Arg(100)->Arg(1000)
#define TONUMBER_ARGS ->Arg(1)
#define TOSTRING_ARGS ->Arg(10)->Arg(100)->Arg(500)

BENCHMARK(BM_CPP_StringLen) STRING_LEN_ARGS;
BENCHMARK(BM_Lua_StringLen) STRING_LEN_ARGS;
BENCHMARK(BM_FakeLua_StringLen_TCC) STRING_LEN_ARGS;
BENCHMARK(BM_FakeLua_StringLen_GCC) STRING_LEN_ARGS;

BENCHMARK(BM_CPP_StringSub) STRING_SUB_ARGS;
BENCHMARK(BM_Lua_StringSub) STRING_SUB_ARGS;
BENCHMARK(BM_FakeLua_StringSub_TCC) STRING_SUB_ARGS;
BENCHMARK(BM_FakeLua_StringSub_GCC) STRING_SUB_ARGS;

BENCHMARK(BM_CPP_StringRep) STRING_REP_ARGS;
BENCHMARK(BM_Lua_StringRep) STRING_REP_ARGS;
BENCHMARK(BM_FakeLua_StringRep_TCC) STRING_REP_ARGS;
BENCHMARK(BM_FakeLua_StringRep_GCC) STRING_REP_ARGS;

BENCHMARK(BM_CPP_StringReverse) STRING_REVERSE_ARGS;
BENCHMARK(BM_Lua_StringReverse) STRING_REVERSE_ARGS;
BENCHMARK(BM_FakeLua_StringReverse_TCC) STRING_REVERSE_ARGS;
BENCHMARK(BM_FakeLua_StringReverse_GCC) STRING_REVERSE_ARGS;

BENCHMARK(BM_CPP_StringLower) STRING_LOWER_ARGS;
BENCHMARK(BM_Lua_StringLower) STRING_LOWER_ARGS;
BENCHMARK(BM_FakeLua_StringLower_TCC) STRING_LOWER_ARGS;
BENCHMARK(BM_FakeLua_StringLower_GCC) STRING_LOWER_ARGS;

BENCHMARK(BM_CPP_StringUpper) STRING_UPPER_ARGS;
BENCHMARK(BM_Lua_StringUpper) STRING_UPPER_ARGS;
BENCHMARK(BM_FakeLua_StringUpper_TCC) STRING_UPPER_ARGS;
BENCHMARK(BM_FakeLua_StringUpper_GCC) STRING_UPPER_ARGS;

BENCHMARK(BM_CPP_StringByte) STRING_BYTE_ARGS;
BENCHMARK(BM_Lua_StringByte) STRING_BYTE_ARGS;
BENCHMARK(BM_FakeLua_StringByte_TCC) STRING_BYTE_ARGS;
BENCHMARK(BM_FakeLua_StringByte_GCC) STRING_BYTE_ARGS;

BENCHMARK(BM_CPP_StringChar) STRING_CHAR_ARGS;
BENCHMARK(BM_Lua_StringChar) STRING_CHAR_ARGS;
BENCHMARK(BM_FakeLua_StringChar_TCC) STRING_CHAR_ARGS;
BENCHMARK(BM_FakeLua_StringChar_GCC) STRING_CHAR_ARGS;

BENCHMARK(BM_CPP_StringFormat) STRING_FORMAT_ARGS;
BENCHMARK(BM_Lua_StringFormat) STRING_FORMAT_ARGS;
BENCHMARK(BM_FakeLua_StringFormat_TCC) STRING_FORMAT_ARGS;
BENCHMARK(BM_FakeLua_StringFormat_GCC) STRING_FORMAT_ARGS;

BENCHMARK(BM_CPP_StringFind) STRING_FIND_ARGS;
BENCHMARK(BM_Lua_StringFind) STRING_FIND_ARGS;
BENCHMARK(BM_FakeLua_StringFind_TCC) STRING_FIND_ARGS;
BENCHMARK(BM_FakeLua_StringFind_GCC) STRING_FIND_ARGS;

BENCHMARK(BM_CPP_StringGsub) STRING_GSUB_ARGS;
BENCHMARK(BM_Lua_StringGsub) STRING_GSUB_ARGS;
BENCHMARK(BM_FakeLua_StringGsub_TCC) STRING_GSUB_ARGS;
BENCHMARK(BM_FakeLua_StringGsub_GCC) STRING_GSUB_ARGS;

BENCHMARK(BM_CPP_ToNumber) TONUMBER_ARGS;
BENCHMARK(BM_Lua_ToNumber) TONUMBER_ARGS;
BENCHMARK(BM_FakeLua_ToNumber_TCC) TONUMBER_ARGS;
BENCHMARK(BM_FakeLua_ToNumber_GCC) TONUMBER_ARGS;

BENCHMARK(BM_CPP_ToString) TOSTRING_ARGS;
BENCHMARK(BM_Lua_ToString) TOSTRING_ARGS;
BENCHMARK(BM_FakeLua_ToString_TCC) TOSTRING_ARGS;
BENCHMARK(BM_FakeLua_ToString_GCC) TOSTRING_ARGS;
