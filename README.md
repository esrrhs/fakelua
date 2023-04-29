# FakeLua(Work in process)
[<img src="https://img.shields.io/github/license/esrrhs/fakelua">](https://github.com/esrrhs/fakelua)
[<img src="https://img.shields.io/github/languages/top/esrrhs/fakelua">](https://github.com/esrrhs/fakelua)
[<img src="https://img.shields.io/github/actions/workflow/status/esrrhs/fakelua/cmake.yml?branch=master">](https://github.com/esrrhs/fakelua/actions)

FakeLua is a tool that can optimize Lua execution, it compile Lua to native code at runtime and can execute concurrently.

# Feature
* compile with C++20
* lexing and parsing by Flex && Bison
* concurrent API, no global state, eg: _G
* use memory pool, no GC
* compile to native code by GCC JIT
* has a toy interpreter
* build on Linux && Mingw
