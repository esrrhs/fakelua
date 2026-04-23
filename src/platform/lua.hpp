// lua.hpp - C++ wrapper for lua.h
// This file provides compatibility for systems that don't ship lua.hpp
// (e.g., macOS Homebrew lua package).
// The official Lua 5.4 distribution includes lua.hpp with extern "C" wrapping.

#ifndef LUA_HPP
#define LUA_HPP

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#endif // LUA_HPP