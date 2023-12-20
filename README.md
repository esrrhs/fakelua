# FakeLua(Work in process)
[<img src="https://img.shields.io/github/license/esrrhs/fakelua">](https://github.com/esrrhs/fakelua)
[<img src="https://img.shields.io/github/languages/top/esrrhs/fakelua">](https://github.com/esrrhs/fakelua)
[<img src="https://img.shields.io/github/actions/workflow/status/esrrhs/fakelua/build.yml?branch=master">](https://github.com/esrrhs/fakelua/actions)
[![codecov](https://codecov.io/gh/esrrhs/fakelua/graph/badge.svg?token=9ZCUH1Q632)](https://codecov.io/gh/esrrhs/fakelua)

FakeLua is a library for fast implementation of subset Lua, it compiles Lua to native code at runtime.

# Feature
* compile with C++20
* build on Linux && Mingw
* support Lua 5.4 grammar
* lexing and parsing by Flex && Bison
* compile to native code by GCC JIT
* can GDB with source code

# Difference with Lua
* no global variable, only global constant
* no GC, use memory pool
* no coroutine/thread/userdata
* table are always hash table, no array table

# Directory structure
* [include](./include) header file
* [src](./src) source code
* [test](./test) test code
* [benchmark](./benchmark) benchmark code
* [cmd](./cmd) command line tool
* [cmake](./cmake) cmake toolchain

# Build
### Linux
* Dependent: install gcc-13 with jit, cmake, flex, bison. install Lua for test.
* Build: 
```shell
mkdir build
cd build
cmake ..
make
```

### Mingw
* Dependent: install [MSYS2](https://www.msys2.org). install [mingw-w64-x86_64-libgccjit](https://packages.msys2.org/package/mingw-w64-x86_64-libgccjit), [mingw-w64-x86_64-lua](https://packages.msys2.org/package/mingw-w64-x86_64-lua). install CLion.
* open project with CLion, set mingw toolchains. and build.
