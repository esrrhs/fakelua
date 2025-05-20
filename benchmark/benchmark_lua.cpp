#include "benchmark/benchmark.h"
#include "fakelua.h"
#include "util/common.h"
#include <lua.hpp>// Lua C API

using namespace fakelua;

static lua_State *load_lua_file(const std::string &file) {
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    if (luaL_dofile(L, file.c_str()) != LUA_OK) {
        fprintf(stderr, "Error: %s\n", lua_tostring(L, -1));
        lua_close(L);
        return nullptr;
    }
    return L;
}

// 辅助：将C++类型推入Lua栈的模板函数
template<typename T>
void push_to_lua(lua_State *L, T value);

// 特化：int
template<>
void push_to_lua<int>(lua_State *L, int value) {
    lua_pushinteger(L, value);
}

// 特化：double
template<>
void push_to_lua<double>(lua_State *L, double value) {
    lua_pushnumber(L, value);
}

// 特化：const char*
template<>
void push_to_lua<const char *>(lua_State *L, const char *value) {
    lua_pushstring(L, value);
}

// 特化：std::string
template<>
void push_to_lua<std::string>(lua_State *L, std::string value) {
    lua_pushstring(L, value.c_str());
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
    if (!lua_isfunction(L, -1)) {
        std::cerr << "Error: " << funcName << " is not a function\n";
        lua_settop(L, top);
        return false;
    }

    // 压入参数
    push_args(L, args...);

    // 调用函数
    constexpr int nargs = sizeof...(Args);
    if (lua_pcall(L, nargs, 1, 0) != LUA_OK) {
        std::cerr << "Lua error: " << lua_tostring(L, -1) << "\n";
        lua_settop(L, top);
        return false;
    }

    ret = lua_tointeger(L, -1);// 获取返回值
    lua_settop(L, top);
    return true;
}

static void BM_fibonacci(benchmark::State &state) {
    auto L = load_lua_file("./algo/fibonacci.lua");
    if (L == nullptr) {
        state.SkipWithError("Failed to load Lua file");
        return;
    }
    int ret = 0;
    for (auto _: state) {
        call_lua_func(L, "fibonacci", ret, 30);
    }
    lua_close(L);
    state.SetItemsProcessed(state.iterations());
    state.SetLabel("fibonacci");
    std::cout << "Fibonacci result: " << ret << std::endl;
}

BENCHMARK(BM_fibonacci);
