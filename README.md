# FakeLua
[<img src="https://img.shields.io/github/license/esrrhs/fakelua">](https://github.com/esrrhs/fakelua)
[<img src="https://img.shields.io/github/languages/top/esrrhs/fakelua">](https://github.com/esrrhs/fakelua)
[<img src="https://img.shields.io/github/actions/workflow/status/esrrhs/fakelua/build.yml?branch=master&label=Linux">](https://github.com/esrrhs/fakelua/actions/workflows/build.yml)
[<img src="https://img.shields.io/github/actions/workflow/status/esrrhs/fakelua/build_with_macos.yml?branch=master&label=macOS">](https://github.com/esrrhs/fakelua/actions/workflows/build_with_macos.yml)
[<img src="https://img.shields.io/github/actions/workflow/status/esrrhs/fakelua/build_with_windows.yml?branch=master&label=Windows">](https://github.com/esrrhs/fakelua/actions/workflows/build_with_windows.yml)
[![codecov](https://codecov.io/gh/esrrhs/fakelua/graph/badge.svg?token=9ZCUH1Q632)](https://codecov.io/gh/esrrhs/fakelua)

FakeLua 是一个可嵌入的 Lua 子集编译引擎：将 Lua 脚本编译为 C 代码，通过 TinyCC 或 GCC 后端动态编译，生成机器码加载执行。提供 C++23 接口，支持脚本与原生代码高效互操作。

## 核心特性

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

### 自动类型推导

- 静态分析 Lua 脚本，自动推导函数参数和返回值类型
- 生成高效的类型化 C 代码，消除动态类型装箱开销
- 支持根据调用上下文自动生成 int64/double 专特化版本

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

### C++ 嵌入 API

- `CompileFile` / `CompileString` / `Call`，RAII 风格 `FakeluaStateGuard`
- 支持基本类型、对象、以及自定义 VarInterface 实现的高级映射
- 支持记录编译生成的 C 代码用于调试和性能分析（`CompileConfig::record_c_code`）

## 当前已知限制

### 语法限制
- 不支持多返回值（Call 仅支持单返回值，多返回可通过 Table 实现）
- 不支持 varargs（`...`）
- 不支持 `label` / `goto`
- 泛型 `for in` 仅支持 `pairs()` / `ipairs()`
- 脚本侧函数调用仅支持简单函数名调用（不支持复杂前缀表达式调用，如 `obj:method()` 需通过间接方式实现）

### 初始化约束
- 全局变量初始化不支持 table constructor
- 全局变量初始化不支持部分复杂表达式
- 文件级 local 变量仅支持数值类型（T_INT/T_FLOAT）和 nil/bool/string 的特定初始化

### 类型系统限制
- 类型推导基于静态分析，复杂的动态类型操作无法优化
- 函数 specialization 基于调用点的 math 参数发现
- 函数参数上限 8 个

## 快速上手

### 构建

#### 系统要求
- **C++23** 编译器（GCC 11+ / Clang 16+ / MSVC 2022+）
- CMake 3.5+
- make 或 ninja

#### Linux / macOS

```bash
cmake -S . -B build
cmake --build build --parallel
```

> macOS 需先 `brew install lua cmake`，并在 cmake 时加 `-DCMAKE_PREFIX_PATH="$(brew --prefix)"`。

仅构建核心库与命令行工具（不含测试/基准）：

```bash
cmake --build build --target fakelua flua --parallel
```

#### Windows（MSYS2 + MinGW）

```bash
cmake -S . -B build -G Ninja
cmake --build build --parallel
ctest --test-dir build -V
```

### 测试与基准

```bash
cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build --parallel
ctest --test-dir build -V
./build/bin/bench_mark
```

> 单元测试与 benchmark 依赖 Lua 开发包（头文件 `lua.h` 和库文件）。
> - 在 Linux 上：`sudo apt-get install liblua5.4-dev` 或 `liblua5.3-dev`
> - 在 macOS 上：`brew install lua`
> - 在 Windows MSYS2 上：`pacman -S mingw-w64-x86_64-lua`

### 命令行工具 `flua`

```bash
./build/bin/flua <script.lua> --entry=<func> --jit_type=<0|1> --repeat=<N>
```

- `--entry`：入口函数名（默认 `main`）
- `--jit_type`：`0`=TCC，`1`=GCC
- `--repeat`：重复调用次数（用于性能测量）

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

## C++ API 详细文档

### 状态管理

```cpp
// 手动管理（不推荐，容易泄漏）
State* s = FakeluaNewState(StateConfig{});
// ... 使用 s ...
FakeluaDeleteState(s);

// 或使用 RAII 风格（推荐）
FakeluaStateGuard guard(StateConfig{});
State* s = guard.GetState();
// ... 使用 s ...
// 自动释放
```

### API 概览

| 函数 | 功能 |
|------|------|
| `FakeluaNewState()` | 创建 FakeLua 状态 |
| `FakeluaDeleteState()` | 释放 FakeLua 状态 |
| `CompileFile()` | 编译 Lua 文件 |
| `CompileString()` | 编译 Lua 代码字符串 |
| `Call()` | 调用编译后的函数 |
| `GetLastRecordedCCode()` | 获取最近编译的 C 代码 |
| `SetVarInterfaceNewFunc()` | 设置自定义 VarInterface 工厂 |
| `SetDebugLogLevel()` | 设置全局调试日志级别 |

### 类型转换

FakeLua 提供 `inter::NativeToFakelua()` 和 `FakeluaToNative()` 自动推导型转换：

```cpp
// 原生 → FakeLua
CVar v_int = inter::NativeToFakelua(s, 42);
CVar v_str = inter::NativeToFakelua(s, std::string("hello"));
CVar v_bool = inter::NativeToFakelua(s, true);

// FakeLua → 原生
int native_int = inter::FakeluaToNative<int>(v_int);
std::string native_str = inter::FakeluaToNative<std::string>(v_str);
```

### Table 与对象互转

通过实现 `VarInterface` 可实现 Lua table 与原生对象的双向映射：

```cpp
class CustomVar : public VarInterface {
    // 实现所有虚函数...
};

// 注册工厂函数
SetVarInterfaceNewFunc(s, []() { return new CustomVar(); });

// 之后在 Call 中传递的 table 类型参数会自动构造为 CustomVar 实例
```

## 架构概览

### 编译流程

```
Lua 源码
   ↓
[词法分析] → tokens (flexer)
   ↓
[语法分析] → AST (bison + syntax_tree)
   ↓
[预处理] → normalized AST (preprocessor)
   ↓
[类型推导] → type hints (type_inferencer)
   ↓
[C 代码生成] → C 源码 (c_gen)
   ↓
[JIT 编译] → 机器码 (tcc_jit / gcc_jit)
   ↓
[加载执行] → 结果
```

### 关键组件

| 模块 | 职责 |
|------|------|
| `lexer/parser` | Lua 词法和语法解析 |
| `syntax_tree` | AST 表示和遍历 |
| `preprocessor` | Lua 语法规范化（如 functiondef 提升） |
| `type_inferencer` | 静态类型推导和 specialization 决策 |
| `c_gen` | C 代码生成和类型驱动优化 |
| `compile_common` | 公共类型推导和代码生成工具 |
| `jit/*` | TCC 和 GCC 后端集成 |
| `state` | FakeLua 运行时状态管理 |
| `var` | 动态值 CVar 和转换工具 |

## 项目结构

```
fakelua/
├── include/
│   └── fakelua.h           # 公共 C++ API
├── src/
│   ├── compile/            # Lua 编译器实现
│   │   ├── c_gen.h/cpp     # C 代码生成
│   │   ├── type_inferencer.h/cpp  # 类型推导
│   │   ├── preprocessor.h/cpp     # 预处理
│   │   ├── syntax_tree.h/cpp      # AST
│   │   └── ...
│   ├── jit/                # JIT 后端
│   │   ├── tcc_jit.h/cpp   # TinyCC 后端
│   │   ├── gcc_jit.h/cpp   # GCC 后端
│   │   ├── vm.h/cpp        # 虚拟机
│   │   └── ...
│   ├── state/              # 运行时状态
│   ├── var/                # 动态值和转换
│   └── util/               # 工具函数
├── test/                   # 单元测试
├── benchmark/              # 性能基准
├── cmd/                    # 命令行工具 flua
└── cmake/                  # CMake 配置
```

## 常见问题

### Q: 为什么选择 Lua 子集而不是完整 Lua？
A: 完整 Lua 的动态特性（如 metatable、varargs、多返回值）很难高效编译。子集实现聚焦于可静态分析的常见模式，通过类型推导和 JIT 编译获得接近 C 的性能。

### Q: TCC 和 GCC 后端如何选择？
A: **TCC** 快速编译（适合脚本小、编译频繁）；**GCC** 优化充分（适合脚本大、运行次数多）。在同一 API 下可根据场景动态选择。

### Q: 可以在嵌入式或受限环境中使用吗？
A: 可以，TCC 后端体积小，编译速度快，适合嵌入式。核心库依赖极少（仅 C++ 标准库），可交叉编译。

### Q: 如何调试生成的 C 代码？
A: 启用 `CompileConfig::debug_mode`，查看日志和 C 代码；使用 `GetLastRecordedCCode()` 导出 C 代码进行分析。

### Q: 支持多线程吗？
A: 每个 `State` 当前为线程本地对象，多线程环境中应为每个线程创建独立的 `State`。
