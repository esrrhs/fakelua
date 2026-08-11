# Fakelua Fuzz Testing

基于 [libFuzzer](https://llvm.org/docs/LibFuzzer.html) 的模糊测试基础设施，用于自动发现 fakelua 编译器/运行时的 crash、内存错误和与标准 Lua 5.4 的行为差异。

## CI 流水线

`.github/workflows/fuzz.yml` 提供自动化 fuzz 测试：

| 触发条件 | 运行时长 (每 target) |
|----------|---------------------|
| Pull Request | 60 秒（快速冒烟） |
| Push to master | 5 分钟 |
| 每日定时 (3:37 UTC) | 5 分钟 |
| 手动触发 (`workflow_dispatch`) | 可配置 |

两个 fuzz target (`fuzz_compile`, `fuzz_differential`) 并行运行，发现 crash 时自动上传产物（crash 输入文件 + 日志）供排查。

## 架构

```
fuzz/
├── CMakeLists.txt              # 构建配置
├── fuzz_compile.cpp            # 编译期 fuzz target
├── fuzz_differential.cpp       # 差分 fuzz target（fakelua vs Lua 5.4）
├── lua_keywords.dict           # libFuzzer 字典（Lua 关键字/模式）
├── corpus/
│   └── seed/                   # 种子语料
│       ├── basic_assign.lua    #   变量赋值
│       ├── function_simple.lua #   函数定义与调用
│       ├── for_loop.lua        #   for 循环
│       ├── if_else.lua         #   条件分支
│       ├── table_array.lua     #   数组表
│       ├── table_hash.lua      #   哈希表
│       ├── string_ops.lua      #   字符串操作
│       └── closure.lua         #   闭包
└── README.md
```

## Fuzz Target 说明

### fuzz_compile — 编译期健壮性测试

测试 `CompileString()` 对任意输入的健壮性。检测：

- Parser / Lexer / Compiler pipeline 中的 crash / segfault
- ASan 检测的内存错误（越界读写、use-after-free、double-free）
- UBSan 检测的未定义行为（空指针解引用、整数溢出）
- 非预期的异常类型

**策略**：只编译，不执行。对任意输入调用 `CompileString`，捕获 `FakeluaException`（预期行为），任何其他 crash 或异常都是 bug。

### fuzz_differential — 差分测试（fakelua vs Lua 5.4）

同一个 Lua 脚本分别在 fakelua 和 Lua 5.4 中编译执行，对比结果：

| 场景 | fakelua | Lua 5.4 | 判断 |
|------|---------|---------|------|
| 语法错误 | ❌ 编译失败 | ❌ 编译失败 | ✅ 正常（无效语法） |
| 有效的 Lua，fakelua 子集外 | ❌ | ✅ | ⚠️ 已知限制 |
| fakelua 接受，Lua 拒绝 | ✅ | ❌ | 🔴 **潜在 bug** |
| 都编译成功，调用结果不同 | ret=X | ret=Y | 🔴 **潜在 bug** |
| 一端调用成功，一端失败 | ✅ | ❌ | 🔴 **潜在 bug** |

## 构建

### 一键构建（推荐）

```bash
./fuzz/build_fuzz.sh          # 构建
./fuzz/build_fuzz.sh test     # 快速验证（1000 runs）
./fuzz/build_fuzz.sh clean    # 清理
```

此脚本自动完成两阶段构建：
1. GCC 编译 `fuzz_bridge` + `libfakelua.so`（处理 C++20 头文件）
2. clang + libFuzzer 编译 fuzz target，链接 GCC 产物

### CI 构建（单阶段 clang）

在 CI 环境（ubuntu-24.04, clang 16+）中，clang 可直接编译整个项目：

```bash
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_C_COMPILER=clang \
  -DBUILD_FUZZ=ON

cmake --build . --target fuzz_bridge -j$(nproc)

# 手动编译 fuzz target
clang++ -fsanitize=fuzzer,address,undefined -g \
  -I../include -I../src -I../src/platform -I../fuzz \
  ../fuzz/fuzz_compile.cpp \
  -L lib64 -L src -lfuzz_bridge -lfakelua \
  -Wl,-rpath,lib64 -Wl,-rpath,src \
  -o bin/fuzz_compile
```

## 运行

### 单进程快速测试

```bash
# 编译期 fuzz（只测编译器不崩溃）
./bin/fuzz_compile -max_len=4096 -runs=100000

# 差分 fuzz（对比 fakelua 与 Lua 5.4）
./bin/fuzz_differential -max_len=2048 -runs=100000
```

### 多进程并行（推荐长期运行）

```bash
# 4 个 worker 并行，每个独立探索
./bin/fuzz_compile -jobs=4 -workers=4 -max_len=4096

# 使用字典加速语法级探索
./bin/fuzz_compile -dict=../fuzz/lua_keywords.dict -jobs=4 -workers=4 -max_len=4096
```

### 复现 crash

```bash
# libFuzzer 发现 crash 后会保存触发输入到当前目录
# 文件名类似: crash-<hash>, slow-unit-<hash>, leak-<hash>

# 直接回放复现
./bin/fuzz_compile crash-<hash>

# GDB 调试
gdb --args ./bin/fuzz_compile crash-<hash>
```

### 长期运行建议

```bash
# 编译期 fuzz（并行、字典辅助、长时间运行）
nohup ./bin/fuzz_compile \
  -dict=../fuzz/lua_keywords.dict \
  -jobs=4 -workers=4 \
  -max_len=8192 \
  -max_total_time=86400 \
  > fuzz_compile.log 2>&1 &

# 差分 fuzz（并行、字典辅助、长时间运行）
nohup ./bin/fuzz_differential \
  -dict=../fuzz/lua_keywords.dict \
  -jobs=4 -workers=4 \
  -max_len=4096 \
  -max_total_time=86400 \
  > fuzz_differential.log 2>&1 &
```

## libFuzzer 常用参数

| 参数 | 说明 | 示例 |
|------|------|------|
| `-max_len=N` | 最大输入长度 | `-max_len=4096` |
| `-runs=N` | 执行 N 次后停止 | `-runs=1000000` |
| `-max_total_time=N` | 运行 N 秒后停止 | `-max_total_time=3600` |
| `-jobs=N` | 总 job 数 | `-jobs=4` |
| `-workers=N` | worker 数 | `-workers=4` |
| `-dict=FILE` | 使用字典 | `-dict=lua_keywords.dict` |
| `-seed=N` | 随机种子（复现用） | `-seed=12345` |
| `-detect_leaks=0` | 关闭内存泄漏检测 | `-detect_leaks=0` |

## 发现 bug 后

1. 用最小化工具缩减输入：
   ```bash
   ./bin/fuzz_compile crash-<hash> -minimize_crash=1 -runs=100000
   ```
2. 用 GDB 获取堆栈：
   ```bash
   gdb --args ./bin/fuzz_compile minimized-crash
   ```
3. 将最小化后的 crash 文件保存到 `corpus/crashes/` 并提交修复
