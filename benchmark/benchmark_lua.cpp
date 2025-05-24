#include "benchmark/benchmark.h"
#include "fakelua.h"
#include "util/common.h"
#include <lua.hpp>// Lua C API

using namespace fakelua;

static void load_lua_file(lua_State *L, const std::string &file) {
    if (luaL_dofile(L, file.c_str()) != LUA_OK) {
        luaL_dofile(L, ("bin/" + file).c_str());
    }
}

// 辅助：将C++类型推入Lua栈的模板函数
template<typename T>
void push_to_lua(lua_State *L, T value);

// 特化：int
template<>
void push_to_lua<int>(lua_State *L, int value) {
    lua_pushinteger(L, value);
}

// 递归展开参数包，将参数推入Lua栈
template<typename T, typename... Args>
void push_args(lua_State *L, T first, Args... args) {
    push_to_lua(L, first);
    push_args(L, args...);
}

// 终止条件
void push_args(lua_State *L) {
}

// 通用调用Lua函数模板
template<typename... Args>
bool call_lua_func(lua_State *L, const std::string &funcName, int &ret, Args... args) {
    int top = lua_gettop(L);           // 获取栈顶位置
    lua_getglobal(L, funcName.c_str());// 获取函数

    // 压入参数
    push_args(L, args...);

    // 调用函数
    constexpr int nargs = sizeof...(Args);
    lua_pcall(L, nargs, 1, 0);

    ret = lua_tointeger(L, -1);// 获取返回值
    lua_settop(L, top);
    return true;
}

struct LuaGlobalIni {
    LuaGlobalIni() {
        L = luaL_newstate();
        luaL_openlibs(L);
        load_lua_file(L, "bench_algo/fibonacci.lua");
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
    int ret = 0;
    for (auto _: state) {
        call_lua_func(lua_global_ini.L, "fibonacci", ret, 30);
    }
}

BENCHMARK(BM_lua_fibonacci);
