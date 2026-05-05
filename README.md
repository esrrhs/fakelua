# FakeLua
[<img src="https://img.shields.io/github/license/esrrhs/fakelua">](https://github.com/esrrhs/fakelua)
[<img src="https://img.shields.io/github/languages/top/esrrhs/fakelua">](https://github.com/esrrhs/fakelua)
[<img src="https://img.shields.io/github/actions/workflow/status/esrrhs/fakelua/build.yml?branch=master&label=Linux">](https://github.com/esrrhs/fakelua/actions/workflows/build.yml)
[<img src="https://img.shields.io/github/actions/workflow/status/esrrhs/fakelua/build_with_macos.yml?branch=master&label=macOS">](https://github.com/esrrhs/fakelua/actions/workflows/build_with_macos.yml)
[<img src="https://img.shields.io/github/actions/workflow/status/esrrhs/fakelua/build_with_windows.yml?branch=master&label=Windows">](https://github.com/esrrhs/fakelua/actions/workflows/build_with_windows.yml)
[![codecov](https://codecov.io/gh/esrrhs/fakelua/graph/badge.svg?token=9ZCUH1Q632)](https://codecov.io/gh/esrrhs/fakelua)

FakeLua 是一个可嵌入的 Lua 子集执行引擎，将 Lua 脚本编译为 C 代码，再由 TinyCC 或 GCC 生成动态库加载执行。

## 技术亮点

### 双 JIT 后端

支持两种 JIT 模式，同一套 API 无缝切换：

- **JIT_TCC**：内嵌 TinyCC，编译速度极快，适合频繁热更新场景；TCC 源码在 CMake 配置阶段自动拉取，无需系统预装。
- **JIT_GCC**：调用系统 GCC（`-O3`），生成高质量原生代码，适合性能敏感场景。

### 数值参数特化（Numeric Specialization）

编译器对函数的数学参数自动做类型推断与特化：

1. **TypeInferencer** 对每个顶层函数运行迭代不动点推断（leave-one-out），识别出真正参与算术运算的参数（math params）。
2. **CGen** 为每个含数学参数的函数生成 `2^k` 个特化版本（`int64_t` / `double` 组合），以及一个运行时入口分发器，根据实际参数类型路由到对应特化体。
3. 特化体内的算术运算直接用原生 C 类型（`int64_t`/`double`）计算，比较表达式也生成原生 C `bool` 而非走 `CVar` 装箱路径，彻底消除热路径上的类型判断开销。

以递归 Fibonacci（n=32）为例，GCC 后端比 Lua 5.4 快 **9x**，TCC 后端快 **1.9x**（详见 [benchmark/README.md](benchmark/README.md)）。

### CVar：ABI 安全的跨边界值类型

```cpp
struct CVar {
    int type_;
    int flag_;
    union { bool b; int64_t i; double f; VarString *s; VarTable *t; } data_;
};
static_assert(std::is_standard_layout_v<CVar>);
static_assert(std::is_trivially_copyable_v<CVar>);
```

`CVar` 是 JIT 代码与 C++ 宿主之间传递值的唯一载体，强制为标准布局（POD），保证 arm64 等平台的 ABI 兼容性。

### VarInterface：可扩展的复杂类型桥接

`VarInterface` 是 Lua table 等复杂类型与宿主之间的抽象接口，宿主可按需实现自己的版本接入原有对象系统。库内附带 `SimpleVarImpl` 开箱即用。

## 快速上手

### 构建

```bash
cmake -S . -B build
cmake --build build --parallel
```

> macOS 需先 `brew install lua cmake`，并在 cmake 时加 `-DCMAKE_PREFIX_PATH="$(brew --prefix)"`。

### C++ 嵌入示例

```cpp
#include "fakelua.h"
using namespace fakelua;

int main() {
    FakeluaStateGuard guard;
    State* s = guard.GetState();

    CompileFile(s, "script.lua", CompileConfig{.debug_mode = false});

    int ret = 0;
    Call(s, JIT_GCC, "add", ret, 1, 2);
}
```

`Call()` 支持最多 8 个参数，参数与返回值在原生 C++ 类型与 `CVar` 之间自动转换。

### 命令行工具 `flua`

```bash
./build/bin/flua <script.lua> --entry=<func> --jit_type=<0|1> --repeat=<N>
```

- `--jit_type`：`0`=TCC，`1`=GCC
- `--repeat`：重复调用次数（用于性能测量）

## 性能基准

对比 C++、Lua 5.4、FakeLua TCC、FakeLua GCC，覆盖 Fibonacci、GCD、快速幂、线性求和、冒泡排序、筛质数等 11 类算法（Release `-O3` 模式）：

| 算法（典型参数） | Lua 5.4 | FakeLua TCC | FakeLua GCC |
|---|---|---|---|
| Fibonacci n=32 | 133 ms | 70 ms（**1.9x**↑） | 14.8 ms（**9.0x**↑） |
| Sum n=5000000 | 21.9 ms | 9.7 ms（**2.3x**↑） | 1.1 ms（**19x**↑） |
| Popcount n=100000 | 9.5 ms | 1.9 ms（**5.1x**↑） | 0.33 ms（**29x**↑） |
| BubbleSort n=200 | 522 μs | 3.3 ms（0.16x） | 511 μs（**1.0x**） |
| Sieve n=5000 | 176 μs | 989 μs（0.18x） | 142 μs（**1.2x**↑） |

> TCC 纯计算类场景（无 table）普遍快于 Lua；含大量 table 操作时 GCC 与 Lua 持平，TCC 偏慢（table 操作尚未特化）。完整数据见 [benchmark/README.md](benchmark/README.md)。

## 已知限制

- 不支持多返回值、varargs（`...`）、`label`/`goto`
- 泛型 `for in` 仅支持 `pairs()` / `ipairs()`
- 函数参数上限 8 个
