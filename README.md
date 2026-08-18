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

```cpp
int ret = 0;
// 同一套 Call API，可按需指定 JIT_GCC 或 JIT_TCC 后端
Call(s, JIT_GCC, "add", ret, 10, 20); // 生产环境推荐：GCC 后端 (-O3 高性能)
Call(s, JIT_TCC, "add", ret, 10, 20); // 开发调试推荐：TCC 后端 (极速编译)
```

### 数值参数特化（Numeric Specialization）

编译器对函数的数学参数自动做类型推断与特化：

1. [TypeInferencer](file:///home/project/fakelua/src/compile/type_inferencer.h) 对每个顶层函数运行迭代不动点推断（leave-one-out），识别出真正参与算术运算的参数（math params）。
2. [CGen](file:///home/project/fakelua/src/compile/c_gen.h) 为每个含数学参数的函数生成 `2^k` 个特化版本（`int64_t` / `double` 组合），以及一个运行时入口分发器，根据实际参数类型路由到对应特化体。
3. 特化体内的算术运算直接用原生 C 类型（`int64_t`/`double`）计算，比较表达式也生成原生 C `bool` 而非走 [CVar](file:///home/project/fakelua/include/fakelua.h#L198) 装箱路径，彻底消除热路径上的类型判断开销。

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

以递归 Fibonacci（n=32）为例，GCC 后端比 Lua 5.4 快 **43.5x**，TCC 后端快 **11.2x**（详见 [benchmark/README.md](benchmark/README.md)）。

### Table 结构体特化（Table Specialization）

如果 Table 构造函数在编译期可以静态推断出其所有 Key（如字符串字面量、显式/隐式整型索引、布尔型、浮点型），编译器会将其特化为 C 语言结构体：

1. **结构体布局生成**：编译器在编译期动态为该 Table 生成对应的 C 结构体布局，各个特化 Key 映射为结构体中固定偏移的成员变量。
2. **初始化与去重**：构造函数初始化时，会采用单次遍历在 JIT 特化结构体内进行填充（遵循 Lua 左到右的语法顺序），并进行静态 Key 重复性的检查。如果检测到重复 Key 初始化操作，编译期会抛出异常。
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

```cpp
// 原生 C++ 类型与 JIT ABI 安全载体 CVar 的相互转换
CVar v_int = inter::NativeToFakelua(s, 42);
int native_int = inter::FakeluaToNative<int>(s, v_int);
```

### [VarInterface](file:///home/project/fakelua/include/fakelua.h#L17)：可扩展的复杂类型桥接

[VarInterface](file:///home/project/fakelua/include/fakelua.h#L17) 是 Lua table 等复杂类型与宿主之间的抽象接口，宿主可按需实现自己的版本接入原有对象系统。库内附带 [SimpleVarImpl](file:///home/project/fakelua/include/fakelua.h#L61) 开箱即用。

```cpp
class CustomVar : public VarInterface { /* 继承并扩展自定义表实现 */ };

// 注册工厂函数，使 FakeLua 创建的 Table 自动构造为 CustomVar
SetVarInterfaceNewFunc(s, []() { return new CustomVar(); });
```

### [NativeObject](file:///home/project/fakelua/include/fakelua.h#L580)：原生对象桥接与组内存池（Group Arena）

提供高性能宿主 C++ 原生对象映射能力：

- **精简宿主公共 API**：在 SDK 头文件中屏蔽底层存储细节（Pimpl），只暴露必要的属性存取（`GetInt`/`SetInt`/`GetFloat`/`SetObject` 等）与迭代方法。
- **组粒度批次释放（Group Allocation & Arena Destroy）**：禁止单个手动申请或卸载对象。所有 `NativeObject` 均强制在指定的 `group_id` 组池内创建（`NativeObjectManager::Instance().Create(group_id, ...)`），在处理请求或逻辑帧完成后通过 `DestroyGroup(group_id)` 批量一次性释放，与 FakeLua 无 GC、Arena 极速重置的设计哲学高度保持一致。
- **C++ 原生成员回调方法绑定 (Member Method Binding)**：支持直接在 `NativeObject` 上通过 `RegisterMethod` 绑定 C++ 函数/Lambda 方法。在 Lua 中可直接使用冒号语法 `obj:method(args...)` 随时调用 C++ 宿主方法。
- **C++ 函数自动装箱转换**：C++ 侧注册的 Native 回调可以直接返回 `NativeObject*` 指针，`fakelua.h` 的 `inter::NativeToFakelua` 会自动安全打包并转换为 Lua 可识别的装箱对象。

#### C++ 成员回调绑定与 Lua 交互示例

```cpp
// 1. C++ 宿主侧：在 NativeObject 实例上注册原生成员方法
player->RegisterMethod("take_damage", [](NativeObject *self, State *s, CVar *args, int n) -> CVar {
    int64_t dmg = inter::FakeluaToNative<int64_t>(s, inter::GetNativeArg(s, args, n, 0));
    self->SetInt("hp", self->GetInt("hp") - dmg);
    return inter::NativeToFakeluaNil(s);
});

player->RegisterMethod("is_alive", [](NativeObject *self, State *s, CVar *args, int n) -> CVar {
    return inter::NativeToFakeluaBool(s, self->GetInt("hp") > 0);
});
```

```lua
-- 2. Lua 侧：使用冒号语法轻松调用绑定的 C++ 成员方法
player:take_damage(30) -- 执行 C++ 回调，扣减 hp

if player:is_alive() then
    print("Player is still alive, current HP:", player.hp)
end
```

### Package 包管理机制（Package & Zero-Require Modules）

FakeLua 提供特有的 `package "ModuleName"` 模块化隔离与零 `require` 跨模块互调能力：

- **模块包定义**：在脚本顶部通过 `package "PackageName"` 声明模块归属。当前文件定义的顶层导出函数会自动绑定并挂载到该包的命名空间（如 `Player.AddItem`）下。
- **跨模块零 `require` 直接互调**：无需显式调用 `require` 加载依赖文件。只要相关包已被编译加载到同一个 `State` 中，跨模块调用（如 `Player.AddItem(...)`）会自动通过动态路由寻址绑定。

#### 模块化代码示例

```lua
-- player.lua
package "Player"

local BASE_BONUS = 1 -- 包内私有变量

function AddItem(id, num) -- 导出为 Player.AddItem
    return id + num + BASE_BONUS
end
```

```lua
-- bag.lua
package "Bag"

function UseItem(id) -- 导出为 Bag.UseItem
    -- 零 require 直接跨模块调用 Player 包的函数
    return Player.AddItem(id, 10)
end
```

```lua
-- main.lua
function test()
    local res1 = Player.AddItem(100, 5) -- 106
    local res2 = Bag.UseItem(200)       -- 211
    return res1 + res2                  -- 317
end
```

### 多返回值与可变参数（Multi-Return & Varargs）

- **多返回值**：函数可以通过 `return a, b` 返回多个值，在赋值或返回语句中正确解包。
- **参数动态展开**：在函数调用或 Table 构造中，若最后一项是多返回值函数调用，其返回值会自动展开。
- **可变参数（`...`）**：支持声明和调用 vararg 函数，C++ 侧调用时多余参数自动打包为 Multi，无需手动组装。
- **C++ 返回值自动解包**：通过 `std::tie(a, b, c)` 接收多返回值，模板自动将 Multi CVar 拆解为各变量。

```lua
-- Lua 侧：定义支持可变参数与多返回值的函数
function calc_multi(a, ...)
    return a, a * 2, "ok"
end
```

```cpp
// C++ 侧：传入变长参数并通过 std::tie 自动解包多返回值
int x = 0, y = 0;
std::string msg;
Call(s, JIT_GCC, "calc_multi", std::tie(x, y, msg), 10, 20, 30); // x=10, y=20, msg="ok"
```

### 标准内置扩展库（Built-in Standard Libraries）

FakeLua 提供完整的核心标准库（`math`、`table`、`string`、`os`、`utf8`、`io`、`net`），完全按照独立 C++ 模块解耦设计（`native_math` / `native_table` / `native_string` / `native_os` / `native_utf8` / `native_io` / `native_net`），既支持在 Lua 脚本中直接使用，也支持由 CGen 编译器生成的 C 代码进行 Fast-path 直连调用：

- **Basic 全局函数**：
  - **类型与转换**：`type`、`tostring`、`tonumber`
  - **输入输出**：`print`、`select`
  - **错误处理**：`error`、`assert`、`pcall`、`xpcall`
  - **表迭代**：`next`、`pairs`、`ipairs`
  - **文件加载**：`loadfile`、`dofile`（加载文件并编译，顶层函数注册为全局）
  - **垃圾回收**：`collectgarbage([opt])`（仅支持 `"count"` 返回内存 KB，其他选项为 no-op）
  - **版本常量**：`_VERSION`（返回 `"Fakelua 5.3"`）
- **Math 数学库 (`math.*`)**：
  - **基础与三角函数**：`math.abs`, `math.floor`, `math.ceil`, `math.min`, `math.max`, `math.sqrt`, `math.sin`, `math.cos`, `math.tan`, `math.asin`, `math.acos`, `math.atan`, `math.sinh`, `math.cosh`, `math.tanh`
  - **指数、对数与分解**：`math.exp`, `math.log`, `math.log10`, `math.deg`, `math.rad`, `math.modf`, `math.frexp`, `math.atan2`, `math.copysign`
  - **随机数与数值常量**：`math.random`, `math.randomseed`, 以及数值常量 `math.pi`, `math.huge`, `math.maxinteger`, `math.mininteger`
- **Table 表操作库 (`table.*`)**：
  - **数组操作**：`table.insert(list [, pos], value)`、`table.remove(list [, pos])`、`table.concat(list [, sep [, i [, j]]])`、`table.sort(list [, comp])`
  - **打包与解包**：`table.pack(...)`、`table.unpack(list [, i [, j]])`
  - **预分配构造**：`table.create(seq_size [, hash_size])`
- **String 字符串处理库 (`string.*`)**：
  - **基础操作**：`string.len`、`string.sub`、`string.rep`、`string.reverse`、`string.lower`、`string.upper`
  - **编码转换**：`string.byte`、`string.char`、`string.charpattern`
  - **格式化**：`string.format`（支持 `%s` `%d` `%i` `%u` `%x` `%X` `%o` `%f` `%e` `%E` `%g` `%G` `%c` `%q` `%p`）
  - **正则匹配**：`string.find`、`string.match`、`string.gmatch`、`string.gsub`（⚠️ **采用 ECMAScript 正则语法，而非 Lua pattern**，详见下方[正则匹配](#正则匹配采用-ecmascript-语法而非-lua-pattern)一节）
  - **序列化与加载**：`string.pack`、`string.packsize`、`string.unpack`、`string.dump`、`load`、`loadstring`、`loadfile`（直接编译文件，顶层函数注册为全局，无需调用闭包）
- **OS 系统库 (`os.*`)**：
  - **时间日期**：`os.clock()`、`os.date([format[, time]]])`（支持 `"*t"` 返回时间表 `{year=, month=, day=, hour=, min=, sec=, wday=, yday=, isdst=}`）、`os.difftime(t2, t1)`、`os.time([table])`
  - **环境执行**：`os.execute([command])`（返回 `(bool|nil, "exit"|"signal"|"error", code)` 三元组）、`os.exit([code[, close]])`、`os.getenv(varname)`
  - **文件操作**：`os.remove(filename)`、`os.rename(oldname, newname)`、`os.tmpname()`
  - **区域设置**：`os.setlocale(locale[, category])`
- **UTF-8 编码库 (`utf8.*`)**：
  - **编解码**：`utf8.char(...)`、`utf8.codepoint(s [, i [, j]])`、`utf8.codes(s)`
  - **长度与偏移**：`utf8.len(s [, i [, j]])`、`utf8.offset(s, n [, i])`
  - **模式常量**：`utf8.charpattern`
- **IO 文件库 (`io.*`)**：
  - **文件打开/关闭**：`io.open(filename [, mode])`、`io.close([file])`、`io.tmpfile()`、`io.popen(command [, mode])`（管道执行外部命令）
  - **读写操作**：`io.read([format ...])`（支持多格式参数，返回多值）、`io.write(...)`、`io.flush()`
  - **文件定位**：`file:seek([whence [, offset]])`、`file:setvbuf(mode [, size])`（成功返回 file，失败返回 nil+errmsg）
  - **类型检查**：`io.type(v)`
  - **标准流**：`io.stdin`、`io.stdout`、`io.stderr`
  - **文件方法**：`file:read([format])`、`file:write(...)`、`file:flush()`、`file:close()`、`file:seek(...)`、`file:setvbuf(...)`、`file:lines()`（逐行迭代器，用于 `for line in file:lines() do ... end`）
- **Net 网络库 (`net.*`)**：
  - **服务端与客户端创建**：`net.server(config)`、`net.client(config)`（支持 `port`, `maxconn`, `backlog`, `nonblocking`, `nodelay`, `keepalive` 等基础配置）
  - **多种常用封包/解包协议**：通过 `framer` 配置无缝切换：
    - `"header4"` / `"header4_be"`（默认）：4 字节大端整数长度头
    - `"header4_le"`：4 字节小端整数长度头
    - `"header2"` / `"header2_be"`：2 字节大端整数长度头
    - `"header2_le"`：2 字节小端整数长度头
    - `"line"`：换行符（`\n` 或 `\r\n`）分隔，自动解包并去除换行符
    - `"fixed"`：固定包长协议（配合 `fixed_len = N` 配置）
    - `"raw"`：原始流透传模式
  - **自定义解包算法（Custom Parser）**：
    - **Lua 自定义解包**：传入 `parser = "Package.my_parser"`，接收缓冲区字符串，返回 `(payload, consumed_bytes)` 或 `nil`（数据不足）
    - **C++ 自定义解包**：在 C++ `NetConfig` 中设置 `custom_parser_fn` 与 `custom_encoder_fn`
  - **事件派发与驱动**：`server:dispatch("Package.on_event")` / `client:dispatch(...)`（注册统一纯函数事件回调入口）、`server:tick()` / `client:tick()`（驱动非阻塞 I/O 与事件分发）
  - **数据发送与连接管理**：`server:send(connid, data)`、`client:send(data)`、`server:close()`、`client:close()`
  - **状态与统计读取**：`obj:get_conn_count()`、`obj:get_recv_count()`、`obj:get_last_data()`、`server:get_connid()`、`obj:get_events()`
- **Timer 定时器库 (`timer.*`)**：
  - **一次性定时器**：`timer.set(delay_ms, "Package.callback")`（注册定时器并返回 `timer_id`，到期时按函数名派发回调，回调签名为 `function cb(type, timer_id)`，其中 `type == "timer"`）、`timer.del(timer_id)`（删除未触发的定时器）
  - **驱动定时器**：`timer.tick()`（在主循环中调用，触发所有到期定时器与心跳）
  - **周期性心跳**：`timer.set_heartbeat(interval_ms, "Package.heartbeat_cb")`（注册全局心跳，到期后自动重新调度，永不自动删除；重复调用覆盖之前的心跳）
  - **状态读写**：`timer.set_result(key, val)` / `timer.get_result(key)`（读写全局 NativeObject，供回调记录状态、测试在 main 读取验证）

```lua
-- 示例：使用标准库完成排序、格式化与数学计算
local scores = { 85, 92, 78, 95 }
table.sort(scores, function(a, b) return a > b end) -- 降序排序

local top_student = string.format("Top score: %d, Angle Rad: %.2f", scores[1], math.rad(180))
local info = table.concat(scores, ", ")
-- top_student => "Top score: 95, Angle Rad: 3.14"
-- info        => "95, 92, 85, 78"
```

#### 正则匹配：采用 ECMAScript 语法，而非 Lua pattern

`string.find` / `string.match` / `string.gmatch` / `string.gsub` 底层由 `std::regex`（`std::regex::ECMAScript`）实现，**不是** Lua 原生的 pattern 引擎。从标准 Lua 迁移脚本时，模式串必须改写：

| 用途 | Lua pattern | FakeLua（ECMAScript 正则） |
|---|---|---|
| 数字 | `%d` | `\\d` |
| 字母 | `%a` | `[A-Za-z]` |
| 字母或数字 | `%w` | `[A-Za-z0-9]`（注意 `\\w` 额外包含 `_`） |
| 空白 | `%s` | `\\s` |
| 转义字面量 | `%.`、`%%` | `\\.`、`%` |
| 惰性重复 | `-`（如 `.-`） | `?`（如 `.*?`） |
| 替换串中引用捕获 | `%1`、`%0` | `$1`、`$&` |

> 由于 Lua 字符串字面量中 `\d` 不是合法转义，正则里的反斜杠需要写成 `"\\d+"`。FakeLua 不支持 `[[...]]` 长字符串，无法用它来规避转义。
>
> 若脚本需要同时兼容标准 Lua 与 FakeLua，可改用两种引擎语义相同的写法，例如用 `[0-9]+` 代替 `%d+`、用 `[A-Za-z]+` 代替 `%a+`——方括号字符集、`+`、`*`、`()` 捕获在两边含义一致。

主要差异：

- **`gsub` 的替换串**使用 JS 风格记法：`$1`…`$9`（捕获组）、`$&`（整个匹配）、`` $` ``（匹配前的文本）、`$'`（匹配后的文本）、`$$`（字面 `$`）。Lua 的 `%1` / `%0` 在这里只会被当作普通字符。
- **可以使用 Lua pattern 没有的能力**：交替 `|`、非贪婪量词 `*?` `+?`、区间重复 `{n,m}`、前瞻断言 `(?=...)` 等 ECMAScript 特性均开箱可用。
- **不支持 Lua 特有语法**：`%b()`（括号平衡匹配）、`%f[set]`（frontier pattern）以及所有 `%` 字符类。
- **非法模式串不抛异常**：`std::regex_error` 被捕获后统一返回 `nil`，脚本不会中断，因此模式写错时表现为「永远匹配不到」而非报错。
- **`string.find` 的 `plain` 参数**语义与 Lua 一致：传 `true` 时退化为纯子串查找，完全绕过正则引擎，也是最快的路径。
- **性能**：正则路径明显慢于 Lua 原生 pattern 引擎（见 [benchmark/README.md](benchmark/README.md)），热路径上建议优先用 `plain` 查找或 `string.sub` / `string.byte` 等基础操作。

```lua
-- Lua pattern 写法（在 FakeLua 中匹配不到，会返回 nil）
local n1 = string.match("abc123", "%d+")      -- nil

-- FakeLua 的 ECMAScript 正则写法
local n2 = string.match("abc123", "\\d+")     -- "123"

-- gsub 的捕获引用用 $1 而不是 %1
local s = string.gsub("hello world", "([a-z]+) ([a-z]+)", "$2 $1")  -- "world hello"
```

### C++ 嵌入 API

- `CompileFile` / `CompileString` / `Call`，RAII 风格 `FakeluaStateGuard`
- 支持基本类型、对象、以及自定义 VarInterface 实现的高级映射
- 支持记录编译生成的 C 代码用于调试和性能分析（`CompileConfig::record_c_code`）

```cpp
FakeluaStateGuard guard;
State* s = guard.GetState();
CompileFile(s, "script.lua", CompileConfig{.debug_mode = false});

int sum = 0;
Call(s, JIT_GCC, "add", sum, 10, 20); // 嵌入调用 Lua 函数
```

### 闭包与 Upvalue 捕获（Closures & Upvalue Capture）

FakeLua 完整支持 Lua 闭包与 Upvalue 捕获：

- **静态 Upvalue 分析**：[ResolveScopes](file:///home/project/fakelua/src/compile/c_gen.cpp) 静态 AST 分析 Pass 自动推导所有局部变量、参数及循环变量的作用域与跨函数捕获关系。
- **Heap Boxing 共享机制**：被捕获的变量在定义时自动提升为堆分配的 `CVar *` 盒子，多闭包共享同一个堆内存指针，天然实现同作用域下多闭包同步修改共享 Upvalue。
- **匿名函数与高阶函数**：支持匿名函数表达式 `function(args) body end` 作为值传递（高阶函数如 `map`），以及任意 Callee 调用（如 `tbl[key]()` 或 `(fn)()` 链式调用）。
- **冒号方法调用**：完整支持 `obj:method(args)` 冒号方法调用语法糖，在 JIT 代码生成中自动将调用者求值并隐式将 `obj` 作为首个 `self` 参数传递给目标闭包。
- **通用 `for in` 泛型迭代器**：完整支持 Lua 泛型 `for var1, ..., varn in explist do` 迭代器（既保留了 `pairs`/`ipairs` 的原生 C 语言结构体极速表循环，又全面支持无状态迭代器和闭包生成器等自定义迭代函数）。
- **循环变量独立捕获**：在 `for` 及 `for in` 循环中，每次迭代自动重新 boxing 循环变量，确保迭代内部创建的闭包绑定独立变量副本。

```lua
-- 高阶函数与闭包计数器示例
function make_counter(start)
    local count = start
    return function()
        count = count + 1
        return count
    end
end

local counter = make_counter(10)
print(counter()) -- 11
print(counter()) -- 12
```

### 全局变量复杂初始化

支持任意复杂表达式作为全局/文件级变量的初始化器：

```lua
local x = math.floor(3.14) + 1
local y = x * 2 - 1
local z = (x + y) / 2.0
```

编译器会将复杂初始化器提取到生成的 `__fakelua_init()` 函数中，在 JIT 加载后立即执行，使全局变量获得正确的运行时值。

### 文件级语句限制

文件级（chunk 顶层）只允许三类语句：可选的首行 `package "Name"` 声明、`local` 变量定义、函数定义。
`if` / `while` / `for` / 赋值等可执行语句必须写在函数体内，否则会在 [semantic_analysis](file:///home/project/fakelua/src/compile/semantic_analysis.h) 的
`CheckFileLevelStmts` 阶段（预处理改写 AST 之前）直接报错：

```lua
local x = 5
if x > 3 then -- 编译错误：unsupported file-level statement If
    x = 10
end
```

文件级 `local` 被视为该文件的常量，会降级成 C 的 `static const`，因此也不允许在文件级对它再次赋值。

## 当前已知限制

### 类型系统限制
- 类型推导基于静态分析，复杂的动态类型操作无法优化
- 函数 specialization 基于调用点的 math 参数发现
- 函数参数上限 32 个（通过常量 `kMaxFunctionInputParams` 统一配置）
- 数学特化参数上限 8 个（通过常量 [kMaxMathSpecializedParams](file:///home/project/fakelua/include/fakelua.h#L14) 统一配置，超过此限制的数学参数不进行特化，作为普通动态参数处理）

### 语言特性缺失
- 不支持协程（coroutine）
- 不支持元表（metatable）
- 不支持 `require` / `module` 模块系统（注：fakelua 有独立的 `package "Name"` 模块化机制）
- 不支持 `rawequal` / `rawget` / `rawset` / `rawlen`（因无元表，这些函数无意义）
- 不支持 debug 标准库

### 标准库语义差异
- `string.find` / `match` / `gmatch` / `gsub` 使用 **ECMAScript 正则**而非 Lua pattern：`%d`、`%b()`、`%f[]` 等 Lua 特有语法不可用，`gsub` 替换串需用 `$1` 而非 `%1`（详见[正则匹配](#正则匹配采用-ecmascript-语法而非-lua-pattern)）
- 算术不做字符串→数字的隐式转换：`"10" + 1` 在 Lua 里能算，在 FakeLua 里会报错

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
| Fibonacci n=32 | 297.9 ms | 26.7 ms（**11.2x**↑） | 6.8 ms（**43.5x**↑） |
| Sum n=5000000 | 33.9 ms | 18.4 ms（**1.8x**↑） | 2.0 ms（**17.1x**↑） |
| Popcount n=100000 | 18.2 ms | 3.1 ms（**5.9x**↑） | 974.0 μs（**18.7x**↑） |
| BubbleSort n=200 | 1.5 ms | 3.3 ms（0.45x） | 738.8 μs（**2.0x**↑） |
| Sieve n=5000 | 353.4 μs | 1.0 ms（0.34x） | 219.3 μs（**1.6x**↑） |

> TCC 纯计算类场景普遍快于 Lua；在包含 Table 操作的场景下，由于引入了 Table 结构体特化，GCC 与 TCC 的 Table 读写性能均得到了大幅度的优化提升。表标准库（`table.insert`/`remove`/`sort`）GCC 后端也快于 Lua 3.6~4.2x。完整数据见 [benchmark/README.md](benchmark/README.md)。

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
