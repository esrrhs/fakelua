# FakeLua (Work in progress)
[<img src="https://img.shields.io/github/license/esrrhs/fakelua">](https://github.com/esrrhs/fakelua)
[<img src="https://img.shields.io/github/languages/top/esrrhs/fakelua">](https://github.com/esrrhs/fakelua)
[<img src="https://img.shields.io/github/actions/workflow/status/esrrhs/fakelua/build.yml?branch=master">](https://github.com/esrrhs/fakelua/actions)
[![codecov](https://codecov.io/gh/esrrhs/fakelua/graph/badge.svg?token=9ZCUH1Q632)](https://codecov.io/gh/esrrhs/fakelua)

FakeLua is a high-performance, embeddable script engine that implements a subset of Lua 5.4 syntax. Its primary goal is to achieve extreme execution speed by Just-In-Time (JIT) compiling scripts into native machine code using `libgccjit`.

# Features:
* **Extreme Performance**: Compiles scripts directly into native machine code using the GCC JIT backend.
* **Lua Compatibility**: Supports core Lua 5.4 syntax and semantics.
* **Modern C++**: Built with C++23, balancing safety and performance.
* **Easily Embeddable**: Designed for seamless integration into C++ applications.
* **Debugging Support**: Supports direct debugging of Lua source code using GDB.
* **Lightweight**: Minimalist execution model without the overhead of heavy Garbage Collection (GC).

# Build
### Linux
* Dependencies: install gcc-13 with JIT support, cmake, flex, bison. Install Lua for testing.
* Build: 
```shell
mkdir build
cd build
cmake ..
make
```

### MinGW
* Dependencies: install [MSYS2](https://www.msys2.org). Install [mingw-w64-x86_64-libgccjit](https://packages.msys2.org/package/mingw-w64-x86_64-libgccjit), [mingw-w64-x86_64-lua](https://packages.msys2.org/package/mingw-w64-x86_64-lua). Install CLion.
* Open project with CLion, set MinGW toolchains, and build.
