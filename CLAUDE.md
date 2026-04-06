# FakeLua Project Overview

FakeLua 是一个高性能、可嵌入的 Lua 脚本执行引擎，实现了 Lua 5.4 语法的子集。核心创新是将 Lua 脚本转译为 C 代码，通过 TCC/GCC 编译为原生机器码执行。

## 核心技术栈
- **语言**: C++23
- **词法/语法分析**: Flex + Bison
- **JIT 后端**: TCC (Tiny C Compiler) 或 GCC JIT
- **构建系统**: CMake
- **测试**: GoogleTest

## 编译流程 (Pipeline)

```
Lua 源码 → 词法分析(Flex) → 语法解析(Bison) → AST → 预处理 → C代码生成 → JIT编译 → 原生机器码
```

关键文件：
- `src/compile/compiler.cpp`: 编译器主类，驱动整个流程
- `src/compile/syntax_tree.h`: AST 节点定义
- `src/compile/c_gen.cpp`: C 代码生成器
- `src/jit/tcc_jit.cpp`: TCC JIT 实现

## 目录结构

```
src/
├── compile/          # 编译前端：词法、语法、AST、预处理、C代码生成
├── jit/              # JIT 后端：TCC/GCC 编译、VM 函数管理
├── state/            # 状态管理：执行状态、堆、栈、常量字符串池
├── var/              # 变量类型：Var、VarString、VarTable
├── util/             # 工具库：日志、异常、字符串处理
└── platform/         # 平台相关：TCC 预编译库
include/fakelua.h     # 公共 API
test/                 # 测试用例 (lua/jit, lua/syntax, lua/exception)
benchmark/            # 性能基准测试
```

## 核心数据结构

### Var / CVar (16字节通用变量)
```cpp
struct CVar {
    int type_;      // 0=Nil, 1=Bool, 2=Int, 3=Float, 4=String, 5=StringId, 6=Table
    int flag_;
    union { bool b; int64_t i; double f; VarString *s; VarTable *t; } data_;
};
```

### VarTable (混合模式哈希表)
- 元素 ≤ 8：线性扫描 `quick_data_[]`，无哈希开销
- 元素 > 8：拉链法哈希表，`active_list_` 提供紧凑遍历
- 性能：Set 比 unordered_map 快 1.3-3x，Iterate 快 1.5-2.7x

## 公共 API (include/fakelua.h)

```cpp
// 创建/销毁状态
State *FakeluaNewState(const StateConfig &cfg = {});
void FakeluaDeleteState(State *s);

// 编译
void CompileFile(State *s, const std::string &filename, const CompileConfig &cfg);
void CompileString(State *s, const std::string &str, const CompileConfig &cfg);

// 调用函数（类型安全）
template<typename Ret, typename... Args>
void Call(State *s, const std::string_view &name, Ret &ret, Args &&...args);
```

## 开发约定

### 代码风格
- 命名空间：`fakelua`
- 类/结构体/枚举：PascalCase (如 `VarTable`, `SyntaxTreeType`)
- 函数：PascalCase (如 `FakeluaNewState`, `NativeToFakelua`)
- 枚举转字符串：使用后缀 `ToString` 的自定义函数 (如 `VarTypeToString`)

### 日志与错误
- 日志宏：`LOG_INFO`, `LOG_ERROR`, `LOG_DEBUG` (定义在 `src/util/logging.h`)
- 异常：`ThrowFakeluaException()` (定义在 `src/util/exception.h`)

### JIT 注意事项
- JIT 调用的辅助函数必须声明为 `extern "C" __attribute__((used))`
- 关键外部函数：`FakeluaAllocTemp`, `FakeluaThrowError`

### C 代码生成标准
- 算术运算使用 GCC Statement Expressions `({ ... })` 消除函数调用开销
- 哈希算法：DJB2，`hash_` 初始化为 0 支持延迟计算
- 相等语义：`VarEqual` (C) 与 `Var::Equal` (C++) 必须保持一致，支持 `VAR_STRING` 与 `VAR_STRINGID` 交叉比较

## 与标准 Lua 的差异
- 无全局变量（只有全局常量）
- 无垃圾回收（使用内存池）
- 无闭包、协程、线程、元表
- 表仅支持哈希表实现

## 构建命令

### Linux
```bash
# 依赖: cmake, flex, bison, TCC
mkdir build && cd build
cmake ..
make
```

### Windows (MinGW)
```powershell
# 安装 MSYS2, mingw-w64-x86_64-gcc, mingw-w64-x86_64-lua
mkdir build
cd build
cmake ..
cmake --build .
```

## 测试
```bash
./bin/test_fakelua    # 单元测试
./bin/bench_mark      # 性能测试
```

## 开发状态
- ✅ 词法/语法分析、AST 构建、预处理
- ✅ C 代码生成、TCC JIT 编译
- ✅ 基本运算、表操作、函数定义调用、控制流
- 🚧 GCC JIT 后端、更多标准库、闭包支持
