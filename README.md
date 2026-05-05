# FakeLua (开发中)
[<img src="https://img.shields.io/github/license/esrrhs/fakelua">](https://github.com/esrrhs/fakelua)
[<img src="https://img.shields.io/github/languages/top/esrrhs/fakelua">](https://github.com/esrrhs/fakelua)
[<img src="https://img.shields.io/github/actions/workflow/status/esrrhs/fakelua/build.yml?branch=master&label=Linux">](https://github.com/esrrhs/fakelua/actions/workflows/build.yml)
[<img src="https://img.shields.io/github/actions/workflow/status/esrrhs/fakelua/build_with_macos.yml?branch=master&label=macOS">](https://github.com/esrrhs/fakelua/actions/workflows/build_with_macos.yml)
[<img src="https://img.shields.io/github/actions/workflow/status/esrrhs/fakelua/build_with_windows.yml?branch=master&label=Windows">](https://github.com/esrrhs/fakelua/actions/workflows/build_with_windows.yml)
[![codecov](https://codecov.io/gh/esrrhs/fakelua/graph/badge.svg?token=9ZCUH1Q632)](https://codecov.io/gh/esrrhs/fakelua)

FakeLua 是一个可嵌入的 Lua 子集执行引擎：将脚本编译为 C 代码，再由本地编译器生成动态库并加载执行。

## 当前特性

- 支持双 JIT 运行类型：`JIT_TCC`、`JIT_GCC`
- 可通过同一套 API 在不同 JIT 后端调用同名函数
- 提供 C++ 嵌入接口（`CompileFile` / `CompileString` / `Call`）
- `Call()` 最多支持 8 个参数，返回值类型自动转换
- 支持将 Lua table 与原生侧对象通过 `VarInterface` 互转
- 支持记录最近一次编译生成的 C 代码（`CompileConfig::record_c_code`）
- 支持全局调试日志级别设置（`SetDebugLogLevel`）
- 支持通过 `StateConfig` 配置 TCC / GCC 的头文件路径、库路径等
- 支持 Linux、macOS、Windows（MSYS2 + MinGW）

## 当前已知限制（以测试与代码实现为准）

- 不支持多返回值
- 不支持 varargs（`...`）
- 不支持 `label` / `goto`
- 泛型 `for in` 仅支持 `pairs()` / `ipairs()`
- 全局变量初始化有约束（如不支持 table constructor、部分复杂表达式）
- 脚本侧函数调用目前仅支持简单函数名调用（不支持复杂前缀表达式调用形式）

## 构建

### Linux

依赖（最小）：

- `cmake`
- C/C++ 编译工具链（如 gcc/g++）
- `make` 或 `ninja`

> TinyCC 源码会在 CMake 配置阶段自动拉取并在构建目录编译，无需系统预装 tinycc。

```bash
cmake -S . -B build
cmake --build build --parallel
```

仅构建核心库与命令行工具（不含测试/基准）：

```bash
cmake --build build --target fakelua flua --parallel
```

### macOS

依赖通过 Homebrew 安装：

```bash
brew install lua cmake
```

构建：

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH="$(brew --prefix)"
cmake --build build --parallel
ctest --test-dir build -V
```

### Windows（MSYS2 + MinGW）

参考 CI 使用以下核心包：

- `git`
- `make`
- `mingw-w64-x86_64-gcc`
- `mingw-w64-x86_64-cmake`
- `mingw-w64-x86_64-ninja`
- `mingw-w64-x86_64-lua`

示例：

```bash
cmake -S . -B build -G Ninja
cmake --build build --parallel
ctest --test-dir build -V
```

## 测试与基准

```bash
ctest --test-dir build -V
./build/bin/bench_mark
```

> 单元测试与 benchmark 依赖 Lua 开发头文件（`lua.hpp`）及 `lua` 库。<br>
> 若本机未安装 Lua 开发包，可先仅构建 `fakelua` 与 `flua` 目标。

## 命令行工具 `flua`

构建后可执行文件在 `build/bin/flua`（Windows 为 `flua.exe`）。

```bash
./build/bin/flua <script.lua> --entry=<func> --jit_type=<0|1> --repeat=<N> [--debug]
```

- `--entry`：入口函数名（默认 `main`）
- `--jit_type`：`0`=`JIT_TCC`，`1`=`JIT_GCC`
- `--repeat`：重复调用次数
- `--debug`：开启调试模式（传递到 `CompileConfig::debug_mode`）

## C++ 嵌入示例

```cpp
#include "fakelua.h"
using namespace fakelua;

int main() {
    FakeluaStateGuard guard;
    State* s = guard.GetState();

    CompileFile(s, "script.lua", CompileConfig{.debug_mode = false});

    int ret = 0;
    Call(s, JIT_TCC, "test", ret, 123);
    // 或：Call(s, JIT_GCC, "test", ret, 123);
}
```
