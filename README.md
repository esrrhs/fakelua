# FakeLua(Work in process)
[<img src="https://img.shields.io/github/license/esrrhs/fakelua">](https://github.com/esrrhs/fakelua)
[<img src="https://img.shields.io/github/languages/top/esrrhs/fakelua">](https://github.com/esrrhs/fakelua)
[<img src="https://img.shields.io/github/actions/workflow/status/esrrhs/fakelua/build.yml?branch=master">](https://github.com/esrrhs/fakelua/actions)
[![codecov](https://codecov.io/gh/esrrhs/fakelua/graph/badge.svg?token=9ZCUH1Q632)](https://codecov.io/gh/esrrhs/fakelua)

FakeLua is a subset of Lua that compiles code into native machine code at runtime using Just-In-Time (JIT) compilation, providing a streamlined language structure and improved performance.

# Features:
* Compiles with C++23
* Builds on Linux and MinGW
* Supports Lua 5.4 syntax
* Uses Flex and Bison for lexing and parsing
* Compiles to native code using GCC JIT
* Supports GDB for debugging Lua source code

# Differences from Lua:
* No global variables, only global constants
* No garbage collection (GC); uses a memory pool instead
* No coroutines, threads, userdata, or metatables
* Tables are exclusively hash tables, not array tables
* String concatenation supports all data types
* No closures, as they are slow and complex to implement


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
