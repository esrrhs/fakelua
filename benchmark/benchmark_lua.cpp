#include "benchmark/benchmark.h"
#include "fakelua.h"
#include "util/common.h"
#include <lua.hpp>// Lua C API

using namespace fakelua;

static void LoadLuaFile(lua_State *L, const std::string &file) {
    if (luaL_dofile(L, file.c_str()) != LUA_OK) {
        luaL_dofile(L, ("bin/" + file).c_str());
    }
}

template<typename T>
void PushToLua(lua_State *L, T value);

template<>
void PushToLua<int>(lua_State *L, int value) {
    lua_pushinteger(L, value);
}

template<typename T, typename... Args>
void PushArgs(lua_State *L, T first, Args... args) {
    PushToLua(L, first);
    PushArgs(L, args...);
}

void PushArgs(lua_State *L) {
}

template<typename... Args>
bool CallLuaFunc(lua_State *L, const std::string &funcName, int &ret, Args... args) {
    int top = lua_gettop(L);           // 获取栈顶位置
    lua_getglobal(L, funcName.c_str());// 获取函数
    DEBUG_ASSERT(lua_isfunction(L, -1));

    PushArgs(L, args...);

    constexpr int nargs = sizeof...(Args);
    int code = lua_pcall(L, nargs, 1, 0);
    DEBUG_ASSERT(code == LUA_OK);

    ret = lua_tointeger(L, -1);
    lua_settop(L, top);
    return true;
}

struct LuaGlobalIni {
    LuaGlobalIni() {
        L = luaL_newstate();
        luaL_openlibs(L);
        LoadLuaFile(L, "bench_algo/fibonacci.lua");
    }
    ~LuaGlobalIni() {
        if (L) {
            lua_close(L);
        }
    }
    lua_State *L = nullptr;
};

static LuaGlobalIni lua_global_ini;

static void BM_lua_fibonacci(benchmark::State &state) {
    for (auto _: state) {
        int ret = 0;
        CallLuaFunc(lua_global_ini.L, "main", ret, 30);
        if (ret != 832040) {
            throw std::runtime_error("Lua Fibonacci result is incorrect: " + std::to_string(ret));
        }
    }
}

BENCHMARK(BM_lua_fibonacci);
