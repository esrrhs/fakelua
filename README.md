# FakeLua
[<img src="https://img.shields.io/github/license/esrrhs/fakelua">](https://github.com/esrrhs/fakelua)
[<img src="https://img.shields.io/github/languages/top/esrrhs/fakelua">](https://github.com/esrrhs/fakelua)
[<img src="https://img.shields.io/github/actions/workflow/status/esrrhs/fakelua/build.yml?branch=master&label=Linux">](https://github.com/esrrhs/fakelua/actions/workflows/build.yml)
[<img src="https://img.shields.io/github/actions/workflow/status/esrrhs/fakelua/build_with_macos.yml?branch=master&label=macOS">](https://github.com/esrrhs/fakelua/actions/workflows/build_with_macos.yml)
[<img src="https://img.shields.io/github/actions/workflow/status/esrrhs/fakelua/build_with_windows.yml?branch=master&label=Windows">](https://github.com/esrrhs/fakelua/actions/workflows/build_with_windows.yml)
[![codecov](https://codecov.io/gh/esrrhs/fakelua/graph/badge.svg?token=9ZCUH1Q632)](https://codecov.io/gh/esrrhs/fakelua)

FakeLua 是一个可嵌入的 Lua 子集编译引擎：将 Lua 脚本编译为 C 代码，通过 GCC 后端动态编译为原生机器码执行。提供 C++23 接口，支持脚本与原生代码高效互操作。

## 设计初衷与内存设计哲学

FakeLua 的设计初衷是为了在**高性能游戏服务器**或类似的实时系统中，解决传统脚本语言（如标准 Lua/LuaJIT）由于**垃圾回收（GC）机制带来的吞吐量抖动和内存膨胀问题**。

### 1. 脚本定位：高内聚的“业务粘合剂”
在典型的实时高性能服务器架构中：
*   **状态与数据驻留 C++**：核心的数据结构（如玩家状态、世界地图、怪物属性、物理引擎）全部保存在高效、紧凑、类型安全的 C++ 宿主侧。
*   **无状态/浅状态的 Lua 逻辑层**：Lua 只用作纯逻辑处理和业务粘合，负责读取 C++ 数据并调用 C++ 函数进行逻辑运算。脚本内不应该长期留存大规模的数据对象。

### 2. 内存设计：极速的 Arena 内存池 + 帧重置
为了配合上述定位，FakeLua **没有引入复杂的动态垃圾回收器**（如三色标记、分代 GC），而是采用了极其高效的 **Arena 内存池（Bump Allocator）**：
*   **指针碰撞分配 ($O(1)$)**：脚本中创建临时变量（Table, String, Multi 等）时，只需在预分配的连续内存块中移动偏移指针，分配效率接近原生堆栈，没有 `malloc` 的碎片和多余开销。
*   **单点瞬间清理 ($O(1)$)**：在每帧逻辑执行结束，或者单次请求处理完毕后，直接调用 `State::Reset()`。它会逆序调用已创建对象的析构函数，但无需单独释放每块内存，而是将内存池的偏移指针直接重置为 0。没有复杂的对象图遍历，没有系统级的 `free` 耗时与内存碎片整理，清理操作瞬时完成。

这种设计使得 FakeLua 在保持 JIT 原生执行速度的同时，能够彻底消除垃圾回收停顿（GC Pause）对帧率的影响，让内存开销保持在一条完全可预测的、极低的水平线上。

## 核心特性

### 双 JIT 后端

支持两种 JIT 模式，同一套 API 无缝切换：

- **JIT_GCC**：调用系统 GCC（`-O3`），生成高质量原生代码。这是 FakeLua 实际运行和生产环境采用的主力后端。
- **JIT_TCC**：内嵌 TinyCC，编译速度极快。主要用于开发调试和测试验证（TCC 源码在 CMake 配置阶段自动拉取，无需系统预装）。

### 数值参数特化（Numeric Specialization）

编译器对函数的数学参数自动做类型推断与特化：

1. [TypeInferencer](file:///home/project/fakelua/src/compile/type_inferencer.h) 对每个顶层函数运行迭代不动点推断（leave-one-out），识别出真正参与算术运算的参数（math params）。
2. [CGen](file:///home/project/fakelua/src/compile/c_gen.h) 为每个含数学参数的函数生成 `2^k` 个特化版本（`int64_t` / `double` 组合），以及一个运行时入口分发器，根据实际参数类型路由到对应特化体。
3. 特化体内的算术运算直接用原生 C 类型（`int64_t`/`double`）计算，比较表达式也生成原生 C `bool` 而非走 [CVar](file:///home/project/fakelua/include/fakelua.h#L198) 装箱路径，彻底消除热路径上的类型判断开销。

以递归 Fibonacci（n=32）为例，GCC 后端比 Lua 5.4 快 **43.7x**，TCC 后端快 **10.4x**（详见 [benchmark/README.md](benchmark/README.md)）。

### [CVar](file:///home/project/fakelua/include/fakelua.h#L198)：ABI 安全的跨边界值类型

```cpp
struct CVar {
    int type_ = 0;
    int flag_ = 0;
    union cvar_data {
        bool b;
        int64_t i;
        double f;
        VarString *s;
        VarTable *t;
        VarMulti *m;
    };
    cvar_data data_{};
};
static_assert(std::is_standard_layout_v<CVar>);
static_assert(std::is_trivially_copyable_v<CVar>);
```

[CVar](file:///home/project/fakelua/include/fakelua.h#L198) 是 JIT 代码与 C++ 宿主之间传递值的唯一载体，强制为标准布局（POD），保证 arm64 等平台的 ABI 兼容性。

### [VarInterface](file:///home/project/fakelua/include/fakelua.h#L17)：可扩展的复杂类型桥接

[VarInterface](file:///home/project/fakelua/include/fakelua.h#L17) 是 Lua table 等复杂类型与宿主之间的抽象接口，宿主可按需实现自己的版本接入原有对象系统。库内附带 [SimpleVarImpl](file:///home/project/fakelua/include/fakelua.h#L61) 开箱即用。

### 多返回值与可变参数（Multi-Return & Varargs）

- **多返回值**：函数可以通过 `return a, b` 返回多个值，在赋值或返回语句中正确解包。
- **参数动态展开**：在函数调用或 Table 构造中，若最后一项是多返回值函数调用，其返回值会自动展开。
- **可变参数（`...`）**：支持声明和调用 vararg 函数，C++ 侧调用时多余参数自动打包为 Multi，无需手动组装。
- **C++ 返回值自动解包**：通过 `std::tie(a, b, c)` 接收多返回值，模板自动将 Multi CVar 拆解为各变量。

### C++ 嵌入 API

- `CompileFile` / `CompileString` / `Call`，RAII 风格 `FakeluaStateGuard`
- 支持基本类型、对象、以及自定义 VarInterface 实现的高级映射
- 支持记录编译生成的 C 代码用于调试和性能分析（`CompileConfig::record_c_code`）

### 全局变量复杂初始化

支持任意复杂表达式作为全局/文件级变量的初始化器：

```lua
local x = math.floor(3.14) + 1
local y = x * 2 - 1
local z = (x + y) / 2.0
```

编译器会将复杂初始化器提取到生成的 `__fakelua_init()` 函数中，在 JIT 加载后立即执行，使全局变量获得正确的运行时值。

## 当前已知限制

### 语法限制
- 不支持 `label` / `goto`
- 泛型 `for in` 仅支持 `pairs()` / `ipairs()`
- 脚本侧函数调用仅支持简单函数名调用（不支持复杂前缀表达式调用，如 `obj:method()` 需通过间接方式实现）

### 初始化约束
- 初始化器中不支持嵌套表构造（如 `local t = {{1,2}, {3,4}}`）

### 类型系统限制
- 类型推导基于静态分析，复杂的动态类型操作无法优化
- 函数 specialization 基于调用点的 math 参数发现
- 函数参数上限 32 个（通过常量 `kMaxFunctionInputParams` 统一配置）
- 数学特化参数上限 8 个（通过常量 [kMaxMathSpecializedParams](file:///home/project/fakelua/include/fakelua.h#L14) 统一配置，超过此限制的数学参数不进行特化，作为普通动态参数处理）

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
- `--debug`：是否启用调试模式（默认 `false`，若为 `true` 则输出生成的 C 源码）

### C++ 嵌入示例

```cpp
#include "fakelua.h"
using namespace fakelua;

int main() {
    FakeluaStateGuard guard;
    State* s = guard.GetState();

    CompileFile(s, "script.lua", CompileConfig{.debug_mode = false});

    // 基本调用
    int ret = 0;
    Call(s, JIT_GCC, "add", ret, 1, 2);

    // 可变参数调用 — 多余参数自动打包为 Multi
    int sum = 0;
    Call(s, JIT_TCC, "sum", sum, 10, 20, 30);

    // 多返回值 — 用 std::tie 自动解包
    int a = 0, b = 0;
    std::string c;
    Call(s, JIT_GCC, "multi_return", std::tie(a, b, c), some_arg);
}
```

[Call()](file:///home/project/fakelua/include/fakelua.h#L318) 支持最多 32 个参数（受到 [kMaxFunctionInputParams](file:///home/project/fakelua/include/fakelua.h#L13) 的统一限制），参数与返回值在原生 C++ 类型与 [CVar](file:///home/project/fakelua/include/fakelua.h#L198) 之间自动转换。调用 vararg 函数时，超出固定参数的多余参数会自动打包；多返回值函数可通过 `std::tie` 接收。

## 性能基准

对比 Lua 5.4、FakeLua TCC、FakeLua GCC，覆盖 Fibonacci、GCD、快速幂、线性求和、冒泡排序、筛质数等 11 类算法（Release `-O3` 模式）：

| 算法（典型参数） | Lua 5.4 | FakeLua TCC | FakeLua GCC |
|---|---|---|---|
| Fibonacci n=32 | 229.6 ms | 22.1 ms（**10.4x**↑） | 5.3 ms（**43.7x**↑） |
| Sum n=5000000 | 32.4 ms | 11.7 ms（**2.8x**↑） | 2.7 ms（**11.8x**↑） |
| Popcount n=100000 | 13.9 ms | 1.95 ms（**7.1x**↑） | 0.49 ms（**28.4x**↑） |
| BubbleSort n=200 | 873.7 μs | 2.46 ms（0.35x） | 461.2 μs（**1.9x**↑） |
| Sieve n=5000 | 282.6 μs | 771.4 μs（0.37x） | 192.9 μs（**1.5x**↑） |

> TCC 纯计算类场景（无 table）普遍快于 Lua；含大量 table 操作时 GCC 与 Lua 持平，TCC 偏慢（table 操作尚未特化）。完整数据见 [benchmark/README.md](benchmark/README.md)。

## C++ API 详细文档

### 状态管理

```cpp
// 手动管理（不推荐，容易泄漏）
State* s = [FakeluaNewState](file:///home/project/fakelua/include/fakelua.h#L262)([StateConfig](file:///home/project/fakelua/include/fakelua.h#L252){});
// ... 使用 s ...
[FakeluaDeleteState](file:///home/project/fakelua/include/fakelua.h#L265)(s);

// 或使用 RAII 风格（推荐）
[FakeluaStateGuard](file:///home/project/fakelua/include/fakelua.h#L268) guard([StateConfig](file:///home/project/fakelua/include/fakelua.h#L252){});
State* s = guard.GetState();
// ... 使用 s ...
// 自动释放
```

### API 概览

| 函数 | 功能 |
|------|------|
| [`FakeluaNewState()`](file:///home/project/fakelua/include/fakelua.h#L262) | 创建 FakeLua 状态 |
| [`FakeluaDeleteState()`](file:///home/project/fakelua/include/fakelua.h#L265) | 释放 FakeLua 状态 |
| [`CompileFile()`](file:///home/project/fakelua/include/fakelua.h#L308) | 编译 Lua 文件 |
| [`CompileString()`](file:///home/project/fakelua/include/fakelua.h#L311) | 编译 Lua 代码字符串 |
| [`Call()`](file:///home/project/fakelua/include/fakelua.h#L318) | 调用编译后的函数 |
| [`GetLastRecordedCCode()`](file:///home/project/fakelua/include/fakelua.h#L315) | 获取最近编译的 C 代码 |
| [`SetVarInterfaceNewFunc()`](file:///home/project/fakelua/include/fakelua.h#L322) | 设置自定义 VarInterface 工厂 |
| [`SetDebugLogLevel()`](file:///home/project/fakelua/include/fakelua.h#L329) | 设置全局调试日志级别 |

### 类型转换

FakeLua 提供 [`inter::NativeToFakelua()`](file:///home/project/fakelua/include/fakelua.h#L355) 和 [`FakeluaToNative()`](file:///home/project/fakelua/include/fakelua.h#L458) 自动推导型转换：

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

通过实现 [`VarInterface`](file:///home/project/fakelua/include/fakelua.h#L17) 可实现 Lua table 与原生对象的双向映射：

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
[语义分析] → analysis result (semantic_analysis)
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
| [`lexer/parser`](file:///home/project/fakelua/src/compile/bison/) | Lua 词法和语法解析 |
| [`syntax_tree`](file:///home/project/fakelua/src/compile/syntax_tree.h) | AST 表示和遍历 |
| [`preprocessor`](file:///home/project/fakelua/src/compile/preprocessor.h) | Lua 语法规范化（如 functiondef 提升） |
| [`semantic_analysis`](file:///home/project/fakelua/src/compile/semantic_analysis.h) | 语义和控制流分析（如未定义符号分析等） |
| [`type_inferencer`](file:///home/project/fakelua/src/compile/type_inferencer.h) | 静态类型推导和 specialization 决策 |
| [`c_gen`](file:///home/project/fakelua/src/compile/c_gen.h) | C 代码生成和类型驱动优化 |
| [`compile_common`](file:///home/project/fakelua/src/compile/compile_common.h) | 公共类型推导和代码生成工具 |
| [`jit/*`](file:///home/project/fakelua/src/jit/) | TCC 和 GCC 后端集成 |
| [`state`](file:///home/project/fakelua/src/state/) | FakeLua 运行时状态管理 |
| [`var`](file:///home/project/fakelua/src/var/) | 动态值 CVar 和转换工具 |

## 常见问题

### Q: 为什么选择 Lua 子集而不是完整 Lua？
A: 完整 Lua 的某些动态特性（如 metatable）很难高效编译。子集实现聚焦于可静态分析的常见模式，通过类型推导和 JIT 编译获得接近 C 的性能。目前已支持多返回值、参数展开与可变参数（varargs），但更复杂的元表（metatable）或协程等特性尚不支持。

### Q: TCC 和 GCC 后端如何选择？
A: **GCC** 是实际运行和生产环境采用的主力后端（开启 `-O3` 优化生成高质量原生代码）；**TCC** 编译极快，但优化有限，主要作为开发调试和测试运行使用。

### Q: 可以在嵌入式或受限环境中使用吗？
A: 可以，TCC 后端体积小，编译速度快，适合嵌入式。核心库依赖极少（仅 C++ 标准库），可交叉编译。

### Q: 如何调试生成的 C 代码？
A: 启用 [`CompileConfig::debug_mode`](file:///home/project/fakelua/include/fakelua.h#L232)，查看日志和 C 代码；使用 [`GetLastRecordedCCode()`](file:///home/project/fakelua/include/fakelua.h#L315) 导出 C 代码进行分析。

### Q: 支持多线程吗？
A: 每个 [`State`](file:///home/project/fakelua/include/fakelua.h#L259) 当前为线程本地对象，多线程环境中应为每个线程创建独立的 [`State`](file:///home/project/fakelua/include/fakelua.h#L259)。
