# FakeLua Project Overview

FakeLua is a subset of the Lua language that implements Just-In-Time (JIT) compilation to native machine code using `libgccjit`. It aims for a streamlined language structure with improved performance by compiling Lua 5.4 syntax directly into native code at runtime.

## Core Technologies
- **Language:** C++23
- **Lexer/Parser:** Flex and Bison
- **JIT Backend:** GCC JIT (`libgccjit`)
- **Build System:** CMake
- **Testing:** GoogleTest (GTest)

## Architecture
1.  **Lexing & Parsing (`src/compile/flex`, `src/compile/bison`):** Lua source code is tokenized by Flex and parsed by Bison into a syntax tree (`syntax_tree.h`).
2.  **Compilation (`src/compile/Compiler.cpp`):** The `Compiler` class traverses the syntax tree and uses `gcc_jit` to generate machine code.
3.  **Virtual Machine (`src/jit/Vm.cpp`):** The `Vm` manages compiled functions and provides the environment for execution.
4.  **State Management (`src/state/state.cpp`):** `FakeluaState` represents the execution environment, similar to `lua_State`.
5.  **Variable System (`src/var/`):** Implements Lua's dynamic types (`nil`, `bool`, `int`, `float`, `string`, `table`) using a `var` class and a `cvar` struct for JIT interfacing.

## Key Differences from Lua
- No global variables (only global constants).
- No Garbage Collection (uses a memory pool instead).
- No closures, coroutines, threads, or metatables.
- Tables are exclusively hash tables.

## Building and Running

### Prerequisites
- **Linux:** `gcc-13` (with JIT support), `cmake`, `flex`, `bison`, `lua` (for testing).
- **Windows (MinGW):** `mingw-w64-x86_64-libgccjit`, `mingw-w64-x86_64-lua`, `flex`, `bison`.

### Build Commands
```powershell
mkdir build
cd build
cmake ..
cmake --build .
```

### Running Tests
After building, you can run the unit tests:
```powershell
# From the build directory
./test/unit_tests
# Or using ctest
ctest
```

## Development Conventions

### Coding Style
- **Namespace:** All core functionality resides in the `fakelua` namespace.
- **Modern C++:** Uses C++23 features like `std::format`, `std::ranges`, and `std::source_location`.
- **Naming:** Follows standard C++ conventions (PascalCase (UpperCamelCase) for classes, structs, enums, and functions).
- **Enum Conversion:** For enum-to-string conversion, avoid using external libraries like `magic_enum`. Use custom helper functions with the suffix `_to_string` (e.g., `VarTypeToString`, `SyntaxTreeTypeToString`) defined alongside the enum to avoid naming conflicts with methods like `var::ToString()`.
- **Error Handling:** Uses a custom exception system (`src/util/exception.h`).
- **Logging:** Use `LOG_INFO` and `LOG_ERROR` macros defined in `src/util/logging.h`.

### Testing Practice
- Tests are located in the `test/` directory.
- New features or bug fixes should include a corresponding Lua script in `test/lua/` and a C++ test case in one of the `test/*.cpp` files.
- Use `EXPECT_EXIT` or other GTest macros to verify behavior and exceptions.

### JIT Implementation
- Helper functions that are called from JIT-compiled code must be declared with `extern "C" __attribute__((used))` in `src/jit/vm.h` to ensure they are visible and not optimized away.

## High-Performance C Generation Standards

### 1. JIT Execution Environment & External Interfaces (Extern "C")
Generated C code is designed to be highly autonomous. Any JIT-compiled C code must link to fixed `extern "C"` interfaces with strict `PascalCase` naming:
- `void* FakeluaAllocTemp(State *s, size_t size)`: Used for Table expansion or dynamic memory allocation.
- `void FakeluaThrowError(State *s, const char *msg)`: Used for handling runtime exceptions (e.g., Nil index).

### 2. Performance Optimization Core
- **Macro Expressions**: All arithmetic operations (`OpAdd`, `OpSub`, etc.) and logical checks (`IsTrue`, `VarEqual`, etc.) must be implemented using **GCC Statement Expressions `({ ... })`** to eliminate function call overhead while maintaining expression nesting capabilities.
- **Manual Loop Unrolling**: Performance-critical paths, such as `VarTable`'s `quick_data_` slot traversal, must be manually unrolled to optimize branch prediction and eliminate loop control overhead.

### 3. Cross-Language Semantic Synchronization
- **Hash Consistency**: Both C++ (`VarString::Hash`) and C (`FlHashString`) must use the **DJB2 algorithm**. The `hash_` field must be initialized to `0` to support lazy evaluation semantics across both environments.
- **Equality Semantics**: `VarEqual` (C) and `Var::Equal` (C++) must maintain 100% semantic parity, specifically supporting cross-type comparison between `VAR_STRING` and `VAR_STRINGID`, and explicitly handling `NaN == NaN` as `true`.
- **Float Normalization**: Both the `SET_FLOAT` macro (C) and `Var::SetFloat` (C++) must include checks for `NaN` and `Infinity`. These special values MUST NOT be normalized to `VAR_INT`. Normalization to `VAR_INT` only occurs when `(double)(int64_t)val == val` is true for finite numbers.

### 4. Preprocessing & Assignment Standards
- **Evaluation Order**: Multi-assignments (e.g., `a, b = b, a`) MUST evaluate all RHS expressions before performing any assignments. The `PreProcessor` achieves this by generating temporary local variables to hold RHS results.
- **Unique Temporaries**: Temporary variables generated during preprocessing must use a naming convention that avoids collisions (e.g., `__fakelua_tmp_LINE_INDEX`).

### 5. Generated Code Style
- **Brace Requirement**: All generated C code must explicitly include braces `{}` for all control flow statements (`if`, `for`, `while`), ensuring maximum robustness and compatibility across various C compilers.

