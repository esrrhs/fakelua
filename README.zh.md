# FakeLua
[<img src="https://img.shields.io/github/license/esrrhs/fakelua">](https://github.com/esrrhs/fakelua)
[<img src="https://img.shields.io/github/languages/top/esrrhs/fakelua">](https://github.com/esrrhs/fakelua)
[<img src="https://img.shields.io/github/actions/workflow/status/esrrhs/fakelua/build.yml?branch=master&label=Linux">](https://github.com/esrrhs/fakelua/actions/workflows/build.yml)
[<img src="https://img.shields.io/github/actions/workflow/status/esrrhs/fakelua/build_with_macos.yml?branch=master&label=macOS">](https://github.com/esrrhs/fakelua/actions/workflows/build_with_macos.yml)
[<img src="https://img.shields.io/github/actions/workflow/status/esrrhs/fakelua/build_with_windows.yml?branch=master&label=Windows">](https://github.com/esrrhs/fakelua/actions/workflows/build_with_windows.yml)
[![codecov](https://codecov.io/gh/esrrhs/fakelua/graph/badge.svg?token=9ZCUH1Q632)](https://codecov.io/gh/esrrhs/fakelua)

中文 | [English](README.md)

FakeLua 是一个可嵌入的 Lua 子集编译引擎：将 Lua 脚本编译为 C 代码，通过 GCC 后端动态编译为原生机器码执行。提供 C++23 接口，支持脚本与原生代码高效互操作。

## 设计初衷与内存设计哲学

FakeLua 的设计初衷是为了在**高性能游戏服务器**或类似的实时系统中，解决传统脚本语言（如标准 Lua/LuaJIT）由于**垃圾回收（GC）机制带来的吞吐量抖动和内存膨胀问题**。

### 1. 脚本定位：高内聚的"业务粘合剂"
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

```cpp
int ret = 0;
// 同一套 Call API，可按需指定 JIT_GCC 或 JIT_TCC 后端
Call(s, JIT_GCC, "add", ret, 10, 20); // 生产环境推荐：GCC 后端 (-O3 高性能)
Call(s, JIT_TCC, "add", ret, 10, 20); // 开发调试推荐：TCC 后端 (极速编译)
```

### 数值参数特化（Numeric Specialization）

编译器对函数的数学参数自动做类型推断与特化：

1. [TypeInferencer](src/compile/type_inferencer.h) 对每个顶层函数运行迭代不动点推断（leave-one-out），识别出真正参与算术运算的参数（math params）。
2. [CGen](src/compile/c_gen.h) 为每个含数学参数的函数生成 $2^k$ 个特化版本（`int64_t` / `double` 组合），以及一个运行时入口分发器，根据实际参数类型路由到对应特化体。
3. 特化体内的算术运算直接用原生 C 类型（`int64_t`/`double`）计算，比较表达式也生成原生 C `bool` 而非走 CVar 装箱路径，彻底消除热路径上的类型判断开销。

```lua
-- 示例 Lua 函数：递归计算 Fibonacci
function fib(n)
    if n <= 1 then return n end
    return fib(n - 1) + fib(n - 2)
end
```

编译器自动生成的特化 C 代码：

```c
// 1. 数值参数特化体：形参/返回值直接提升为原生 int64_t，无需 CVar 装箱与运行时类型判断
static int64_t fib_spec_0(int64_t n) {
    if (n <= 1) {
        return n;
    }
    return fib_spec_0(n - 1) + fib_spec_0(n - 2);
}

// 2. 通用入口分发器：快速判断传入类型，零开销路由到原生 C 特化函数
static CVar fib_dispatcher(CVar n_var) {
    if (LIKELY(n_var.type_ == VAR_INT)) {
        return (CVar){.type_ = VAR_INT, .data_.i = fib_spec_0(n_var.data_.i)};
    }
    // ... 动态路由至 double 特化体或通用 CVar 分支
}
```

以递归 Fibonacci（n=32）为例，GCC 后端比 Lua 5.4 快 **36.6x**，TCC 后端快 **11.2x**（详见 [benchmark/README.zh.md](benchmark/README.zh.md) / [English](benchmark/README.md)）。

### Table 结构体特化（Table Specialization）

如果 Table 构造函数在编译期可以静态推断出其所有 Key（如字符串字面量、显式/隐式整型索引、布尔型、浮点型），编译器会将其特化为 C 语言结构体：

1. **结构体布局生成**：编译器在编译期动态为该 Table 生成对应的 C 结构体布局，各个特化 Key 映射为结构体中固定偏移的成员变量。
2. **初始化与去重**：构造函数初始化时，会采用单次遍历在 JIT 特化结构体内进行填充（遵循 Lua 左到右的语法顺序），并进行静态 Key 重复性的检查。
3. **极速指针偏移读写**：对于特化的 Key 读取和写入操作，直接使用指针偏移宏（`FL_SPEC`/`FL_SET_SPEC`）进行极速读写，彻底避免了哈希查找与键值对比。
4. **动态降级**：如果读写时使用的 Key 是动态变量，则自动回退到常规运行时动态分发逻辑，通过注册在 Table 上的 `spec_get` / `spec_set` 专有函数指针进行回调查找。

```lua
-- 示例 Lua 代码：定义与读写 Table 字段
local point = { x = 10, y = 20 }
point.x = point.x + 5
```

编译器自动生成的特化 C 结构体与指针偏移访问代码：

```c
// 1. 编译期推断 Key 布局，自动生成 C 结构体定义
typedef struct Table_Spec_1 {
    CVar x;
    CVar y;
} Table_Spec_1;

// 2. 初始化 Table 时绑定专有 struct 布局与 spec 读写句柄
SET_TABLE_SPEC(point, Table_Spec_1, spec_get_fn, spec_set_fn, 2);
FL_SET_SPEC(Table_Spec_1, point, x, 0, (CVar){.type_ = VAR_INT, .data_.i = 10});
FL_SET_SPEC(Table_Spec_1, point, y, 1, (CVar){.type_ = VAR_INT, .data_.i = 20});

// 3. 字段读写转换为极速指针成员偏移（彻底免除哈希表查找开销）
FL_SPEC(Table_Spec_1, point, x) = NativeAdd(FL_SPEC(Table_Spec_1, point, x), (CVar){.type_ = VAR_INT, .data_.i = 5});
```

## 语言特性

### 已支持

- **闭包与 Upvalue 捕获**：静态 AST 分析自动推导作用域与跨函数捕获关系，被捕获变量自动提升为堆分配 `CVar *`，同作用域多闭包共享同一堆指针。
- **多返回值与可变参数**：函数可 `return a, b`，C++ 侧通过 `std::tie(a, b, c)` 接收。vararg 函数 `...` 完整支持。
- **匿名函数与高阶函数**：`function(args) body end` 作为值传递，支持任意 Callee 调用如 `tbl[key]()`。
- **冒号方法调用**：`obj:method(args)` 语法糖，自动将调用者作为 `self` 参数传递。
- **泛型 `for in` 迭代器**：支持无状态迭代器和闭包生成器，`pairs`/`ipairs` 保留原生 C 结构体极速循环。
- **循环变量独立捕获**：每次迭代自动重新 boxing 循环变量，确保闭包绑定独立副本。
- **Package 包管理**：`package "Name"` 命名空间隔离，零 `require` 跨模块互调。
- **全局变量复杂初始化**：文件级变量支持任意复杂表达式初始化器，在生成的 `__fakelua_init()` 中执行。
- **NativeObject 与 C++ 互操作**：支持组粒度 Arena 批量释放、C++ 成员方法 `RegisterMethod` 绑定、冒号语法调用。
- **ECMAScript 正则**：`string.find`/`match`/`gmatch`/`gsub` 底层使用 `std::regex`（支持前瞻、交替、非贪婪等能力，强于 Lua pattern）。

### 未支持

- **协程（Coroutine）**：不支持 `coroutine.create`/`resume`/`yield`。
- **元表（Metatable）**：不支持 `__index`、`__newindex` 等元方法。
- **`require`/`module`**：无标准模块系统（由 `package "Name"` 机制替代）。
- **`rawequal`/`rawget`/`rawset`/`rawlen`**：因无元表，这些函数无意义。
- **debug 标准库**：不支持 `debug.*`。
- **隐式类型转换**：算术中不做字符串→数字的隐式转换（`"10" + 1` 会报错）。

## 标准内置扩展库

FakeLua 在 `src/native/` 下提供 19 个独立 C++ 原生模块，覆盖数学、字符串、表、IO、网络、定时器、事件、压缩、加密、序列化、数据库、Protobuf 等领域。

> **完整 API 文档：** [src/native/README.zh.md](src/native/README.zh.md) / [English](src/native/README.md)

| 分类 | 模块 |
|------|------|
| 核心 Lua | `math`、`table`、`string`、`os`、`utf8`、`io` |
| 网络 | `net`（TCP 服务端/客户端）、`timer`、`event` |
| 数据 | `json`、`csv`、`serialize`、`protobuf` |
| 数据库 | `mysql`（异步 + 连接池）、`sqlite`（同步） |
| 加解密 | `compress`（LZ4/zlib/gzip/Zstd）、`crypto`（MD5/SHA/AES/RC4/Blowfish/DES） |
| 对象 | `object`（NativeObject Lua 侧 API） |

> ⚠️ `string.find`/`match`/`gmatch`/`gsub` 底层使用 **ECMAScript 正则**（`std::regex::ECMAScript`），而非 Lua pattern。从标准 Lua 迁移时需改写模式串。

### 正则匹配：ECMAScript 语法

| 用途 | Lua pattern | FakeLua（ECMAScript 正则） |
|---|---|---|
| 数字 | `%d` | `\\d` |
| 字母 | `%a` | `[A-Za-z]` |
| 字母或数字 | `%w` | `[A-Za-z0-9]`（注意 `\\w` 额外包含 `_`） |
| 空白 | `%s` | `\\s` |
| 惰性重复 | `-`（如 `.-`） | `?`（如 `.*?`） |
| 替换串捕获引用 | `%1`、`%0` | `$1`、`$&` |

> Lua 字符串中 `\d` 不是合法转义，正则里的反斜杠需写成 `"\\d+"`。FakeLua 不支持 `[[...]]` 长字符串。
>
> 兼容写法：用 `[0-9]+` 代替 `%d+`，`[A-Za-z]+` 代替 `%a+`，两种引擎语义一致。

主要差异：

- **`gsub` 替换串**使用 JS 风格：`$1`…`$9`、`$&`、`` $` ``、`$'`、`$$`
- **非法模式串不抛异常**：`std::regex_error` 被捕获后返回 `nil`
- **`plain` 参数**：传 `true` 退化为纯子串查找，绕过正则引擎，是最快路径
- **性能**：正则路径慢于 Lua 原生 pattern，热路径建议优先用 `plain` 查找

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

## 性能基准

对比 Lua 5.4、FakeLua TCC、FakeLua GCC，覆盖 Fibonacci、GCD、快速幂、线性求和、冒泡排序、筛质数等 11 类算法（Release `-O3` 模式）：

| 算法（典型参数） | Lua 5.4 | FakeLua TCC | FakeLua GCC |
|---|---|---|---|
| Fibonacci n=32 | 297.9 ms | 26.7 ms（**11.2x**↑） | 6.8 ms（**36.6x**↑） |
| Sum n=5000000 | 33.9 ms | 18.4 ms（**1.8x**↑） | 1.1 ms（**30.4x**↑） |
| Popcount n=100000 | 18.2 ms | 3.1 ms（**5.9x**↑） | 488.0 μs（**37.3x**↑） |
| BubbleSort n=200 | 1.5 ms | 3.3 ms（0.45x） | 738.8 μs（**1.9x**↑） |
| Sieve n=5000 | 353.4 μs | 1.0 ms（0.34x） | 219.3 μs（**1.8x**↑） |
| FloatPoly n=1000000 | — | — | **34.9x**↑（浮点特化，GCC 2x 快于 C++） |

> TCC 纯计算类场景普遍快于 Lua；在包含 Table 操作的场景下，Table 结构体特化使 GCC 与 TCC 的 Table 读写性能均大幅提升。完整数据见 [benchmark/README.zh.md](benchmark/README.zh.md) / [English](benchmark/README.md)。

## C++ API 详细文档

### 快速使用

```cpp
FakeluaStateGuard guard;
State* s = guard.GetState();
CompileFile(s, "script.lua", CompileConfig{.debug_mode = false});

int sum = 0;
Call(s, JIT_GCC, "add", sum, 10, 20); // 嵌入调用 Lua 函数
```

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

```cpp
// 原生 → FakeLua
CVar v_int = inter::NativeToFakelua(s, 42);
CVar v_str = inter::NativeToFakelua(s, std::string("hello"));

// FakeLua → 原生
int native_int = inter::FakeluaToNative<int>(v_int);
std::string native_str = inter::FakeluaToNative<std::string>(v_str);
```

### Table 与对象互转

```cpp
class CustomVar : public VarInterface { /* ... */ };
SetVarInterfaceNewFunc(s, []() { return new CustomVar(); });
// Call 中传递的 table 类型参数会自动构造为 CustomVar 实例
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
[文件级语句校验] → 拒绝非声明语句 (semantic_analysis)
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
| `lexer/parser` | Lua 词法和语法解析 |
| `syntax_tree` | AST 表示和遍历 |
| `preprocessor` | Lua 语法规范化（如 functiondef 提升） |
| `semantic_analysis` | 语义和控制流分析 |
| `type_inferencer` | 静态类型推导和 specialization 决策 |
| `c_gen` | C 代码生成和类型驱动优化 |
| `compile_common` | 公共类型推导和代码生成工具 |
| `jit/*` | TCC 和 GCC 后端集成 |
| `state` | FakeLua 运行时状态管理 |
| `var` | 动态值 CVar 和转换工具 |

## 常见问题

### Q: 为什么选择 Lua 子集而不是完整 Lua？
A: 完整 Lua 的某些动态特性（如 metatable）很难高效编译。子集实现聚焦于可静态分析的常见模式，通过类型推导和 JIT 编译获得接近 C 的性能。

### Q: TCC 和 GCC 后端如何选择？
A: **GCC** 是生产环境主力后端（`-O3` 生成高质量原生代码）；**TCC** 编译极快，主要用于开发调试和测试。

### Q: 可以在嵌入式环境中使用吗？
A: 可以，TCC 后端体积小、编译速度快。核心库依赖极少（仅 C++ 标准库），可交叉编译。

### Q: 如何调试生成的 C 代码？
A: 启用 `CompileConfig::debug_mode` 查看日志和 C 代码；使用 `GetLastRecordedCCode()` 导出 C 代码进行分析。

### Q: 支持多线程吗？
A: 每个 `State` 当前为线程本地对象，多线程环境中应为每个线程创建独立的 `State`。
