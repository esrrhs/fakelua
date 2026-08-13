#pragma once

#include "benchmark/benchmark.h"
#include "fakelua.h"

#include <lua.hpp>

#include <cstdint>
#include <stdexcept>
#include <string>

using namespace fakelua;

// ---------------------------------------------------------------------------
// Lua argument helpers
// ---------------------------------------------------------------------------

void PushLuaArg(lua_State *L, int64_t value);
void PushLuaArg(lua_State *L, const std::string &value);

inline void PushLuaArgs(lua_State *) {}

template<typename T, typename... Args>
void PushLuaArgs(lua_State *L, T first, Args... args) {
    PushLuaArg(L, first);
    PushLuaArgs(L, args...);
}

// ---------------------------------------------------------------------------
// Lua call helpers
// ---------------------------------------------------------------------------

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
double CallLuaDouble(lua_State *L, const char *func_name, Args... args) {
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
    const auto ret = lua_tonumber(L, -1);
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
// RuntimeContext — creates and owns one Lua + one FakeLua state
// ---------------------------------------------------------------------------

struct RuntimeContext {
    /// Create Lua + FakeLua states and compile the given scripts in both.
    void Init(const char *const *scripts, size_t count);

    /// Destroy both states.
    void Destroy();

    lua_State *lua = nullptr;
    State *flua = nullptr;
};

// ---------------------------------------------------------------------------
// Verification helpers
// ---------------------------------------------------------------------------

void VerifyEqual(int64_t got, int64_t expected, const char *name);
void VerifyEqual(const std::string &got, const std::string &expected, const char *name);
void VerifyEqualDouble(double got, double expected, const char *name, double rtol = 1e-9, double atol = 1e-12);
