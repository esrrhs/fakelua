# FakeLua
[<img src="https://img.shields.io/github/license/esrrhs/fakelua">](https://github.com/esrrhs/fakelua)
[<img src="https://img.shields.io/github/languages/top/esrrhs/fakelua">](https://github.com/esrrhs/fakelua)
[<img src="https://img.shields.io/github/actions/workflow/status/esrrhs/fakelua/build.yml?branch=master&label=Linux">](https://github.com/esrrhs/fakelua/actions/workflows/build.yml)
[<img src="https://img.shields.io/github/actions/workflow/status/esrrhs/fakelua/build_with_macos.yml?branch=master&label=macOS">](https://github.com/esrrhs/fakelua/actions/workflows/build_with_macos.yml)
[<img src="https://img.shields.io/github/actions/workflow/status/esrrhs/fakelua/build_with_windows.yml?branch=master&label=Windows">](https://github.com/esrrhs/fakelua/actions/workflows/build_with_windows.yml)
[![codecov](https://codecov.io/gh/esrrhs/fakelua/graph/badge.svg?token=9ZCUH1Q632)](https://codecov.io/gh/esrrhs/fakelua)

FakeLua 是一个可嵌入的 Lua 子集编译引擎：将 Lua 脚本编译为 C 代码，通过 GCC 后端动态编译为原生机器码执行。提供 C++23 接口，支持脚本与原生代码高效互操作。

**设计目标**：面向高性能游戏服务器等实时系统，彻底消除传统脚本语言 GC 带来的吞吐量抖动。采用 Arena 内存池 + 帧重置设计，分配和清理均为 O(1)，无 GC 停顿。

## 核心特性

- **数值参数特化**：自动识别参与算术运算的参数，生成 `2^k` 个类型特化版本（`int64_t`/`double`），彻底消除热路径上的类型判断开销。Fibonacci n=32 比 Lua 5.4 快 **43.7x**
- **双 JIT 后端**：GCC（`-O3`，生产主力）和内嵌 TCC（快速编译，调试用），同一套 API 无缝切换
- **可变参数（`...`）**：支持声明和调用 vararg 函数，C++ 侧多余参数自动打包，无需手动组装
- **多返回值**：支持 `return a, b` 多返回值和参数展开，C++ 侧通过 `std::tie(a, b, c)` 自动解包
- **全局变量复杂初始化**：支持任意复杂表达式作为全局变量初始化器，编译器自动提取到 `__fakelua_init()` 函数
- **C++23 嵌入 API**：`CompileFile` / `CompileString` / `Call`，RAII 风格 `FakeluaStateGuard`

## 快速上手

### 构建

```bash
# Linux / macOS
cmake -S . -B build
cmake --build build --parallel

# Windows (MSYS2 + MinGW)
cmake -S . -B build -G Ninja
cmake --build build --parallel
```

> 需要 C++23 编译器（GCC 11+ / Clang 16+ / MSVC 2022+）。macOS 需先 `brew install lua cmake`。

### C++ 嵌入示例

```cpp
#include "fakelua.h"
using namespace fakelua;

int main() {
    FakeluaStateGuard guard;
    State* s = guard.GetState();

    CompileFile(s, "script.lua", {.debug_mode = false});

    // 基本调用
    int ret = 0;
    Call(s, JIT_GCC, "add", ret, 1, 2);

    // 可变参数 — 多余参数自动打包为 Multi
    int sum = 0;
    Call(s, JIT_GCC, "sum", sum, 10, 20, 30);

    // 多返回值 — 用 std::tie 自动解包
    int a = 0, b = 0;
    Call(s, JIT_GCC, "multi_return", std::tie(a, b, c), some_arg);
}
```

`Call()` 支持最多 32 个参数，调用 vararg 函数时多余参数自动打包，多返回值通过 `std::tie` 自动解包。

### 命令行工具

```bash
./build/bin/flua <script.lua> --entry=<func> --jit_type=<0|1> --repeat=<N>
```

## 性能基准

Release `-O3` 模式，对比 Lua 5.4：

| 算法 | Lua 5.4 | FakeLua TCC | FakeLua GCC |
|---|---|---|---|
| Fibonacci n=32 | 229.6 ms | 22.1 ms（**10.4x**） | 5.3 ms（**43.7x**） |
| Sum n=5000000 | 32.4 ms | 11.7 ms（**2.8x**） | 2.7 ms（**11.8x**） |
| Popcount n=100000 | 13.9 ms | 1.95 ms（**7.1x**） | 0.49 ms（**28.4x**） |

> TCC 纯计算场景快于 Lua；含 table 操作时 TCC 偏慢。完整数据见 [benchmark/README.md](benchmark/README.md)。

## 已知限制

- 不支持 `label`/`goto`、metatable、协程
- 全局变量初始化不支持 table constructor
- 函数参数上限 32 个，数学特化参数上限 8 个
- 泛型 `for in` 仅支持 `pairs()`/`ipairs()`
- 每个 `State` 为线程本地对象，多线程需独立 State

## 常见问题

**Q: 为什么选择 Lua 子集？**
A: 完整 Lua 的 metatable 等动态特性难以高效编译。子集聚焦可静态分析的模式，通过类型推导和 JIT 获得接近 C 的性能。已支持多返回值和 varargs。

**Q: TCC 和 GCC 如何选择？**
A: **GCC** 是生产主力（`-O3` 优化）；**TCC** 编译极快，用于开发调试。

**Q: 如何调试生成的 C 代码？**
A: 启用 `CompileConfig::debug_mode`，或用 `GetLastRecordedCCode()` 导出 C 代码。

## 架构概览

```
Lua 源码 → 词法分析(flexer) → 语法分析(bison) → 预处理 → 语义分析
→ 类型推导 → C代码生成(c_gen) → JIT编译(GCC/TCC) → 加载执行
```
