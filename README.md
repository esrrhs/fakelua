# FakeLua (开发中)
[<img src="https://img.shields.io/github/license/esrrhs/fakelua">](https://github.com/esrrhs/fakelua)
[<img src="https://img.shields.io/github/languages/top/esrrhs/fakelua">](https://github.com/esrrhs/fakelua)
[<img src="https://img.shields.io/github/actions/workflow/status/esrrhs/fakelua/build.yml?branch=master">](https://github.com/esrrhs/fakelua/actions)
[![codecov](https://codecov.io/gh/esrrhs/fakelua/graph/badge.svg?token=9ZCUH1Q632)](https://codecov.io/gh/esrrhs/fakelua)

FakeLua 是一个高性能、可嵌入的脚本执行引擎，实现了 Lua 5.4 语法的子集。其核心目标是将 Lua 脚本转译为 C 代码，并利用现有的 C 编译器（如 TCC、GCC 或 Clang）将其编译为原生机器码，从而提供极致的运行速度。

# 特性 (Features):
* **极致性能**: 将 Lua 脚本转译为 C 代码，通过原生编译器编译为动态库（.so/.dll）加载执行。
* **灵活的后端**: 支持多种 C 编译器后端，包括轻量级的 TCC 以及高性能的 GCC 和 Clang。
* **Lua 5.4 兼容性**: 支持 Lua 5.4 核心语法和语义的精简子集。
* **现代 C++**: 基于 C++23 构建，充分利用现代语言特性。
* **易于嵌入**: 专为 C++ 应用程序集成而设计，提供简洁的交互接口。
* **轻量化**: 极简的执行模型，无繁重的垃圾回收（GC）负担。

# 构建 (Build)
### Linux
* 依赖: `cmake`, `flex`, `bison`。
* 安装TCC。
```shell
# git clone https://repo.or.cz/tinycc.git
# cd tinycc
# ./configure --extra-cflags="-fPIC"
# make
# sudo make install
```
* 编译: 
```shell
mkdir build
cd build
cmake ..
make
```

### Windows (MinGW)
* 安装 [MSYS2](https://www.msys2.org)。安装[mingw-w64-x86_64-gcc](https://packages.msys2.org/packages/mingw-w64-x86_64-gcc), [mingw-w64-x86_64-lua](https://packages.msys2.org/package/mingw-w64-x86_64-lua)。
* 使用 CLion 打开项目编译。
