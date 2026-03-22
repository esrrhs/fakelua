# FakeLua (开发中)
[<img src="https://img.shields.io/github/license/esrrhs/fakelua">](https://github.com/esrrhs/fakelua)
[<img src="https://img.shields.io/github/languages/top/esrrhs/fakelua">](https://github.com/esrrhs/fakelua)
[<img src="https://img.shields.io/github/actions/workflow/status/esrrhs/fakelua/build.yml?branch=master">](https://github.com/esrrhs/fakelua/actions)
[![codecov](https://codecov.io/gh/esrrhs/fakelua/graph/badge.svg?token=9ZCUH1Q632)](https://codecov.io/gh/esrrhs/fakelua)

FakeLua 是一个高性能、可嵌入的脚本执行引擎，实现了 Lua 5.4 语法的子集。其核心目标是通过使用 `libgccjit` 将脚本即时编译（JIT）为原生机器码，从而提供极致的运行速度。

# 特性 (Features):
* **极致性能**: 使用 GCC JIT 后端将脚本直接编译为原生机器码执行。
* **Lua 兼容性**: 支持 Lua 5.4 核心语法和语义。
* **现代 C++**: 基于 C++23 构建，兼顾安全与性能。
* **易于嵌入**: 专为 C++ 应用程序集成而设计。
* **调试支持**: 支持使用 GDB 直接调试脚本源码。
* **轻量化**: 极简的执行模型，无繁重的垃圾回收（GC）负担。

# 构建 (Build)
### Linux
* 依赖: 安装支持 JIT 的 gcc-13, cmake, flex, bison。安装 Lua 用于测试。
* 编译: 
```shell
mkdir build
cd build
cmake ..
make
```

### MinGW
* 依赖: 安装 [MSYS2](https://www.msys2.org)。安装 [mingw-w64-x86_64-libgccjit](https://packages.msys2.org/package/mingw-w64-x86_64-libgccjit), [mingw-w64-x86_64-lua](https://packages.msys2.org/package/mingw-w64-x86_64-lua)。安装 CLion。
* 使用 CLion 打开项目，设置 MinGW 工具链并编译。
