# FakeLua(Work in process)
[<img src="https://img.shields.io/github/license/esrrhs/fakelua">](https://github.com/esrrhs/fakelua)
[<img src="https://img.shields.io/github/languages/top/esrrhs/fakelua">](https://github.com/esrrhs/fakelua)
[<img src="https://img.shields.io/github/actions/workflow/status/esrrhs/fakelua/build.yml?branch=master">](https://github.com/esrrhs/fakelua/actions)
[![codecov](https://codecov.io/gh/esrrhs/fakelua/graph/badge.svg?token=9ZCUH1Q632)](https://codecov.io/gh/esrrhs/fakelua)

FakeLua is a library for fast implementation of custom Lua, it compile Lua to native code at runtime and can execute concurrently.

# Feature
* compile with C++20
* build on Linux && Mingw
* support Lua 5.4 grammar
* lexing and parsing by Flex && Bison
* concurrent API, no global state, eg: _G
* use memory pool, no GC
* has a toy interpreter
* compile to native code by GCC JIT

# Directory structure
* [include](./include) header file
* [src](./src) source code
* [test](./test) test code
* [benchmark](./benchmark) benchmark code
* [cmd](./cmd) command line tool
* [cmake](./cmake) cmake toolchain
