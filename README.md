# FakeLua
[<img src="https://img.shields.io/github/license/esrrhs/fakelua">](https://github.com/esrrhs/fakelua)
[<img src="https://img.shields.io/github/languages/top/esrrhs/fakelua">](https://github.com/esrrhs/fakelua)
[<img src="https://img.shields.io/github/actions/workflow/status/esrrhs/fakelua/build.yml?branch=master&label=Linux">](https://github.com/esrrhs/fakelua/actions/workflows/build.yml)
[<img src="https://img.shields.io/github/actions/workflow/status/esrrhs/fakelua/build_with_macos.yml?branch=master&label=macOS">](https://github.com/esrrhs/fakelua/actions/workflows/build_with_macos.yml)
[<img src="https://img.shields.io/github/actions/workflow/status/esrrhs/fakelua/build_with_windows.yml?branch=master&label=Windows">](https://github.com/esrrhs/fakelua/actions/workflows/build_with_windows.yml)
[![codecov](https://codecov.io/gh/esrrhs/fakelua/graph/badge.svg?token=9ZCUH1Q632)](https://codecov.io/gh/esrrhs/fakelua)

[中文](README.zh.md) | English

FakeLua is an embeddable Lua-subset compilation engine: it compiles Lua scripts into C code and dynamically compiles them into native machine code via the GCC backend for execution. It provides a C++23 interface with high-performance interop between scripts and native code.

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

## Core Features

### Dual JIT Backends

Supports two JIT modes with a seamless API switch:

- **JIT_GCC**: Invokes system GCC (`-O3`) to generate high-quality native code. This is the primary backend for production use.
- **JIT_TCC**: Embeds TinyCC for extremely fast compilation. Primarily used for development, debugging, and testing (TCC source is automatically fetched during CMake configuration — no system installation needed).

```cpp
int ret = 0;
// Same Call API, switching between JIT_GCC and JIT_TCC on demand
Call(s, JIT_GCC, "add", ret, 10, 20); // Production: GCC backend (-O3 high performance)
Call(s, JIT_TCC, "add", ret, 10, 20); // Development: TCC backend (ultra-fast compilation)
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

With recursive Fibonacci (n=32) as an example, the GCC backend is **36.6x** faster than Lua 5.4, and the TCC backend is **11.2x** faster (see [benchmark/README.md](benchmark/README.md)).

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
- **ECMAScript regex**: `string.find`/`match`/`gmatch`/`gsub` via `std::regex` (supports lookahead, alternation, non-greedy quantifiers — more powerful than Lua patterns).

### Not Supported

- **Coroutines**: No `coroutine.create`/`resume`/`yield` support.
- **Metatables**: No `__index`, `__newindex`, metamethods, or operator overloading.
- **`require`/`module`**: No standard module system (replaced by `package "Name"` mechanism).
- **`rawequal`/`rawget`/`rawset`/`rawlen`**: Meaningless without metatables.
- **Debug library**: No `debug.*` standard library.
- **Implicit type coercion**: No string→number conversion in arithmetic (`"10" + 1` errors).

## Built-in Standard Libraries

FakeLua provides 19 independent C++ native modules under `src/native/`, covering math, string, table, IO, networking, timers, events, compression, encryption, serialization, databases, and protobuf.

> **Full API reference:** [src/native/README.md](src/native/README.md)

| Category | Modules |
|----------|---------|
| Core Lua | `math`, `table`, `string`, `os`, `utf8`, `io` |
| Networking | `net` (TCP server/client), `timer`, `event` |
| Data | `json`, `csv`, `serialize`, `protobuf` |
| Database | `mysql` (async + pool), `sqlite` (synchronous) |
| Crypto | `compress` (LZ4/zlib/gzip/Zstd), `crypto` (MD5/SHA/AES/RC4/Blowfish/DES) |
| Object | `object` (NativeObject Lua-side API) |

**Regex note:** `string.find`/`match`/`gmatch`/`gsub` use **ECMAScript regex** (`std::regex::ECMAScript`), not Lua patterns. See [Regex Guide](#regex-matching-ecmascript-syntax-not-lua-patterns) below for migration tips.

### Regex Matching: ECMAScript Syntax, Not Lua Patterns

| Purpose | Lua Pattern | FakeLua (ECMAScript Regex) |
|---|---|---|
| Digits | `%d` | `\\d` |
| Letters | `%a` | `[A-Za-z]` |
| Alphanumeric | `%w` | `[A-Za-z0-9]` (note `\\w` additionally includes `_`) |
| Whitespace | `%s` | `\\s` |
| Escape literal | `%.`, `%%` | `\\.`、`%` |
| Lazy repeat | `-` (e.g. `.-`) | `?` (e.g. `.*?`) |
| Backreference in replacement | `%1`, `%0` | `$1`, `$&` |

> Since `\d` is not a valid escape in Lua string literals, backslashes in regex patterns must be written as `"\\d+"`. FakeLua does not support `[[...]]` long strings as a workaround.
>
> For scripts that need to be compatible with both standard Lua and FakeLua, use syntax that has the same semantics in both engines, e.g. `[0-9]+` instead of `%d+`.

Key differences:

- **`gsub` replacement strings** use JS-style notation: `$1`…`$9` (capture groups), `$&` (entire match), `` $` `` (text before match), `$'` (text after match), `$$` (literal `$`). Lua's `%1` / `%0` are treated as literal characters here.
- **Invalid patterns don't throw**: `std::regex_error` is caught and returns `nil`, so the script doesn't interrupt.
- **`string.find`'s `plain` parameter** has the same semantics as Lua: passing `true` degrades to pure substring search, completely bypassing the regex engine — also the fastest path.
- **Performance**: The regex path is significantly slower than Lua's native pattern engine; prefer `plain` search or `string.sub` / `string.byte` basic operations on hot paths.

## C++ Embedding API

- `CompileFile` / `CompileString` / `Call`, RAII-style `FakeluaStateGuard`
- `CompileConfig` with `debug_mode`, `skip_jit`, `disable_jit`, `record_c_code` options
- `NativeObject` with `RegisterMethod` for C++ member binding, `new_native_group`/`del_native_group` for arena management
- `SetVarInterfaceNewFunc` for custom table implementations
- `GetLastRecordedCCode()` for debugging generated C code

```cpp
FakeluaStateGuard guard;
State* s = guard.GetState();
CompileFile(s, "script.lua", CompileConfig{.debug_mode = false});

int sum = 0;
Call(s, JIT_GCC, "add", sum, 10, 20); // embed-call a Lua function
```

## Known Limitations

- Function parameter limit: 32 (`kMaxFunctionInputParams`)
- Math specialization parameter limit: 8 (`kMaxMathSpecializedParams`)
- No coroutine, metatable, `require`/`module`, `debug` library
- No implicit string→number conversion in arithmetic
- File-level only allows `package "Name"`, `local`, and function definitions

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
./build/bin/flua <script.lua> --entry=<func> --jit_type=<0|1> --repeat=<N>
```

- `--entry`: Entry function name (default `main`)
- `--jit_type`: `0`=TCC, `1`=GCC
- `--repeat`: Repeat call count (for performance measurement)
- `--debug`: Enable debug mode (default `false`; when `true`, outputs generated C source)

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

> TCC is generally faster than Lua for pure computation; in Table-operation-heavy scenarios, Table struct specialization gives both GCC and TCC a significant boost. Full data available in [benchmark/README.md](benchmark/README.md).

## C++ API Reference

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
| `SetDebugLogLevel()` | Set global debug log level |

### Type Conversion

```cpp
// Native → FakeLua
CVar v_int = inter::NativeToFakelua(s, 42);
CVar v_str = inter::NativeToFakelua(s, std::string("hello"));

// FakeLua → Native
int native_int = inter::FakeluaToNative<int>(v_int);
std::string native_str = inter::FakeluaToNative<std::string>(v_str);
```

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
[C code generation] → C source (c_gen)
   ↓
[JIT compilation] → machine code (tcc_jit / gcc_jit)
   ↓
[Load & execute] → result
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
| `compile_common` | Common type inference and codegen utilities |
| `jit/*` | TCC and GCC backend integration |
| `state` | FakeLua runtime state management |
| `var` | Dynamic value CVar and conversion utilities |

## FAQ

### Q: Why choose a Lua subset over full Lua?
A: Certain dynamic features of full Lua (e.g., metatables) are difficult to compile efficiently. The subset focuses on statically analyzable common patterns, achieving near-C performance through type inference and JIT compilation.

### Q: How to choose between TCC and GCC backends?
A: **GCC** is the primary backend for production (with `-O3` optimization); **TCC** compiles extremely fast, primarily for development and testing.

### Q: Can it be used in embedded or constrained environments?
A: Yes — the TCC backend is small and fast. The core library has minimal dependencies (C++ standard library only) and is cross-compilable.

### Q: How to debug generated C code?
A: Enable `CompileConfig::debug_mode` to inspect logs and C code; use `GetLastRecordedCCode()` to export C code for analysis.

### Q: Is multithreading supported?
A: Each `State` is currently thread-local; in multithreaded environments, create an independent `State` per thread.
