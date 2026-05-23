# FakeLua (开发中)
[<img src="https://img.shields.io/github/license/esrrhs/fakelua">](https://github.com/esrrhs/fakelua)
[<img src="https://img.shields.io/github/languages/top/esrrhs/fakelua">](https://github.com/esrrhs/fakelua)
[<img src="https://img.shields.io/github/actions/workflow/status/esrrhs/fakelua/build.yml?branch=master">](https://github.com/esrrhs/fakelua/actions)
[![codecov](https://codecov.io/gh/esrrhs/fakelua/graph/badge.svg?token=9ZCUH1Q632)](https://codecov.io/gh/esrrhs/fakelua)

FakeLua 是一个可嵌入的 Lua 子集编译引擎：将 Lua 脚本编译为 C 代码，通过 TinyCC 或 GCC 后端动态编译，生成机器码加载执行。提供 C++23 接口，支持脚本与原生代码高效互操作。

## 核心特性

### 编译与执行
- **多 JIT 后端**：支持 `JIT_TCC`（TinyCC，快速编译）和 `JIT_GCC`（完整优化）
- **自动类型推导**：静态分析 Lua 脚本，自动推导函数参数和返回值类型，生成高效类型化 C 代码
- **数值专特化**：根据调用上下文自动生成 int64/double 专特化版本，避免动态类型装箱开销
- **同一 API 多后端调用**：通过统一接口 `Call()` 在运行时选择 JIT 后端

### 互操作性
- **C++ 嵌入 API**：`CompileFile` / `CompileString` / `Call`，RAII 风格 `FakeluaStateGuard`
- **表对象映射**：通过 `VarInterface` 实现 Lua table 与原生对象双向转换
- **灵活参数转换**：支持基本类型、对象、以及自定义 VarInterface 实现的高级映射
- **C 代码导出**：支持记录编译生成的 C 代码用于调试和性能分析（`CompileConfig::record_c_code`）

### 代码生成
- **C 中间代码生成**：完整的 Lua 子集语法支持，生成结构清晰的 C 代码
- **类型驱动生成**：根据推导的类型生成原生类型操作（如 int64_t 加法而非动态 CVar 加法）
- **局部变量作用域管理**：完整支持 Lua 作用域，包括嵌套控制流块中的作用域重叠处理

## 当前已知限制（以测试与代码实现为准）

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
- 函数 specialization 基于调用点的 math 参数发现，不支持通过 varargs 泛化的参数

## 构建

### 系统要求
- **C++23** 编译器（GCC 11+ / Clang 16+ / MSVC 2022+）
- CMake 3.5+
- make 或 ninja

### Linux / macOS

最小依赖：
- `cmake`
- C/C++ 编译工具链（gcc/g++）
- `make` 或 `ninja`

> TinyCC 源码会在 CMake 配置阶段自动拉取（通过 CPM），并在构建目录编译，无需预装。

```bash
cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build --parallel
```

仅构建核心库与命令行工具（不含测试/基准）：

```bash
cmake --build build --target fakelua flua --parallel
```

### Windows（MSYS2 + MinGW）

参考 GitHub Actions CI 使用以下核心包：

- `git`
- `make`
- `mingw-w64-x86_64-gcc`
- `mingw-w64-x86_64-cmake`
- `mingw-w64-x86_64-ninja`
- `mingw-w64-x86_64-lua` （仅测试/基准需要）

示例：

```bash
cmake -S . -B build -G Ninja
cmake --build build --parallel
ctest --test-dir build -V
```

## 测试与基准

### 单元测试

```bash
cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build --parallel
ctest --test-dir build -V
```

**依赖**：单元测试和 benchmark 需要 Lua 开发包（头文件 `lua.h` 和库文件）。

- 在 Linux 上：`sudo apt-get install liblua5.4-dev` 或 `liblua5.3-dev`
- 在 macOS 上：`brew install lua`
- 在 Windows MSYS2 上：`pacman -S mingw-w64-x86_64-lua`

若本机未安装 Lua 开发包，可先仅构建核心库 `fakelua` 与命令行工具 `flua`：

```bash
cmake --build build --target fakelua flua --parallel
```

### 性能基准

```bash
./build/bin/bench_mark
```

包含算法性能和表操作基准测试，对比 FakeLua 与原生 Lua 性能。

### 测试覆盖

覆盖的测试场景包括：
- **语法树解析**：Lua 语法子集的完整解析测试
- **类型推导**：自动类型推导和 specialization 验证
- **运行时行为**：变量作用域、控制流、函数调用等
- **互操作性**：C++ 与 Lua 参数互转，VarInterface 映射
- **异常处理**：编译错误、运行时错误捕获
- **算法性能**：JIT 编译代码性能对标

## 命令行工具 `flua`

构建后可执行文件位于 `build/bin/flua`（Windows 为 `flua.exe`）。

```bash
./build/bin/flua <script.lua> --entry=<func> --jit_type=<0|1> --repeat=<N> [--debug]
```

**选项**：
- `--entry`：入口函数名（默认 `main`）
- `--jit_type`：JIT 后端选择
  - `0` = `JIT_TCC`（TinyCC，快速编译）
  - `1` = `JIT_GCC`（完整优化）
- `--repeat`：重复调用入口函数的次数（默认 1）
- `--debug`：启用调试模式，输出编译日志和 C 代码（等价于 `CompileConfig::debug_mode`）

**示例**：

```bash
# 用 TCC 编译并执行，调用 test 函数
./build/bin/flua script.lua --entry=test --jit_type=0 --repeat=100

# 用 GCC 编译，启用调试查看生成的 C 代码
./build/bin/flua script.lua --entry=main --jit_type=1 --debug
```

## C++ 嵌入 API

### 基本使用

```cpp
#include "fakelua.h"
using namespace fakelua;

int main() {
    // RAII 风格管理 FakeLua 状态
    FakeluaStateGuard guard;
    State* s = guard.GetState();

    // 编译 Lua 脚本文件
    CompileFile(s, "script.lua", CompileConfig{
        .debug_mode = false,
        .record_c_code = true  // 记录生成的 C 代码
    });

    // 调用编译后的 Lua 函数（支持 JIT 后端选择）
    int ret = 0;
    Call(s, JIT_TCC, "test", ret, 123);
    // 或：Call(s, JIT_GCC, "test", ret, 123);
    
    std::cout << "Result: " << ret << std::endl;
    
    // 查看生成的 C 代码（record_c_code=true 时可用）
    std::string c_code = GetLastRecordedCCode(s);
    std::cout << "Generated C code:\n" << c_code << std::endl;
    
    return 0;
}
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

### 类型系统

- **编译期推导**：基于 call sites 的数据流分析，推导参数和返回值类型
- **数值专特化**：函数自动生成 int64_t 和 double 两个专特化版本
- **类型表示**：T_DYNAMIC（动态）、T_INT（int64_t）、T_FLOAT（double）、T_BOOL、T_STRING、T_TABLE
- **装箱优化**：推导为基础数值类型时，生成原生类型操作，避免 CVar 装箱开销

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
A: 启用 `CompileConfig::debug_mode` 或 `--debug` 标志，查看日志和 C 代码；使用 `GetLastRecordedCCode()` 导出 C 代码进行分析。

### Q: 支持多线程吗？
A: 每个 `State` 当前为线程本地对象，多线程环境中应为每个线程创建独立的 `State`。
