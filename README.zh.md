# FakeLua
[<img src="https://img.shields.io/github/license/esrrhs/fakelua">](https://github.com/esrrhs/fakelua)
[<img src="https://img.shields.io/github/languages/top/esrrhs/fakelua">](https://github.com/esrrhs/fakelua)
[<img src="https://img.shields.io/github/actions/workflow/status/esrrhs/fakelua/build.yml?branch=master&label=Linux">](https://github.com/esrrhs/fakelua/actions/workflows/build.yml)
[<img src="https://img.shields.io/github/actions/workflow/status/esrrhs/fakelua/build_with_macos.yml?branch=master&label=macOS">](https://github.com/esrrhs/fakelua/actions/workflows/build_with_macos.yml)
[<img src="https://img.shields.io/github/actions/workflow/status/esrrhs/fakelua/build_with_windows.yml?branch=master&label=Windows">](https://github.com/esrrhs/fakelua/actions/workflows/build_with_windows.yml)
[![codecov](https://codecov.io/gh/esrrhs/fakelua/graph/badge.svg?token=9ZCUH1Q632)](https://codecov.io/gh/esrrhs/fakelua)

中文 | [English](README.md)

FakeLua 是面向高性能宿主的可嵌入 Lua 子集运行时：脚本可编译为字节码由内置虚拟机执行，也可经 GCC/TCC JIT 生成原生机器码；并内置大量 C++ native 标准库（网络/HTTP/数据库/加密等），采用 Arena 内存池 + 帧重置，无 GC 停顿。

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

### 3. 工程化与语言设计哲学：严谨取舍，拒绝无脑照搬
FakeLua 并非无脑照搬标准 Lua 的所有动态弱约束特性，而是在**执行性能**、**大规模工程协同**和**代码易维护性**上做出了有意识的取舍：

*   **定参函数实参个数严格匹配（Strict Arity Enforcement）**：
    标准 Lua 在调用定参函数时，少传参数会自动补 `nil`，多传参数会被默默丢弃。在大规模多人协作项目中，这种过度宽松的特性极易掩盖接口签名变更、遗漏必要传参或参数错位等隐蔽 bug。FakeLua 刻意严格收紧该语义：
    - 直接调用在**编译期**严格校验参数个数，不一致直接报编译错误；
    - 动态/按名分发在**运行期**严格拦截，实参个数不匹配直接抛出异常；
    - 需要变长实参时必须显式声明可变参数（`...`）。严格匹配不仅让接口契约清晰透明、杜绝隐蔽缺陷，也让 JIT 能够生成最纯粹高效的原生 C 调用，省去运行时动态补 nil 和截断的冗余指令。
*   **杜绝隐式全局变量**：变量必须显式使用 `local` 声明，未声明直接赋值会在编译期拦截，彻底消除拼写错误带来的全局污染。
*   **杜绝隐式类型转换**：算术运算不做字符串到数字的隐式转换（如 `"10" + 1` 明确报错），确保计算意图可预测，支撑编译器激进的原生数值特化。
*   **原生函数静态分发**：C++ 原生函数作为高效宿主接口，在编译期按名静态分发而非作为第一类值闭包，大幅提升热点调用性能。

## 核心特性

### 三种执行后端

同一套 `Call` API 可切换任意已注册后端：

- **JIT_GCC**：调用系统 GCC（`-O3`）生成高质量原生代码，生产环境主力。
- **JIT_TCC**：内嵌 TinyCC，编译极快，适合开发调试与测试（CMake 自动拉取 TCC，无需系统预装）。
- **JIT_INTERP**：编译为 FakeLua 字节码，由内置解释器执行（`src/interp/`）。无需宿主 C 编译器，便于移植、工具链以及 JIT↔解释器混合闭包。

```cpp
int ret = 0;
Call(s, JIT_GCC, "add", ret, 10, 20);    // 生产：GCC (-O3)
Call(s, JIT_TCC, "add", ret, 10, 20);    // 开发/测试：TCC（极速编译）
Call(s, JIT_INTERP, "add", ret, 10, 20); // 字节码虚拟机（无需宿主 C 编译器）
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
- **Lua 5.4 模式匹配**：`string.find`/`match`/`gmatch`/`gsub` 使用自包含的 Lua 模式引擎（`%d`/`%w` 字符类、自定义集合、惰性 `-`、捕获、前沿 `%f[set]`、平衡 `%bxy`），与标准 Lua 完全兼容。
- **字符串算法**：`string.trim`/`trim_left`/`trim_right`/`split`/`join`/`replace`/`starts_with`/`ends_with`/`contains`/`iequals`/`icontains`/`istarts_with`/`iends_with` 底层使用 Boost.Algorithm。

### 未支持 / 与标准 Lua 的设计差异

- **函数实参个数严格匹配**：定参函数的实参与形参个数必须严格一致，少传或多传参数会在编译期/运行时直接报错，杜绝标准 Lua 隐式补 nil 或静默截断掩盖的接口 bug（变长参数请显式声明 `...`）。
- **协程（Coroutine）**：不支持 `coroutine.create`/`resume`/`yield`。
- **元表（Metatable）**：不支持 `__index`、`__newindex` 等元方法。
- **`require`/`module`**：无标准模块系统（由 `package "Name"` 机制替代）。
- **`rawequal`/`rawget`/`rawset`/`rawlen`**：因无元表，这些函数无意义。
- **debug 标准库**：不支持 `debug.*`。
- **隐式类型转换**：算术中不做字符串→数字的隐式转换（`"10" + 1` 会报错）。
- **隐式全局变量**：局部变量必须使用 `local` 声明，不支持隐式全局赋值。
- **文件级 local 为 const 绑定**：初始化后不可整槽再赋，`__fakelua_init` 创建的 Table 深 const；跨帧可变状态请用 NativeObject / `new_global_obj`。
- **`json.encode` 拒绝数组空洞**：整数键就代表数组意图——键都是正整数但不从 1 起连续的表
  （`t[1]="a"; t[3]="c"`，甚至 `{[1001]=...}`）一律报错，不再退化成 `{"1":"a","3":"c"}`：上线的
  数据形状不许静默改变。协议数组请保持无洞（用 `""` 或 `{}` 占位，别留 nil）；要按 id 建映射就
  用字符串键（或 `container.map`）。`#t` 仍与标准 Lua 一致只数到第一个 nil；
  `serialize.encode/decode` 会保留非连续键。
- **运行期错误带 Lua 源码位置**：三个后端都会输出形如 `attempt to perform arithmetic on non-numeric value at main.lua:4` 的消息，原生函数里抛出的错误同样定位到调用它的那条语句。

## 标准内置扩展库

FakeLua 在 `src/native/` 下提供 30+ 个独立 C++ 原生模块（每个 `State` 创建时自动注册），覆盖数学、字符串、表、IO、网络、定时器、事件、随机数、容器、压缩、加密、序列化、数据库、Protobuf、配置解析、日志、子进程等领域。

> **完整 API 文档：** [src/native/README.zh.md](src/native/README.zh.md) / [English](src/native/README.md)

| 分类 | 模块 |
|------|------|
| 核心 Lua | `basic`、`math`、`table`、`string`、`os`、`utf8`、`io`、`random` |
| 运行时 / IO | `runtime`（`runtime.tick()`）、`net`（TCP/UDP）、`http`（HTTP/1.1）、`url`、`timer`、`event` |
| 数据 | `json`、`csv`、`serialize`、`protobuf`、`container`（Boost.Container deque/vector/list/map/set） |
| 配置解析 | `yaml`、`toml`、`xml`、`ini` |
| 数据库 | `mysql`（异步 + 连接池）、`redis`（异步）、`sqlite`（同步） |
| 加解密 / 压缩 | `compress`（LZ4/zlib/gzip/Zstd）、`crypto`（OpenSSL 摘要/对称加密、UUID、CRC-32、xxHash） |
| 进程 | `process`（`process.run`；不替换 `os.execute`） |
| 日志 | `log`（级别、分类标签输出、文件滚动） |
| 对象 | `object`（NativeObject Lua 侧 API） |

> `string.find`/`match`/`gmatch`/`gsub` 使用 **Lua 5.4 模式**（自包含字节模式引擎，位于 `src/native/string/lua_pattern.*`），不是 ECMAScript/POSIX 正则。

### Lua 模式匹配

`string.find`/`match`/`gmatch`/`gsub` 严格遵循 PUC-Rio Lua 5.4 语义，包括：

- **转义**：标点用 `%.` `%(` `%)` `%%` `%+` 等；`.` 匹配任意单字节。
- **字符类**：`%a %c %d %g %l %p %s %u %w %x`，大写取反；`%z` 匹配零字节。
- **集合/区间**：`[set]`、`[^set]`、`[a-z]`、集合内嵌字符类（`[%d_]`）、开头的 `]`/`-` 按字面量处理。
- **量词**：`* + - ?`，含 Lua 独有的惰性 `-`（`a.-b`）。
- **锚点**：模式开头的 `^` 与末尾的 `$`。
- **捕获**：嵌套 `(...)`、位置捕获 `()`、模式内反向引用 `%1`…，gsub 替换串 `%0`…`%9` 与 `%%`。
- **前沿模式** `%f[set]` 与**平衡匹配** `%bxy`。
- **gsub**：替换值可为字符串/函数/表；函数或表返回 `nil`/`false` 时保留原匹配；`find` 的 `plain=true` 跳过模式引擎。
- **非法模式直接报错**（可被 `pcall` 捕获），而不是静默返回不匹配。

```lua
string.match("limit=15", "%d+")                          --> "15"
string.gsub("hello world", "(%w+) (%w+)", "%2 %1")      --> "world hello", 1
string.match("a(b(c)d)e", "%b()")                       --> "(b(c)d)"
```

> Lua 模式**不是**正则：没有分支交替（`a|b`），转义前缀是 `%` 而非 `\`。为旧 ECMAScript 行为写的脚本（如 `"\\d+"`、`$1` 替换）需改写为 Lua 写法（`"%d+"`、`"%1"`）。

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

依赖由 CPM 拉取（`CPM_SOURCE_CACHE`，默认 `~/.cache/CPM`）。Boost 是超级构建，每个库都是一
个 git 子模块，缓存里可能「有仓库对象但子模块工作区是空的」，此时 configure 会一个接一个地
报缺 `Boost::*` target。检测到这种情况时 configure 会直接停下并给出修复命令：

```bash
cd ~/.cache/CPM/boost/<hash>
git submodule update --init --depth 1 --jobs 8
git submodule foreach --recursive 'if [ -e .git ]; then git checkout HEAD -- .; fi'
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
./build/bin/flua <script.lua | dir/> [--once] [--jit_type=<0|1|2>]
```

入口名固定为 package `main` 下的三个函数，不提供改名参数，所有应用写法一致：

```lua
package "main"

function init() ... end      -- 启动时调用一次
function update() ... end    -- 每帧调用一次
function fini() ... end      -- 退出前调用一次
```

- `<script.lua | dir/>`：单个脚本，或一个目录（把目录下所有 `.lua` 按文件名排序编译）
- `--once`：只跑 `init` + `fini`，不进 `update` 循环（一次性脚本 / 自检）
- `--jit_type`：`0`=TCC，`1`=GCC，`2`=INTERP（字节码虚拟机）。只编译选中的后端：选 INTERP
  就不必再为每个文件跑一遍 GCC（约 0.6 秒）去产出永远不会执行的代码。
- `--log_level`：最低日志级别，默认 `3`=Warn（`0`=Trace … `6`=Off）。常驻服务不该被逐文件的
  INFO 编译日志盖掉真正的错误；想看回来就传 `--log_level=2`。
- `--debug`：是否启用调试模式（默认 `false`，若为 `true` 则输出生成的 C 源码 / 更详细诊断）

`update` 返回后宿主立刻再调一次；需要固定节奏的脚本自己在 `update` 末尾 `os.sleep(毫秒)`
（fakelua 的 `os.sleep` 单位是毫秒）。

优雅退出（两种都会先执行 `fini`）：`update` 返回非 0，或 Lua 侧调 `flua.set_exit(code)`。
注意 `os.exit` 仍是立即退出、跳过 `fini`。信号不由 flua 处理——要不要接管
SIGINT / SIGTERM 是应用自己的决定。

三个阶段各自是一次独立的顶层 Call。这一点很关键：fakelua 没有 GC，帧 arena 只在顶层 Call
入口（reentrant 计数回到 0）重置，若把 `while true` 写在一个 Lua Call 里，arena 会无限增长。
`update` 里只做一帧的活（泵 IO/定时器、分发请求）。

### 安装与 CMake 集成

#### 安装到系统

```bash
# 编译
cmake -S . -B build
cmake --build build --parallel

# 安装（默认前缀：/usr/local）
sudo cmake --install build
# 或：cd build && sudo make install
```

`flua` 是单独的 component（`cli`）并标了 `OPTIONAL`，所以「只构建库」之后再执行安装不会再
因为找不到可执行文件而失败：

```bash
cmake --build build --target fakelua --parallel
sudo cmake --install build            # 装库，跳过没构建出来的 flua
sudo cmake --install build --component cli   # 或者只装命令行工具
```

#### 在其他 CMake 项目中使用

安装到系统后，其他 CMake 项目可以通过标准的 `find_package` 引入并链接 FakeLua：

```cmake
cmake_minimum_required(VERSION 3.20)
project(my_project CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找 FakeLua
find_package(fakelua REQUIRED)

add_executable(my_project main.cpp)

# 链接 fakelua（自动引入头文件路径和链接参数）
target_link_libraries(my_project PRIVATE fakelua::fakelua)
# 同时兼容不带命名空间的别名写法：
# target_link_libraries(my_project PRIVATE fakelua)
```

在 C++ 代码中引用头文件：

```cpp
#include "fakelua.h"
// 或者使用带目录前缀的形式：
// #include <fakelua/fakelua.h>
```

FakeLua 还会同时安装 `fakelua.pc`，支持通过 `pkg-config` 使用。

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
| `SetDebugLogLevel(s, level)` | 设置本 State 的日志级别（`LogLevel::Warn` … `LogLevel::Trace`；Lua 侧用 `log.set_level`） |

### 类型转换

宿主代码请用公开助手；`inter::` 那套转换是解释器与 JIT 的内部实现，宿主不该依赖。

```cpp
// 原生回调里读参数（参数缺失或为 nil 时取默认值）
int64_t id = GetNativeIntArg(s, args, n, 0, 0);
std::string name = GetNativeStringArg(s, args, n, 1, "anon");
bool flag = GetNativeBoolArg(s, args, n, 2, false);
double ratio = GetNativeFloatArg(s, args, n, 3, 1.0);

// 返回值
return NativeString(s, name);   // 另有 NativeInt / NativeBool / NativeFloat / NativeNil
```

### 注册原生函数

两种写法。回调式自己读 `CVar` 数组并声明形参个数（`arg_count` + `is_vararg`）：

```cpp
RegisterNativeFunction(s, "greet", 1, false,
    [](State *s, CVar *args, int n) -> CVar {
        return NativeString(s, "hello " + GetNativeStringArg(s, args, n, 0));
    });
```

强类型模板把「首参是 `State *`」的可调用对象包成原生函数。模板形参是 `<Ret, Args...>`，
所以函数类型必须写在 `std::function<>` 里，只有这一种写法能编译：

```cpp
RegisterNativeFunction(s, "hp_of", false,
    std::function<int64_t(State *, int64_t)>([](State *, int64_t id) { return id * 2; }));
```

写成 `RegisterNativeFunction<std::string(State *, int64_t)>(...)` 会把整个函数类型当成
`Ret`；现在头文件用 `static_assert` 直接点出这一点，不再刷出一屏
「function cannot return function type」。

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
        ┌─────────────────────────────┬──────────────────────────────┐
        ↓                             ↓                              ↓
[C 代码生成]                      [字节码生成]                    （共享 AST）
   (c_gen)                         (interp/codegen)
        ↓                             ↓
[JIT TCC / GCC]                   [解释器虚拟机]
   原生机器码                        (interp/interpreter)
        └───────────── Call(s, JIT_*, …) ─────────────┘
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
| `interp/*` | 字节码生成、指令集与解释器虚拟机 |
| `compile_common` | 公共类型推导和代码生成工具 |
| `jit/*` | TCC/GCC 后端与 `Vm` 函数注册表 |
| `native/*` | 内置标准库（net、http、db、crypto 等） |
| `state` | FakeLua 运行时状态管理 |
| `var` | 动态值 CVar 和转换工具 |

## 常见问题

### Q: 为什么选择 Lua 子集而不是完整 Lua？
A: 完整 Lua 的某些动态特性（如 metatable）很难高效编译。子集实现聚焦于可静态分析的常见模式，通过类型推导和 JIT 编译获得接近 C 的性能。

### Q: TCC、GCC 与 INTERP 如何选择？
A: **GCC** 是生产环境主力（`-O3`）。**TCC** 编译极快，适合开发与 CI。**INTERP** 跑字节码、无需宿主 C 编译器，适合受限环境、工具链与语义校验；热路径仍可与 JIT 闭包混合调用。

### Q: 可以在嵌入式环境中使用吗？
A: 可以——用 **JIT_INTERP**（运行时不依赖 GCC/TCC）或体积很小的 **TCC** 后端。OpenSSL/MySQL 等 native 依赖可按构建需求裁剪。

### Q: 如何调试生成的 C 代码？
A: 启用 `CompileConfig::debug_mode` 查看日志和 C 代码；使用 `GetLastRecordedCCode()` 导出 C 代码进行分析。

### Q: 支持多线程吗？
A: 每个 `State` 当前为线程本地对象，多线程环境中应为每个线程创建独立的 `State`。
