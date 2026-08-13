#include "benchmark_common.h"

#include <cmath>

// ---------------------------------------------------------------------------
// PushLuaArg implementations
// ---------------------------------------------------------------------------

void PushLuaArg(lua_State *L, int64_t value) {
    lua_pushinteger(L, static_cast<lua_Integer>(value));
}

void PushLuaArg(lua_State *L, const std::string &value) {
    lua_pushlstring(L, value.c_str(), value.size());
}

// ---------------------------------------------------------------------------
// RuntimeContext
// ---------------------------------------------------------------------------

void RuntimeContext::Init(const char *const *scripts, size_t count) {
    lua = luaL_newstate();
    luaL_openlibs(lua);

    for (size_t i = 0; i < count; ++i) {
        if (luaL_dostring(lua, scripts[i]) != LUA_OK) {
            const char *err = lua_tostring(lua, -1);
            throw std::runtime_error(std::string("init lua scripts failed: ") + (err ? err : "unknown"));
        }
    }

    flua = FakeluaNewState();
    for (size_t i = 0; i < count; ++i) {
        CompileString(flua, scripts[i], {.debug_mode = false});
    }
}

void RuntimeContext::Destroy() {
    if (lua) {
        lua_close(lua);
        lua = nullptr;
    }
    if (flua) {
        FakeluaDeleteState(flua);
        flua = nullptr;
    }
}

// ---------------------------------------------------------------------------
// VerifyEqual implementations
// ---------------------------------------------------------------------------

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

void VerifyEqualDouble(double got, double expected, const char *name, double rtol, double atol) {
    const double diff = std::fabs(got - expected);
    const double scale = std::fabs(expected) > 1.0 ? std::fabs(expected) : 1.0;
    if (diff > atol + rtol * scale) {
        throw std::runtime_error(std::string(name) + " wrong result: got " + std::to_string(got) + ", expected " + std::to_string(expected));
    }
}
