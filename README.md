# FakeLua
[<img src="https://img.shields.io/github/license/esrrhs/fakelua">](https://github.com/esrrhs/fakelua)
[<img src="https://img.shields.io/github/languages/top/esrrhs/fakelua">](https://github.com/esrrhs/fakelua)
[<img src="https://img.shields.io/github/actions/workflow/status/esrrhs/fakelua/build.yml?branch=master&label=Linux">](https://github.com/esrrhs/fakelua/actions/workflows/build.yml)
[<img src="https://img.shields.io/github/actions/workflow/status/esrrhs/fakelua/build_with_macos.yml?branch=master&label=macOS">](https://github.com/esrrhs/fakelua/actions/workflows/build_with_macos.yml)
[<img src="https://img.shields.io/github/actions/workflow/status/esrrhs/fakelua/build_with_windows.yml?branch=master&label=Windows">](https://github.com/esrrhs/fakelua/actions/workflows/build_with_windows.yml)
[![codecov](https://codecov.io/gh/esrrhs/fakelua/graph/badge.svg?token=9ZCUH1Q632)](https://codecov.io/gh/esrrhs/fakelua)

[中文](README.zh.md) | English

FakeLua is an embeddable Lua-subset runtime for high-performance hosts: it compiles scripts to a bytecode VM and optionally to native code via GCC/TCC JIT, ships a large C++ native standard library (net/http/db/crypto/…), and uses an arena allocator with frame reset so there is no GC pause.

## Design Philosophy & Memory Model

FakeLua was designed to address the **throughput jitter and memory bloat** caused by garbage collection in traditional scripting languages (standard Lua/LuaJIT) when used in **high-performance game servers** or similar real-time systems.

### 1. Scripting as High-Cohesion "Business Glue"
In a typical real-time high-performance server architecture:
*   **State & data reside in C++**: Core data structures (player state, world maps, monster attributes, physics engine) are all stored in efficient, compact, type-safe C++ on the host side.
*   **Stateless/shallow-state Lua logic layer**: Lua is used only for pure logic processing and business orchestration — reading C++ data and invoking C++ functions. Scripts should not retain large-scale data objects long-term.

### 2. Memory: Ultra-Fast Arena Pool + Frame Reset
To support this positioning, FakeLua **does not implement a complex dynamic garbage collector** (tri-color marking, generational GC, etc.). Instead, it uses an extremely efficient **Arena memory pool (Bump Allocator)**:
*   **Bump allocation ($O(1)$)**: When creating temporary variables (Table, String, Multi, etc.), FakeLua simply moves an offset pointer within a pre-allocated contiguous memory block. Allocation is nearly as fast as native stack allocation, without `malloc` fragmentation or overhead.
*   **Instant cleanup ($O(1)$)**: At the end of each frame or request processing, `State::Reset()` is called. It invokes destructors in reverse order, but instead of freeing individual blocks, the pool offset pointer is simply reset to zero. There is no complex object graph traversal, no system-level `free` cost or defragmentation — cleanup is instantaneous.

This design allows FakeLua to fully eliminate GC pause impact on frame rates while maintaining JIT native execution speed, keeping memory overhead at a completely predictable, extremely low level.

### 3. Engineering & Language Philosophy: Deliberate Trade-offs, Not Blind Cloning
FakeLua is not a blind clone of standard Lua's overly permissive dynamic features. Instead, it makes conscious, intentional trade-offs for **execution performance**, **large-scale engineering collaboration**, and **long-term maintainability**:

*   **Strict Function Arity Enforcement**:
    In standard Lua, calling a fixed-parameter function with too few arguments silently pads `nil`, and extra arguments are silently discarded. In large-scale, multi-developer projects, this looseness easily conceals bugs caused by signature changes, omitted arguments, or parameter mismatches. FakeLua deliberately tightens this rule:
    - Direct calls are strictly validated at **compile time**, rejecting mismatched argument counts with a compile error;
    - Dynamic / named calls are strictly intercepted at **runtime**, throwing an exception if argument count differs;
    - Functions legitimately expecting variable arguments must explicitly declare varargs (`...`). Strict arity keeps interfaces explicit, prevents silent runtime nil pollution, and enables the JIT compiler to emit direct, zero-overhead native C calls without redundant runtime nil padding or truncation boilerplate.
*   **No Implicit Globals**: Variables must be explicitly declared with `local`; undeclared assignments trigger compile errors, preventing accidental global pollution from typos.
*   **No Implicit Type Coercion**: Arithmetic operators do not implicitly convert strings to numbers (`"10" + 1` errors), keeping types predictable and enabling aggressive native numeric specialization.
*   **Static Native Dispatch**: Built-in C++ functions are statically dispatched by name at compile time rather than wrapped as first-class closure values, maximizing call performance.

## Core Features

### Three Execution Backends

The same `Call` API can target any registered backend:

- **JIT_GCC**: Invokes system GCC (`-O3`) to generate high-quality native code. Primary backend for production.
- **JIT_TCC**: Embeds TinyCC for extremely fast compilation. Best for development, debugging, and tests (TCC is fetched automatically by CMake).
- **JIT_INTERP**: Compiles to FakeLua bytecode and runs on the built-in interpreter (`src/interp/`). No external C compiler required; useful for portability, tooling, and mixed JIT↔interp closures.

```cpp
int ret = 0;
Call(s, JIT_GCC, "add", ret, 10, 20);    // Production: GCC (-O3)
Call(s, JIT_TCC, "add", ret, 10, 20);    // Dev/test: TCC (fast compile)
Call(s, JIT_INTERP, "add", ret, 10, 20); // Bytecode VM (no host C compiler)
```

### Numeric Specialization

The compiler automatically performs type inference and specialization for function math parameters:

1. [TypeInferencer](src/compile/type_inferencer.h) runs iterative fixed-point inference on each top-level function (leave-one-out) to identify parameters that truly participate in arithmetic (math params).
2. [CGen](src/compile/c_gen.h) generates $2^k$ specializations (`int64_t` / `double` combinations) plus a runtime entry dispatcher that routes to the appropriate specialization based on actual argument types.
3. Specialized bodies use native C types (`int64_t`/`double`) for arithmetic and generate native C `bool` for comparisons, completely eliminating boxing overhead on hot paths.

```lua
-- Example Lua function: recursive Fibonacci
function fib(n)
    if n <= 1 then return n end
    return fib(n - 1) + fib(n - 2)
end
```

Auto-generated specialized C code:

```c
// 1. Numeric specialization: params/return promoted to native int64_t, no boxing
static int64_t fib_spec_0(int64_t n) {
    if (n <= 1) {
        return n;
    }
    return fib_spec_0(n - 1) + fib_spec_0(n - 2);
}

// 2. Generic entry dispatcher: fast type check, zero-overhead routing
static CVar fib_dispatcher(CVar n_var) {
    if (LIKELY(n_var.type_ == VAR_INT)) {
        return (CVar){.type_ = VAR_INT, .data_.i = fib_spec_0(n_var.data_.i)};
    }
    // ... dynamic dispatch to double specialization or generic CVar path
}
```

With recursive Fibonacci (n=32) as an example, the GCC backend is **36.6x** faster than Lua 5.4, and the TCC backend is **11.2x** faster (see [benchmark/README.md](benchmark/README.md) / [中文](benchmark/README.zh.md)).

### Table Struct Specialization

If a Table constructor can statically infer all its keys at compile time (string literals, explicit/implicit integer indices, booleans, floats), the compiler specializes it as a C struct:

1. **Struct layout generation**: The compiler dynamically generates a C struct layout at compile time, with each specialized key mapped to a fixed-offset member.
2. **Initialization & deduplication**: Constructor initialization fills the JIT specialized struct in a single pass (following Lua's left-to-right order) and checks for duplicate keys at compile time.
3. **Ultra-fast pointer-offset access**: For specialized key reads/writes, pointer offset macros (`FL_SPEC`/`FL_SET_SPEC`) are used directly, completely avoiding hash lookups and key comparisons.
4. **Dynamic fallback**: If the key used for read/write is a dynamic variable, it falls back to runtime dynamic dispatch via registered `spec_get` / `spec_set` function pointers.

```lua
-- Example Lua code: defining and accessing Table fields
local point = { x = 10, y = 20 }
point.x = point.x + 5
```

Auto-generated specialized C struct and pointer-offset access:

```c
// 1. Compile-time key layout inference, auto-generate C struct definition
typedef struct Table_Spec_1 {
    CVar x;
    CVar y;
} Table_Spec_1;

// 2. On Table initialization, bind specialized struct layout and spec accessors
SET_TABLE_SPEC(point, Table_Spec_1, spec_get_fn, spec_set_fn, 2);
FL_SET_SPEC(Table_Spec_1, point, x, 0, (CVar){.type_ = VAR_INT, .data_.i = 10});
FL_SET_SPEC(Table_Spec_1, point, y, 1, (CVar){.type_ = VAR_INT, .data_.i = 20});

// 3. Field access converted to ultra-fast pointer member offsets (no hash table lookup)
FL_SPEC(Table_Spec_1, point, x) = NativeAdd(FL_SPEC(Table_Spec_1, point, x), (CVar){.type_ = VAR_INT, .data_.i = 5});
```

## Language Features

### Supported

- **Closures & upvalue capture**: Static AST analysis automatically derives scope and cross-function capture relationships. Captured variables are heap-boxed (`CVar *`), shared across closures in the same scope.
- **Multi-return & varargs**: Functions can `return a, b`; C++ side receives via `std::tie(a, b, c)`. Vararg functions with `...` fully supported.
- **Anonymous & higher-order functions**: `function(args) body end` as values, arbitrary callee calls like `tbl[key]()` or `(fn)()`.
- **Colon method syntax**: `obj:method(args)` sugar with implicit `self` parameter.
- **Generic `for in` iterators**: Stateless iterators, closure generators, and `pairs`/`ipairs` with native C struct-optimized loops.
- **Per-iteration loop variable capture**: Loop variables re-boxed each iteration for independent closure binding.
- **Package modules**: `package "Name"` for namespace isolation, zero-`require` cross-module calls.
- **Complex global initialization**: Arbitrary expressions as file-level variable initializers, executed in generated `__fakelua_init()`.
- **NativeObject & C++ interop**: Host-side object mapping with group arena batch release, C++ member method binding via `RegisterMethod`, colon-syntax calls from Lua.
- **Lua 5.4 pattern matching**: `string.find`/`match`/`gmatch`/`gsub` use a self-contained Lua-pattern engine (`%d`/`%w` classes, custom sets, lazy `-`, captures, frontier `%f[set]`, balanced `%bxy`), fully compatible with standard Lua.
- **String algorithms**: `string.trim`/`trim_left`/`trim_right`/`split`/`join`/`replace`/`starts_with`/`ends_with`/`contains`/`iequals`/`icontains`/`istarts_with`/`iends_with` via Boost.Algorithm.

### Not Supported / Differences from Standard Lua

- **Strict function arity**: Fixed-parameter functions require exact argument-to-parameter count matching. Calling with too few or too many arguments triggers compile-time or runtime errors instead of silently padding nil or discarding extras (use `...` for variadic arguments).
- **Coroutines**: No `coroutine.create`/`resume`/`yield` support.
- **Metatables**: No `__index`, `__newindex`, metamethods, or operator overloading.
- **`require`/`module`**: No standard module system (replaced by `package "Name"` mechanism).
- **`rawequal`/`rawget`/`rawset`/`rawlen`**: Meaningless without metatables.
- **Debug library**: No `debug.*` standard library.
- **Implicit type coercion**: No string→number conversion in arithmetic (`"10" + 1` errors).
- **Implicit globals**: Local variables must be declared with `local`; implicit global declarations are rejected.
- **File-level locals are const bindings**: no whole-slot reassign after init; tables from `__fakelua_init` are deep-const; use NativeObject / `new_global_obj` for mutable cross-frame state.
- **`json.encode` rejects array holes**: integer keys mean array intent, so a table whose keys are positive integers but not contiguous from 1 (`t[1]="a"; t[3]="c"`, or even `{[1001]=...}`) is an error instead of an object with `"1"`/`"3"` keys — the wire shape must not change silently. Keep protocol arrays gap-free (use `""` or `{}` placeholders, not nil), and use string keys (or `container.map`) for id-keyed maps. `#t` still stops at the first nil, like standard Lua; `serialize.encode/decode` keeps non-contiguous keys.
- **Runtime errors carry the Lua source location**: `attempt to perform arithmetic on non-numeric value at main.lua:4` on all three backends, including errors raised inside native functions.

## Built-in Standard Libraries

FakeLua provides 30+ independent C++ native modules under `src/native/` (registered automatically on each `State`), covering math, string, table, IO, networking, timers, events, random, containers, compression, cryptography, serialization, databases, protobuf, config formats, logging, and subprocesses.

> **Full API reference:** [src/native/README.md](src/native/README.md) / [中文](src/native/README.zh.md)

| Category | Modules |
|----------|---------|
| Core Lua | `basic`, `math`, `table`, `string`, `os`, `utf8`, `io`, `random` |
| Runtime / I/O | `runtime` (`runtime.tick()`), `net` (TCP/UDP), `http` (HTTP/1.1), `url`, `timer`, `event` |
| Data | `json`, `csv`, `serialize`, `protobuf`, `container` (Boost.Container deque/vector/list/map/set) |
| Config | `yaml`, `toml`, `xml`, `ini` |
| Database | `mysql` (async + pool), `redis` (async), `sqlite` (synchronous) |
| Crypto / compress | `compress` (LZ4/zlib/gzip/Zstd), `crypto` (OpenSSL digests/ciphers, UUID, CRC-32, xxHash) |
| Process | `process` (`process.run`; does not replace `os.execute`) |
| Logging | `log` (levels, tagged output, file rotation) |
| Object | `object` (NativeObject Lua-side API) |

**Pattern note:** `string.find`/`match`/`gmatch`/`gsub` use **Lua 5.4 patterns** (a self-contained byte-pattern engine in `src/native/string/lua_pattern.*`), not ECMAScript/POSIX regex. See [Lua Pattern Matching](#lua-pattern-matching) below.

### Lua Pattern Matching

`string.find`/`match`/`gmatch`/`gsub` follow PUC-Rio Lua 5.4 semantics exactly, including:

- **Escapes**: `%.` `%(` `%)` `%%` `%+` ... for punctuation; `.` matches any byte.
- **Classes**: `%a %c %d %g %l %p %s %u %w %x` and their uppercase complements; `%z` matches the zero byte.
- **Sets/ranges**: `[set]`, `[^set]`, `[a-z]`, classes inside sets (`[%d_]`), leading `]`/`-` as literals.
- **Quantifiers**: `* + - ?` — including Lua's lazy `-` (`a.-b`).
- **Anchors**: `^` at pattern start, `$` at pattern end.
- **Captures**: nested `(...)`, position captures `()`, back-references `%1`… in patterns and `%0`…`%9` + `%%` in `gsub` replacement strings.
- **Frontier** `%f[set]` and **balanced match** `%bxy`.
- **`gsub`**: string/function/table replacements; a function/table result of `nil`/`false` keeps the original match; `plain=true` on `find` bypasses the pattern engine.
- **Malformed patterns raise an error** (caught by `pcall`) instead of silently returning no match.

```lua
string.match("limit=15", "%d+")                 --> "15"
string.gsub("hello world", "(%w+) (%w+)", "%2 %1")  --> "world hello", 1
string.match("a(b(c)d)e", "%b()")              --> "(b(c)d)"
```

> Lua patterns are *not* regular expressions: there is no alternation (`a|b`), and the escape prefix is `%`, not `\`. Scripts written for the old ECMAScript behavior (e.g. `"\\d+"`, `$1` replacements) must be updated to Lua form (`"%d+"`, `"%1"`).

## Quick Start

### Building

#### System Requirements
- **C++23** compiler (GCC 11+ / Clang 16+ / MSVC 2022+)
- CMake 3.5+
- make or ninja

#### Linux / macOS

```bash
cmake -S . -B build
cmake --build build --parallel
```

> On macOS, first `brew install lua cmake` and add `-DCMAKE_PREFIX_PATH="$(brew --prefix)"` to the cmake command.

Build only core library and CLI tools (no tests/benchmarks):

```bash
cmake --build build --target fakelua flua --parallel
```

Dependencies come through CPM (`CPM_SOURCE_CACHE`, default `~/.cache/CPM`). Boost is a
superbuild whose libraries are git submodules, so a cache entry can hold the repository
but have empty submodule worktrees — configure then reports one missing `Boost::*` target
after another. Configure stops with the repair command when it detects this; it is:

```bash
cd ~/.cache/CPM/boost/<hash>
git submodule update --init --depth 1 --jobs 8
git submodule foreach --recursive 'if [ -e .git ]; then git checkout HEAD -- .; fi'
```

#### Windows (MSYS2 + MinGW)

```bash
cmake -S . -B build -G Ninja
cmake --build build --parallel
ctest --test-dir build -V
```

### Testing & Benchmarks

```bash
cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build --parallel
ctest --test-dir build -V
./build/bin/bench_mark
```

> Unit tests and benchmarks require the Lua development package (header `lua.h` and library files).
> - Linux: `sudo apt-get install liblua5.4-dev` or `liblua5.3-dev`
> - macOS: `brew install lua`
> - Windows MSYS2: `pacman -S mingw-w64-x86_64-lua`

### CLI Tool `flua`

```bash
./build/bin/flua <script.lua | dir/> [--once] [--jit_type=<0|1|2>]
```

Entry points are fixed and always live in package `main` — there are no flags to rename
them, so every app looks the same:

```lua
package "main"

function init() ... end      -- called once at startup
function update() ... end    -- called every frame
function fini() ... end      -- called once before exit
```

- `<script.lua | dir/>`: a single script, or a directory whose `.lua` files are all compiled (sorted by name)
- `--once`: run `init` + `fini` only, without the `update` loop (one-shot script / self-check)
- `--jit_type`: `0`=TCC, `1`=GCC, `2`=INTERP (bytecode VM). Only the backend you pick gets
  compiled: an INTERP run skips the GCC pass (~0.6 s per file) instead of paying for code it
  will never execute.
- `--log_level`: minimum log level, `3`=Warn by default (`0`=Trace … `6`=Off). A long-running
  host does not want the per-file INFO compile lines drowning out real errors; pass
  `--log_level=2` to get them back.
- `--debug`: Enable debug mode (default `false`; when `true`, outputs generated C source / richer diagnostics)

`update` is called back-to-back; a script that wants a fixed cadence paces itself, e.g.
`os.sleep(1000)` at the end of `update` (`os.sleep` takes milliseconds).

Graceful shutdown (both run `fini` first): return a non-zero code from `update`, or call
`flua.set_exit(code)` from Lua. Note that `os.exit` still exits immediately and skips
`fini`. Signals are not handled by flua -- installing a handler for SIGINT / SIGTERM is
the application's own decision.

Each lifecycle phase is a separate top-level Call. That matters because fakelua has no GC:
its frame arena is only reset when a top-level Call starts (reentrant count back to zero),
so a `while true` loop inside a single Lua Call would make the arena grow without bound.
Keep `update` to one frame of work (pump IO/timers, dispatch requests).

### Installation & CMake Integration

#### Install to System

```bash
# Build
cmake -S . -B build
cmake --build build --parallel

# Install (default prefix: /usr/local)
sudo cmake --install build
# or: cd build && sudo make install
```

`flua` is installed as its own component (`cli`) and is marked `OPTIONAL`, so building only
the library no longer makes the install step fail:

```bash
cmake --build build --target fakelua --parallel
sudo cmake --install build            # installs the library, skips the missing flua
sudo cmake --install build --component cli   # or: only the CLI
```

#### Using FakeLua in Other CMake Projects

After installing to your system, other CMake projects can discover and link FakeLua using standard `find_package`:

```cmake
cmake_minimum_required(VERSION 3.20)
project(my_project CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Discover FakeLua package
find_package(fakelua REQUIRED)

add_executable(my_project main.cpp)

# Link against fakelua (automatically sets up include directories and link flags)
target_link_libraries(my_project PRIVATE fakelua::fakelua)
# Alternatively, the unqualified alias is also supported:
# target_link_libraries(my_project PRIVATE fakelua)
```

In your C++ code:

```cpp
#include "fakelua.h"
// or
#include <fakelua/fakelua.h>
```

FakeLua also installs `fakelua.pc` for `pkg-config` consumers.

## Performance Benchmarks

Comparing Lua 5.4, FakeLua TCC, FakeLua GCC across 11 algorithms (Release `-O3` mode):

| Algorithm (typical params) | Lua 5.4 | FakeLua TCC | FakeLua GCC |
|---|---|---|---|
| Fibonacci n=32 | 297.9 ms | 26.7 ms (**11.2x**↑) | 6.8 ms (**36.6x**↑) |
| Sum n=5000000 | 33.9 ms | 18.4 ms (**1.8x**↑) | 1.1 ms (**30.4x**↑) |
| Popcount n=100000 | 18.2 ms | 3.1 ms (**5.9x**↑) | 488.0 μs (**37.3x**↑) |
| BubbleSort n=200 | 1.5 ms | 3.3 ms (0.45x) | 738.8 μs (**1.9x**↑) |
| Sieve n=5000 | 353.4 μs | 1.0 ms (0.34x) | 219.3 μs (**1.8x**↑) |
| FloatPoly n=1000000 | — | — | **34.9x**↑ (浮点特化，GCC 2x 快于 C++) |

> TCC is generally faster than Lua for pure computation; in Table-operation-heavy scenarios, Table struct specialization gives both GCC and TCC a significant boost. Full data available in [benchmark/README.md](benchmark/README.md) / [中文](benchmark/README.zh.md).

## C++ API Reference

### Quick Usage

```cpp
FakeluaStateGuard guard;
State* s = guard.GetState();
CompileFile(s, "script.lua", CompileConfig{.debug_mode = false});

int sum = 0;
Call(s, JIT_GCC, "add", sum, 10, 20); // embed-call a Lua function
```

### State Management

```cpp
// Manual management (not recommended — easy to leak)
State* s = FakeluaNewState(StateConfig{});
// ... use s ...
FakeluaDeleteState(s);

// Or RAII style (recommended)
FakeluaStateGuard guard(StateConfig{});
State* s = guard.GetState();
// ... use s ...
// automatically freed
```

### API Overview

| Function | Description |
|------|------|
| `FakeluaNewState()` | Create FakeLua state |
| `FakeluaDeleteState()` | Free FakeLua state |
| `CompileFile()` | Compile a Lua file |
| `CompileString()` | Compile a Lua code string |
| `Call()` | Invoke a compiled function |
| `GetLastRecordedCCode()` | Get the most recently compiled C code |
| `SetVarInterfaceNewFunc()` | Set custom VarInterface factory |
| `SetDebugLogLevel(s, level)` | Set this State's log level (`LogLevel::Warn` … `LogLevel::Trace`; Lua: `log.set_level`) |

### Type Conversion

Host code should use the public helpers; the `inter::` converters are the interpreter and
JIT internals and hosts are not supposed to depend on them.

```cpp
// Inside a native callback: read arguments (missing/nil argument -> default value)
int64_t id = GetNativeIntArg(s, args, n, 0, 0);
std::string name = GetNativeStringArg(s, args, n, 1, "anon");
bool flag = GetNativeBoolArg(s, args, n, 2, false);
double ratio = GetNativeFloatArg(s, args, n, 3, 1.0);

// Return values
return NativeString(s, name);   // also NativeInt / NativeBool / NativeFloat / NativeNil
```

### Registering a Native Function

Two forms. The callback form takes the raw `CVar` array and declares its arity
(`arg_count` + `is_vararg`):

```cpp
RegisterNativeFunction(s, "greet", 1, false,
    [](State *s, CVar *args, int n) -> CVar {
        return NativeString(s, "hello " + GetNativeStringArg(s, args, n, 0));
    });
```

The typed form wraps a `std::function` whose first parameter is `State *`. The template
parameters are `<Ret, Args...>`, so the signature has to be spelled out inside
`std::function<>` — that is the only form that compiles:

```cpp
RegisterNativeFunction(s, "hp_of", false,
    std::function<int64_t(State *, int64_t)>([](State *, int64_t id) { return id * 2; }));
```

Writing it as `RegisterNativeFunction<std::string(State *, int64_t)>(...)` passes the whole
function type as `Ret`; the header catches that with a `static_assert` instead of the wall
of "function cannot return function type" errors it used to produce.

### Table ↔ Object Mapping

```cpp
class CustomVar : public VarInterface { /* ... */ };
SetVarInterfaceNewFunc(s, []() { return new CustomVar(); });
// Table-type arguments in Call automatically construct CustomVar instances
```

## Architecture Overview

### Compilation Pipeline

```
Lua source
   ↓
[Lexing] → tokens (flexer)
   ↓
[Parsing] → AST (bison + syntax_tree)
   ↓
[File-level stmt check] → reject non-declaration statements (semantic_analysis)
   ↓
[Preprocessing] → normalized AST (preprocessor)
   ↓
[Semantic analysis] → analysis result (semantic_analysis)
   ↓
[Type inference] → type hints (type_inferencer)
   ↓
        ┌─────────────────────────────┬──────────────────────────────┐
        ↓                             ↓                              ↓
[C code generation]            [Bytecode codegen]            (shared AST)
   (c_gen)                        (interp/codegen)
        ↓                             ↓
[JIT TCC / GCC]                [Interpreter VM]
   native code                    (interp/interpreter)
        └───────────── Call(s, JIT_*, …) ─────────────┘
```

### Key Components

| Module | Responsibility |
|------|------|
| `lexer/parser` | Lua lexing and parsing |
| `syntax_tree` | AST representation and traversal |
| `preprocessor` | Lua syntax normalization (e.g., functiondef hoisting) |
| `semantic_analysis` | Semantic and control flow analysis |
| `type_inferencer` | Static type inference and specialization decisions |
| `c_gen` | C code generation and type-driven optimization |
| `interp/*` | Bytecode codegen, opcodes, and interpreter VM |
| `compile_common` | Common type inference and codegen utilities |
| `jit/*` | TCC/GCC backends plus `Vm` function registry |
| `native/*` | Built-in standard libraries (net, http, db, crypto, …) |
| `state` | FakeLua runtime state management |
| `var` | Dynamic value CVar and conversion utilities |

## FAQ

### Q: Why choose a Lua subset over full Lua?
A: Certain dynamic features of full Lua (e.g., metatables) are difficult to compile efficiently. The subset focuses on statically analyzable common patterns, achieving near-C performance through type inference and JIT compilation.

### Q: How to choose among TCC, GCC, and INTERP?
A: **GCC** is the primary production backend (`-O3`). **TCC** compiles extremely fast for development and CI. **INTERP** runs bytecode without a host C compiler — good for constrained environments, tooling, and validating script semantics; hot paths can still call JIT closures when mixed.

### Q: Can it be used in embedded or constrained environments?
A: Yes — use **JIT_INTERP** (no GCC/TCC required at runtime) or the small **TCC** backend. Native modules that need OpenSSL/MySQL/etc. are optional at the dependency level for your build.

### Q: How to debug generated C code?
A: Enable `CompileConfig::debug_mode` to inspect logs and C code; use `GetLastRecordedCCode()` to export C code for analysis.

### Q: Is multithreading supported?
A: Each `State` is currently thread-local; in multithreaded environments, create an independent `State` per thread.
