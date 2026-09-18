[中文](README.zh.md) | English

# Benchmark Results

This file records results of running `bench_mark` compiled in **Release mode** (`-O3 -DNDEBUG`) locally. It covers **51 Lua performance scenarios across 6 categories**, each with C++ / Lua 5.4 / FakeLua TCC / FakeLua GCC / FakeLua INTERP (bytecode interpreter). Analysis below splits into **interpreter vs Lua 5.4** (this machine, 2026-09-16) and **GCC JIT vs Lua / C++** (previous machine, 2026-08-14).

## Running

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DFAKELUA_BUILD_BENCHMARKS=ON
cmake --build build --target bench_mark --parallel
# Lua 5.4 vs bytecode interpreter only:
build/bin/bench_mark --benchmark_filter='BM_Lua_|BM_FakeLua_.*_INTERP' \
  --benchmark_repetitions=1 --benchmark_report_aggregates_only=true
# Full suite (C++ / Lua / TCC / GCC / INTERP):
build/bin/bench_mark --benchmark_repetitions=1 --benchmark_report_aggregates_only=true
```

---

## Interpreter vs Lua 5.4 (2026-09-16)

Bytecode VM (`JIT_INTERP`): 8-byte `Inst`, GNU computed-goto dispatch, Lua 5.4-style integer `FORPREP`/`FORLOOP` countdown, dest-driven `ADD`. Production path remains **JIT_GCC**, not the interpreter.

### Environment (INTERP run)

- Date: 2026-09-16
- Machine: AMD EPYC 7551 32-Core Processor, 16 X 2000 MHz CPU s (KVM)
- CPU Caches: L1d 64 KiB (x16), L1i 64 KiB (x16), L2 512 KiB (x16), L3 65536 KiB (x2)
- Build mode: **Release** (`-O3 -DNDEBUG`), GCC 16.2.0
- Lua: 5.4.7
- FakeLua INTERP: `debug_mode=false` (scripts still compiled for TCC/GCC at init; timed path is `Call(..., JIT_INTERP, ...)`)
- Binary: `build/bin/bench_mark`

> Absolute runtimes are highly dependent on machine and compiler version and are not comparable across environments. **INTERP vs Lua** = Lua CPU time / INTERP CPU time (>1 means the interpreter is faster). The **slower by** column is INTERP / Lua (how many times Lua's time the interpreter takes). Only ratios inside this section are meaningful; do not mix them with the 2026-08-14 GCC tables.

Largest parameter per scenario:

### Algorithms (algo)

| Scenario | Param | INTERP vs Lua | INTERP / Lua time | Notes |
|----------|-------|---------------|-------------------|-------|
| Fibonacci | n=32 | 0.13x | **7.6x** | Recursive `CALL`; Lua is much cheaper per call |
| GCD | 2147483647/1073741823 | 0.29x | 3.4x | Tiny body, dispatch + boxing dominate |
| PowMod | 1234567/7654321/1e9+7 | 0.56x | 1.8x | |
| Sum | n=5M | 0.69x | **1.5x** | Closest numeric loop (in-place `ADD` + countdown `FORLOOP`) |
| BubbleSort | n=200 | 0.29x | 3.4x | Table index R/W, no array part |
| Sieve | n=5000 | 0.30x | 3.3x | |
| BinarySearch | n=1000 | 0.33x | 3.1x | |
| FastPow | 1234567/7654321/1e9+7 | 0.48x | 2.1x | |
| Popcount | n=100K | 0.33x | 3.0x | |
| InsertionSort | n=200 | 0.22x | 4.5x | |
| MatMul | 3×3 | 0.51x | 1.9x | |
| Vector3 | n=1M | 0.29x | 3.5x | Hash-only tables vs Lua array part |
| FloatPoly | n=1M | 0.77x | **1.3x** | Tight float loop, near Sum |

### String (string)

| Scenario | Param | INTERP vs Lua | INTERP / Lua time | Notes |
|----------|-------|---------------|-------------------|-------|
| StringLen | n=10K | **2.3x** | 0.44x | Work in C (`string.len`); INTERP faster |
| StringSub | n=10K | 1.86x | 0.54x | |
| StringRep | n=1000 | 1.08x | 0.93x | Roughly on par |
| StringReverse | n=10K | 1.13x | 0.89x | |
| StringLower | n=10K | **6.2x** | 0.16x | C native `string.lower` |
| StringUpper | n=10K | **6.1x** | 0.16x | Same as above |
| StringByte | n=1000 | 0.77x | 1.3x | |
| StringChar | n=500 | 0.44x | 2.3x | |
| StringFormat | n=500 | 0.78x | 1.3x | |
| StringFind | n=10K | 1.23x | 0.82x | Plain substring |
| StringGsub | n=1000 | 0.31x | 3.2x | ECMAScript Boost.Regex |
| ToNumber | n=1 | 0.34x | 2.9x | Single `Call()` + boxing |
| ToString | n=500 | 0.74x | 1.4x | |
| StringFindPattern | n=1000 | 0.25x | 4.0x | ECMAScript regex |
| StringGmatch | n=1000 | 0.55x | 1.8x | ECMAScript regex |

### Table Operations (table)

| Scenario | Param | INTERP vs Lua | INTERP / Lua time | Notes |
|----------|-------|---------------|-------------------|-------|
| TableInsert | n=5K | 0.29x | 3.5x | No array part |
| TableRemove | n=5K | 0.41x | 2.4x | |
| TableConcat | n=1000 | 1.05x | 0.95x | On par (C concat) |
| TablePack | n=1 | 1.12x | 0.90x | |
| TableMove | n=5K | 0.38x | 2.6x | |
| TableSort | n=1000 | 1.87x | 0.54x | C `table.sort` |
| TableCreate | n=5K | 0.30x | 3.3x | |
| HashInsert | n=1000 | 1.33x | 0.75x | Arena vs Lua GC |
| HashLookup | n=1000 | 1.13x | 0.89x | |
| NestedTable | n=10K | 0.25x | 4.0x | Repeated `GETTABLE` |

### Function Calls (function)

| Scenario | Param | INTERP vs Lua | INTERP / Lua time | Notes |
|----------|-------|---------------|-------------------|-------|
| EmptyCall | n=100K | 0.13x | **7.8x** | Per-call `CallByName` / interp enter |
| Recursion | n=25 | 0.17x | **6.0x** | Same as Fibonacci |
| Variadic | n=1 | 0.19x | 5.2x | `select` / vararg |
| MultiReturn | n=10K | 0.18x | 5.6x | |
| Closure | n=1000 | 0.36x | 2.7x | |
| TailRecursion | n=5K | 0.03x | **34x** | No TCO; real C recursion (`ulimit -s unlimited`) |

### GC & Memory Pressure (gc)

| Scenario | Param | INTERP vs Lua | INTERP / Lua time | Notes |
|----------|-------|---------------|-------------------|-------|
| TableChurn | n=1000 | **2.2x** | 0.45x | Arena, no GC |
| StringChurn | n=1000 | 1.40x | 0.71x | |
| MixedAlloc | n=1000 | 1.72x | 0.58x | |

### Math Functions (math)

| Scenario | Param | INTERP vs Lua | INTERP / Lua time | Notes |
|----------|-------|---------------|-------------------|-------|
| MathTrig (sin+cos) | n=100K | 0.49x | 2.0x | Native math + interp loop |
| MathSqrt | n=100K | 0.33x | 3.1x | |
| MathExpLog | n=100K | 0.42x | 2.4x | |
| MathMinMax | n=100K | 0.43x | 2.3x | |

### Interpreter vs Lua — key findings

1. **Tight integer/float loops are the closest**: Sum n=5M is **1.5× Lua**, FloatPoly n=1M is **1.3× Lua**. Remaining cost is tagged `CVar` copies and `boxes[]` checks on every register R/W.

2. **Call-heavy code is 6–8× slower**: Fibonacci / Recursion / EmptyCall. Lua's call is a VM opcode; FakeLua INTERP re-enters `Call()` / `CallByName` with boxed arguments.

3. **Tail recursion is the outlier (34×)**: Lua 5.4 turns `return f(...)` into a loop. INTERP does not; n=5000 is 5000 nested C frames (and overflows the default 8 MiB stack).

4. **Table loops without an array part are ~3–4× slower** (BubbleSort, Vector3, TableInsert, NestedTable). Hash-only `VarTable` vs Lua's array part.

5. **C stdlib work can beat Lua**: `string.lower`/`upper` **6.2×**, `string.len` 2.3×, `table.sort` 1.9×, TableChurn 2.2× (arena). When the opcode loop is not the bottleneck, native helpers plus no-GC allocation win.

6. **Regex remains slower** (Gsub 3.2× Lua time, FindPattern 4.0×): ECMAScript Boost.Regex, not Lua pattern — same tradeoff as GCC JIT.

---

## JIT GCC vs Lua / C++ (2026-08-14)

The tables below are from a **previous** run on a different machine (AMD EPYC 7K62, GCC 15.1.0). They are **not** comparable to the INTERP numbers above. **GCC vs Lua** = speedup of FakeLua GCC relative to Lua 5.4 (>1 means GCC JIT is faster); **GCC vs C++** = FakeLua GCC / handwritten C++ (<1 means FakeLua is faster).

### Environment (GCC run)

- Date: 2026-08-14
- Machine: AMD EPYC 7K62 48-Core Processor, 2 X 2595.12 MHz CPU s
- CPU Caches: L1d 32 KiB (x2), L1i 32 KiB (x2), L2 4096 KiB (x2), L3 16384 KiB (x1)
- Build mode: **Release** (`-O3 -DNDEBUG`), GCC 15.1.0
- FakeLua GCC JIT: **Release mode** (`debug_mode=false`, GCC `-O3` optimization)
- Binary: `build/bin/bench_mark`

> C++ reference implementations in some scenarios may be fully folded by the compiler (see annotations below), in which case the GCC vs C++ column is not informative.

### Algorithms (algo)

| Scenario | Param | GCC vs Lua | GCC vs C++ | Notes |
|----------|-------|-----------|-----------|-------|
| Fibonacci | n=32 | **36.6x** | 0.13 | Numeric specialization, close to C++ |
| GCD | 2147483647/1073741823 | 0.45x | 10.8 | Single call ~0.4 µs, dominated by call overhead |
| PowMod | 1234567/7654321/1e9+7 | 1.7x | 2.56 | |
| Sum | n=5M | **30.4x** | 0.06 | GCC vectorization, far faster than C++ |
| BubbleSort | n=200 | 1.9x | 2.80 | Table index read/write drag |
| Sieve | n=5000 | 1.8x | 2.68 | |
| BinarySearch | n=1000 | 3.7x | 2.40 | |
| FastPow | 1234567/7654321/1e9+7 | 1.6x | 2.45 | |
| Popcount | n=100K | **37.3x** | 0.12 | Extreme bitwise optimization |
| InsertionSort | n=200 | 2.6x | 2.92 | |
| MatMul | 3×3 | 2.9x | 5.10 | |
| Vector3 | n=1M | 4.8x | 5.85 | Table specialized to struct, pointer offset |
| FloatPoly | n=1M | **34.9x** | 0.49 | Float specialization, GCC 2x faster than C++ |

### String (string)

| Scenario | Param | GCC vs Lua | GCC vs C++ | Notes |
|----------|-------|-----------|-----------|-------|
| StringLen | n=10K | 1.8x | 67.3 | |
| StringSub | n=10K | 1.3x | 6.15 | |
| StringRep | n=1000 | 1.1x | 0.16 | CGen `FlStringRep` inline |
| StringReverse | n=10K | 1.7x | 0.26 | CGen `FlStringReverse` inline |
| StringLower | n=10K | **6.1x** | 0.02 | CGen inline + ASCII single pass |
| StringUpper | n=10K | **5.0x** | 0.02 | Same as above |
| StringByte | n=1000 | 1.0x | 37.7 | |
| StringChar | n=500 | 1.0x | 10.6 | |
| StringFormat | n=500 | 1.8x | 2.71 | Const `"%d"` → `FlFormatInt` |
| StringFind | n=10K | 0.85x | 19.2 | Plain substring search (`FlStringFindPlain` inline, but still has call overhead) |
| StringGsub | n=1000 | 0.06x | 38.1 | ECMAScript regex (see below) |
| ToNumber | n=1 | 0.53x | 5.65 | CGen `FlTonumber` decimal integer inline parse (same input `"1234567890"`) |
| ToString | n=500 | 0.74x | 0.01 | INT → `FlFormatInt` |
| StringFindPattern | n=1000 | 0.12x | 39.6 | ECMAScript regex; compile cache added |
| StringGmatch | n=1000 | 0.21x | 23.3 | ECMAScript regex; compile cache added |

> **On regex being slower than Lua**: FakeLua's `string.find` / `match` / `gmatch` / `gsub` use **ECMAScript Boost.Regex** (with process-level compile caching), which is more powerful than Lua 5.4's built-in pattern (lookahead, full character classes, non-greedy, etc.). Therefore regex scenarios being slower than Lua (currently ~0.06~0.21x) is **acceptable** — it's capability for performance. To match Lua in the future, the direction is a separate Lua pattern engine, not further optimizing Boost.Regex. Scripts uniformly use `[0-9]+` (semantically identical in Lua pattern and ECMAScript regex).

### Table Operations (table)

| Scenario | Param | GCC vs Lua | GCC vs C++ | Notes |
|----------|-------|-----------|-----------|-------|
| TableInsert | n=5K | **4.0x** | 4.87 | |
| TableRemove | n=5K | **3.2x** | 2.99 | |
| TableConcat | n=1000 | 1.0x | 4.16 | Arena single write |
| TablePack | n=1 | 1.9x | 157 | |
| TableMove | n=5K | 1.0x | 2.84 | CGen `FlTableMove` + empty table pre-grow |
| TableSort | n=1000 | 1.2x | 4.74 | |
| TableCreate | n=5K | 1.1x | 2.20 | |
| HashInsert | n=1000 | 2.0x | 0.88 | |
| HashLookup | n=1000 | 1.9x | 0.45 | |
| NestedTable | n=10K | 3.4x | 1.39 | Outer table pointer offset traversal |

### Function Calls (function)

| Scenario | Param | GCC vs Lua | GCC vs C++ | Notes |
|----------|-------|-----------|-----------|-------|
| EmptyCall | n=100K | **14.1x** | 61561 | C++ inlined to 0 |
| Recursion | n=25 | **42.9x** | 0.13 | Numeric specialization |
| Variadic | n=1 | 0.91x | 75.4 | High vararg construction overhead |
| MultiReturn | n=10K | **23.3x** | 0.46 | |
| Closure | n=1000 | 2.1x | 19.4 | |
| TailRecursion | n=5K | **107.5x** | 0.07 | Tail call to loop + vectorization |

### GC & Memory Pressure (gc)

| Scenario | Param | GCC vs Lua | GCC vs C++ | Notes |
|----------|-------|-----------|-----------|-------|
| TableChurn | n=1000 | **9.3x** | 1.24 | Arena allocator advantage |
| StringChurn | n=1000 | 2.2x | 1.47 | String allocation is a weakness |
| MixedAlloc | n=1000 | 2.3x | 1.68 | |

### Math Functions (math)

| Scenario | Param | GCC vs Lua | GCC vs C++ | Notes |
|----------|-------|-----------|-----------|-------|
| MathTrig (sin+cos) | n=100K | **5.8x** | 0.87 | Close to native C++ speed |
| MathSqrt | n=100K | **22.3x** | 0.48 | GCC 2x faster than C++ |
| MathExpLog | n=100K | **4.3x** | 0.87 | |
| MathMinMax | n=100K | **10.4x** | 0.90 | |

### Key Findings

1. **Pure numeric scenarios comprehensively beat Lua 5.4 and approach handwritten C++**: Fibonacci 36.6x, TailRecursion 107.5x, Recursion 42.9x, FloatPoly 34.9x, Popcount 37.3x, Sum 30.4x. Numeric specialization makes the generated C code nearly identical to handwritten versions; the rest is handled by GCC `-O3`.

2. **All four math library functions are faster than Lua** (4.3x~22.3x); sin/cos/sqrt approach native C++ speed.

3. **Most string standard library functions are faster than Lua**: lower/upper 5-6x, format 1.8x, rep 1.1x, reverse 1.7x — all via CGen inline (`FlStringLower/Upper`, `FlFormatInt`, `FlStringRep`, `FlStringReverse`) avoiding `FakeluaCallByName` overhead. Only StringFind (plain) at 0.85x is slightly slower than Lua, the remaining gap being single-call dispatch overhead.

4. **Table operations are overall advantageous**: table.insert 4.0x, remove 3.2x, sort 1.2x faster than Lua; move / concat roughly on par. `VarTable` caches contiguous integer key prefix length making `#t` O(1); host side and JIT side share the same hash/bucket layout.

5. **Arena allocator shows clear advantage in heavy table-creation scenarios**: TableChurn 9.3x faster than Lua — no GC, bulk free.

6. **Regex scenarios being slower than Lua is acceptable** (Gsub 0.06x, FindPattern 0.12x, Gmatch 0.21x): uses more powerful ECMAScript Boost.Regex (not Lua pattern), with compile caching; different capability, not a target to match.

7. **Remaining slow items are dominated by call dispatch overhead**: GCD (0.45x), ToNumber (0.53x), ToString (0.74x), Variadic (0.91x) have extremely small function bodies (< 1 µs); JIT CVar boxing/unboxing and calling convention overhead dominate — not the computation itself being slow. Cross-function inlining or calling convention optimization is needed to catch up.

   > Note: ToNumber bench parses the same input string `"1234567890"` on both sides. The remaining gap is mainly from single `Call()` dispatch and string boxing constant overhead, not parsing itself.

---

## Complete Raw Output (Lua vs INTERP, 2026-09-16)

Filter: `BM_Lua_|BM_FakeLua_.*_INTERP`. CPU times are the ones used in the INTERP vs Lua tables above.

```text
Starting benchmarks...
2026-09-16T15:48:38+08:00
Running ./bin/bench_mark
Run on (16 X 2000 MHz CPU s)
CPU Caches:
  L1 Data 64 KiB (x16)
  L1 Instruction 64 KiB (x16)
  L2 Unified 512 KiB (x16)
  L3 Unified 65536 KiB (x2)
***WARNING*** ASLR is enabled, the results may have unreproducible noise in them.
----------------------------------------------------------------------------
Benchmark                                                  Time             CPU   Iterations
----------------------------------------------------------------------------
BM_Lua_Fibonacci/20                                     1144569 ns      1084586 ns          637
BM_Lua_Fibonacci/25                                    13283206 ns     12397758 ns           57
BM_Lua_Fibonacci/30                                   150373384 ns    136381787 ns            5
BM_Lua_Fibonacci/32                                   334257089 ns    334253452 ns            2
BM_FakeLua_Fibonacci_INTERP/20                          8578088 ns      8577897 ns           81
BM_FakeLua_Fibonacci_INTERP/25                         89749845 ns     89746764 ns            8
BM_FakeLua_Fibonacci_INTERP/30                        975132570 ns    975108155 ns            1
BM_FakeLua_Fibonacci_INTERP/32                       2552108645 ns   2552008862 ns            1
BM_Lua_GCD/832040/514229                                    556 ns          556 ns      1290670
BM_Lua_GCD/123456789/987654321                              149 ns          149 ns      4642582
BM_Lua_GCD/2147483647/1073741823                            134 ns          134 ns      5272710
BM_FakeLua_GCD_INTERP/832040/514229                        1788 ns         1788 ns       407229
BM_FakeLua_GCD_INTERP/123456789/987654321                   518 ns          518 ns      1176854
BM_FakeLua_GCD_INTERP/2147483647/1073741823                 460 ns          460 ns      1597461
BM_Lua_PowMod/2/1000/1000000007                             699 ns          699 ns       990032
BM_Lua_PowMod/7/1000000/1000000007                         1210 ns         1210 ns       590702
BM_Lua_PowMod/1234567/7654321/1000000007                   1526 ns         1526 ns       450278
BM_FakeLua_PowMod_INTERP/2/1000/1000000007                 1394 ns         1394 ns       504574
BM_FakeLua_PowMod_INTERP/7/1000000/1000000007              2395 ns         2395 ns       299151
BM_FakeLua_PowMod_INTERP/1234567/7654321/1000000007        2714 ns         2713 ns       257497
BM_Lua_Sum/10000                                          68382 ns        68378 ns        10435
BM_Lua_Sum/100000                                        690733 ns       690725 ns          987
BM_Lua_Sum/1000000                                      6726799 ns      6726654 ns          104
BM_Lua_Sum/5000000                                     33570042 ns     33568361 ns           21
BM_FakeLua_Sum_INTERP/10000                               98246 ns        98242 ns         7088
BM_FakeLua_Sum_INTERP/100000                             975815 ns       975789 ns          719
BM_FakeLua_Sum_INTERP/1000000                           9770269 ns      9769755 ns           72
BM_FakeLua_Sum_INTERP/5000000                          48913989 ns     48914112 ns           14
BM_Lua_BubbleSort/50                                      97623 ns        97619 ns         7161
BM_Lua_BubbleSort/100                                    399558 ns       399543 ns         1844
BM_Lua_BubbleSort/200                                   1529475 ns      1529435 ns          440
BM_FakeLua_BubbleSort_INTERP/50                          333897 ns       333884 ns         2106
BM_FakeLua_BubbleSort_INTERP/100                        1349297 ns      1349251 ns          532
BM_FakeLua_BubbleSort_INTERP/200                        5262565 ns      5262498 ns          132
BM_Lua_Sieve/100                                           9456 ns         9456 ns        74524
BM_Lua_Sieve/500                                          36011 ns        36011 ns        19372
BM_Lua_Sieve/1000                                         72348 ns        72345 ns         9478
BM_Lua_Sieve/5000                                        324762 ns       324745 ns         2091
BM_FakeLua_Sieve_INTERP/100                               17545 ns        17544 ns        40212
BM_FakeLua_Sieve_INTERP/500                               89880 ns        89879 ns         7841
BM_FakeLua_Sieve_INTERP/1000                             192069 ns       192069 ns         3874
BM_FakeLua_Sieve_INTERP/5000                            1070652 ns      1070611 ns          654
BM_Lua_BinarySearch/100                                   41313 ns        41312 ns        17030
BM_Lua_BinarySearch/500                                  282204 ns       282205 ns         2473
BM_Lua_BinarySearch/1000                                 636610 ns       636602 ns         1108
BM_FakeLua_BinarySearch_INTERP/100                       124217 ns       124214 ns         5632
BM_FakeLua_BinarySearch_INTERP/500                       863565 ns       863516 ns          811
BM_FakeLua_BinarySearch_INTERP/1000                     1947828 ns      1947729 ns          361
BM_Lua_FastPow/2/1000/1000000007                            704 ns          704 ns       992443
BM_Lua_FastPow/7/1000000/1000000007                        1103 ns         1103 ns       638242
BM_Lua_FastPow/1234567/7654321/1000000007                  1462 ns         1462 ns       479726
BM_FakeLua_FastPow_INTERP/2/1000/1000000007                1523 ns         1523 ns       462710
BM_FakeLua_FastPow_INTERP/7/1000000/1000000007             2578 ns         2578 ns       283648
BM_FakeLua_FastPow_INTERP/1234567/7654321/1000000007       3051 ns         3050 ns       231470
BM_Lua_Popcount/1000                                     118658 ns       118656 ns         5883
BM_Lua_Popcount/10000                                   1471087 ns      1471053 ns          475
BM_Lua_Popcount/100000                                 17968504 ns     17967236 ns           39
BM_FakeLua_Popcount_INTERP/1000                          343823 ns       343819 ns         2028
BM_FakeLua_Popcount_INTERP/10000                        4310395 ns      4309845 ns          162
BM_FakeLua_Popcount_INTERP/100000                      53686039 ns     53683637 ns           13
BM_Lua_InsertionSort/50                                   49325 ns        49323 ns        13744
BM_Lua_InsertionSort/100                                 180948 ns       180941 ns         3869
BM_Lua_InsertionSort/200                                 705938 ns       705929 ns          989
BM_FakeLua_InsertionSort_INTERP/50                       207825 ns       207820 ns         3366
BM_FakeLua_InsertionSort_INTERP/100                      804500 ns       804489 ns          877
BM_FakeLua_InsertionSort_INTERP/200                     3155980 ns      3155957 ns          221
BM_Lua_MatMul                                              4416 ns         4416 ns       158495
BM_FakeLua_MatMul_INTERP                                   8596 ns         8596 ns        81727
BM_Lua_Vector3/10000                                    1709644 ns      1709608 ns          425
BM_Lua_Vector3/100000                                  17277215 ns     17276556 ns           41
BM_Lua_Vector3/1000000                                167336192 ns    167336712 ns            4
BM_FakeLua_Vector3_INTERP/10000                         5870958 ns      5870569 ns          120
BM_FakeLua_Vector3_INTERP/100000                       59038871 ns     59037918 ns           12
BM_FakeLua_Vector3_INTERP/1000000                     583587155 ns    583577824 ns            1
BM_Lua_FloatPoly/1000000                               92155439 ns     92148782 ns            6
BM_FakeLua_FloatPoly_INTERP/1000000                   119274266 ns    119273134 ns            6
BM_Lua_EmptyCall/10000                                   410740 ns       410735 ns         1715
BM_Lua_EmptyCall/100000                                 4115022 ns      4114857 ns          163
BM_FakeLua_EmptyCall_INTERP/10000                       3225408 ns      3225336 ns          217
BM_FakeLua_EmptyCall_INTERP/100000                     32207058 ns     32207147 ns           22
BM_Lua_Recursion/10                                       10530 ns        10530 ns        66500
BM_Lua_Recursion/20                                     1285968 ns      1285954 ns          548
BM_Lua_Recursion/25                                    14696263 ns     14696145 ns           49
BM_FakeLua_Recursion_INTERP/10                            67874 ns        67869 ns        10328
BM_FakeLua_Recursion_INTERP/20                          8065164 ns      8064800 ns           82
BM_FakeLua_Recursion_INTERP/25                         88200150 ns     88200363 ns            8
BM_Lua_Variadic/1                                           548 ns          548 ns      1271307
BM_FakeLua_Variadic_INTERP/1                               2825 ns         2825 ns       250154
BM_Lua_MultiReturn/1000                                   61903 ns        61903 ns        11129
BM_Lua_MultiReturn/10000                                 591379 ns       591332 ns         1169
BM_FakeLua_MultiReturn_INTERP/1000                       329144 ns       329132 ns         2145
BM_FakeLua_MultiReturn_INTERP/10000                     3290976 ns      3290938 ns          213
BM_Lua_Closure/100                                        24524 ns        24522 ns        28867
BM_Lua_Closure/1000                                      244847 ns       244544 ns         2871
BM_FakeLua_Closure_INTERP/100                             68483 ns        68479 ns        10128
BM_FakeLua_Closure_INTERP/1000                           670507 ns       670485 ns         1049
BM_Lua_TailRecursion/100                                   3762 ns         3762 ns       186916
BM_Lua_TailRecursion/1000                                 36355 ns        36353 ns        19180
BM_Lua_TailRecursion/5000                                185267 ns       185121 ns         3844
BM_FakeLua_TailRecursion_INTERP/100                       40193 ns        40187 ns        17432
BM_FakeLua_TailRecursion_INTERP/1000                     503108 ns       503077 ns         1000
BM_FakeLua_TailRecursion_INTERP/5000        6259916 ns      6259115 ns          111
BM_Lua_TableChurn/100                         59469 ns        59465 ns        11172
BM_Lua_TableChurn/500                        291246 ns       291232 ns         2342
BM_Lua_TableChurn/1000                       608475 ns       608455 ns         1156
BM_FakeLua_TableChurn_INTERP/100              27575 ns        27573 ns        25768
BM_FakeLua_TableChurn_INTERP/500             137383 ns       137377 ns         5133
BM_FakeLua_TableChurn_INTERP/1000            273501 ns       273496 ns         2568
BM_Lua_StringChurn/100                        86787 ns        86784 ns         7981
BM_Lua_StringChurn/500                       560238 ns       560227 ns         1216
BM_Lua_StringChurn/1000                     1137369 ns      1137349 ns          611
BM_FakeLua_StringChurn_INTERP/100             82985 ns        82979 ns         8970
BM_FakeLua_StringChurn_INTERP/500            381000 ns       380990 ns         1793
BM_FakeLua_StringChurn_INTERP/1000           812526 ns       812512 ns          882
BM_Lua_MixedAlloc/100                        108735 ns       108731 ns         6403
BM_Lua_MixedAlloc/500                        564361 ns       564349 ns         1297
BM_Lua_MixedAlloc/1000                      1095508 ns      1095467 ns          633
BM_FakeLua_MixedAlloc_INTERP/100              61081 ns        61078 ns        11328
BM_FakeLua_MixedAlloc_INTERP/500             317101 ns       317086 ns         2245
BM_FakeLua_MixedAlloc_INTERP/1000            636884 ns       636886 ns         1073
BM_Lua_MathTrig/100000                     28406530 ns     28399049 ns           25
BM_FakeLua_MathTrig_INTERP/100000          57870338 ns     57867951 ns           12
BM_Lua_MathSqrt/100000                      6499086 ns      6498149 ns          112
BM_FakeLua_MathSqrt_INTERP/100000          19838534 ns     19837035 ns           35
BM_Lua_MathExpLog/100000                   21038376 ns     21034487 ns           34
BM_FakeLua_MathExpLog_INTERP/100000        49770330 ns     49764563 ns           14
BM_Lua_MathMinMax/100000                   33257803 ns     33253252 ns           21
BM_FakeLua_MathMinMax_INTERP/100000        78068647 ns     78066167 ns            9
BM_Lua_StringLen/10                             133 ns          133 ns      5255089
BM_Lua_StringLen/100                            299 ns          299 ns      2315101
BM_Lua_StringLen/1000                           598 ns          598 ns      1214737
BM_Lua_StringLen/10000                         2402 ns         2402 ns       294565
BM_FakeLua_StringLen_INTERP/10                  311 ns          311 ns      2212979
BM_FakeLua_StringLen_INTERP/100                 365 ns          365 ns      1932951
BM_FakeLua_StringLen_INTERP/1000                565 ns          565 ns      1232995
BM_FakeLua_StringLen_INTERP/10000              1064 ns         1064 ns       657973
BM_Lua_StringSub/10                             266 ns          266 ns      2603296
BM_Lua_StringSub/100                            500 ns          500 ns      1311387
BM_Lua_StringSub/1000                          1046 ns         1046 ns       667537
BM_Lua_StringSub/10000                         3807 ns         3806 ns       191614
BM_FakeLua_StringSub_INTERP/10                  753 ns          753 ns       926310
BM_FakeLua_StringSub_INTERP/100                 869 ns          869 ns       807951
BM_FakeLua_StringSub_INTERP/1000               1130 ns         1130 ns       609855
BM_FakeLua_StringSub_INTERP/10000              2046 ns         2045 ns       329919
BM_Lua_StringRep/10                             327 ns          327 ns      2154444
BM_Lua_StringRep/100                            932 ns          932 ns       727883
BM_Lua_StringRep/1000                          5267 ns         5265 ns       133440
BM_FakeLua_StringRep_INTERP/10                  726 ns          725 ns       931657
BM_FakeLua_StringRep_INTERP/100                1117 ns         1117 ns       616015
BM_FakeLua_StringRep_INTERP/1000               4874 ns         4874 ns       145223
BM_Lua_StringReverse/10                         258 ns          257 ns      2716507
BM_Lua_StringReverse/100                        696 ns          696 ns       970621
BM_Lua_StringReverse/1000                      1952 ns         1952 ns       361174
BM_Lua_StringReverse/10000                    12733 ns        12733 ns        54546
BM_FakeLua_StringReverse_INTERP/10              712 ns          712 ns       979915
BM_FakeLua_StringReverse_INTERP/100             883 ns          883 ns       793220
BM_FakeLua_StringReverse_INTERP/1000           2191 ns         2191 ns       318712
BM_FakeLua_StringReverse_INTERP/10000         11298 ns        11296 ns        62191
BM_Lua_StringLower/10                           252 ns          251 ns      2809706
BM_Lua_StringLower/100                          696 ns          696 ns      1013403
BM_Lua_StringLower/1000                        2313 ns         2309 ns       310662
BM_Lua_StringLower/10000                      16374 ns        16350 ns        43397
BM_FakeLua_StringLower_INTERP/10                619 ns          619 ns      1088881
BM_FakeLua_StringLower_INTERP/100               766 ns          766 ns       933398
BM_FakeLua_StringLower_INTERP/1000             1179 ns         1179 ns       555653
BM_FakeLua_StringLower_INTERP/10000            2653 ns         2653 ns       258309
BM_Lua_StringUpper/10                           250 ns          250 ns      2752023
BM_Lua_StringUpper/100                          712 ns          712 ns       992559
BM_Lua_StringUpper/1000                        2263 ns         2263 ns       299372
BM_Lua_StringUpper/10000                      15686 ns        15686 ns        44445
BM_FakeLua_StringUpper_INTERP/10                559 ns          559 ns      1225659
BM_FakeLua_StringUpper_INTERP/100               664 ns          664 ns      1044102
BM_FakeLua_StringUpper_INTERP/1000             1079 ns         1079 ns       650906
BM_FakeLua_StringUpper_INTERP/10000            2586 ns         2586 ns       271093
BM_Lua_StringByte/10                            216 ns          216 ns      3245889
BM_Lua_StringByte/100                           410 ns          410 ns      1707452
BM_Lua_StringByte/1000                          708 ns          708 ns       982592
BM_FakeLua_StringByte_INTERP/10                 665 ns          665 ns      1004509
BM_FakeLua_StringByte_INTERP/100                737 ns          737 ns       936469
BM_FakeLua_StringByte_INTERP/1000               918 ns          918 ns       758807
BM_Lua_StringChar/10                           3101 ns         3100 ns       227212
BM_Lua_StringChar/100                         18121 ns        18108 ns        38797
BM_Lua_StringChar/500                         80596 ns        80591 ns         8481
BM_FakeLua_StringChar_INTERP/10                4713 ns         4713 ns       148278
BM_FakeLua_StringChar_INTERP/100              38219 ns        38219 ns        18354
BM_FakeLua_StringChar_INTERP/500             183011 ns       183006 ns         3867
BM_Lua_StringFormat/10                         3735 ns         3735 ns       192317
BM_Lua_StringFormat/100                       34750 ns        34749 ns        19989
BM_Lua_StringFormat/500                      186590 ns       186590 ns         3834
BM_FakeLua_StringFormat_INTERP/10              5285 ns         5285 ns       124800
BM_FakeLua_StringFormat_INTERP/100            47515 ns        47515 ns        14820
BM_FakeLua_StringFormat_INTERP/500           238127 ns       238110 ns         2943
BM_Lua_StringFind/10                            371 ns          371 ns      1948124
BM_Lua_StringFind/100                           504 ns          504 ns      1312309
BM_Lua_StringFind/1000                          860 ns          860 ns       836242
BM_Lua_StringFind/10000                        3017 ns         2850 ns       246275
BM_FakeLua_StringFind_INTERP/10                1135 ns         1082 ns       672638
BM_FakeLua_StringFind_INTERP/100               1068 ns         1064 ns       695238
BM_FakeLua_StringFind_INTERP/1000              1501 ns         1500 ns       526220
BM_FakeLua_StringFind_INTERP/10000             2325 ns         2325 ns       257862
BM_Lua_StringGsub/10                            847 ns          847 ns       871151
BM_Lua_StringGsub/100                          4804 ns         4804 ns       144975
BM_Lua_StringGsub/1000                        41984 ns        41983 ns        16648
BM_FakeLua_StringGsub_INTERP/10                2700 ns         2700 ns       257624
BM_FakeLua_StringGsub_INTERP/100              14755 ns        14754 ns        47288
BM_FakeLua_StringGsub_INTERP/1000            134159 ns       134159 ns         5268
BM_Lua_ToNumber/1                               210 ns          210 ns      3308067
BM_FakeLua_ToNumber_INTERP/1                    613 ns          613 ns      1157814
BM_Lua_ToString/10                              531 ns          531 ns      1317167
BM_Lua_ToString/100                             545 ns          545 ns      1287984
BM_Lua_ToString/500                             510 ns          510 ns      1268430
BM_FakeLua_ToString_INTERP/10                   687 ns          687 ns      1009822
BM_FakeLua_ToString_INTERP/100                  693 ns          693 ns      1010478
BM_FakeLua_ToString_INTERP/500                  691 ns          691 ns      1012225
BM_Lua_StringFindPattern/1000                841315 ns       841113 ns          831
BM_FakeLua_StringFindPattern_INTERP/1000    3395999 ns      3395933 ns          207
BM_Lua_StringGmatch/1000                    1254819 ns      1254217 ns          580
BM_FakeLua_StringGmatch_INTERP/1000         2282902 ns      2282906 ns          309
BM_Lua_TableInsert/100                        11848 ns        11847 ns        58757
BM_Lua_TableInsert/500                        50790 ns        50783 ns        12873
BM_Lua_TableInsert/1000                       99252 ns        99248 ns         7221
BM_Lua_TableInsert/5000                      481485 ns       481467 ns         1459
BM_FakeLua_TableInsert_INTERP/100             35333 ns        35331 ns        21032
BM_FakeLua_TableInsert_INTERP/500            170284 ns       170282 ns         4103
BM_FakeLua_TableInsert_INTERP/1000           338430 ns       338377 ns         2055
BM_FakeLua_TableInsert_INTERP/5000          1675580 ns      1675498 ns          420
BM_Lua_TableRemove/100                        16745 ns        16744 ns        44233
BM_Lua_TableRemove/500                        69826 ns        69824 ns         9644
BM_Lua_TableRemove/1000                      139181 ns       139181 ns         4979
BM_Lua_TableRemove/5000                      818740 ns       818715 ns          893
BM_FakeLua_TableRemove_INTERP/100             39744 ns        39741 ns        17697
BM_FakeLua_TableRemove_INTERP/500            193218 ns       193204 ns         3640
BM_FakeLua_TableRemove_INTERP/1000           417360 ns       417277 ns         1816
BM_FakeLua_TableRemove_INTERP/5000          1980494 ns      1980486 ns          357
BM_Lua_TableConcat/100                        40270 ns        40269 ns        17312
BM_Lua_TableConcat/500                       195418 ns       195390 ns         3575
BM_Lua_TableConcat/1000                      395958 ns       395955 ns         1766
BM_FakeLua_TableConcat_INTERP/100             38597 ns        38596 ns        17322
BM_FakeLua_TableConcat_INTERP/500            194953 ns       194946 ns         3471
BM_FakeLua_TableConcat_INTERP/1000           378109 ns       378105 ns         1854
BM_Lua_TablePack/1                             1391 ns         1391 ns       516095
BM_FakeLua_TablePack_INTERP/1                  1246 ns         1246 ns       583527
BM_Lua_TableMove/100                           8386 ns         8386 ns        81054
BM_Lua_TableMove/500                          29067 ns        29066 ns        27500
BM_Lua_TableMove/1000                         46120 ns        46119 ns        14068
BM_Lua_TableMove/5000                        227038 ns       227034 ns         3061
BM_FakeLua_TableMove_INTERP/100               11804 ns        11804 ns        59495
BM_FakeLua_TableMove_INTERP/500               52757 ns        52753 ns        13284
BM_FakeLua_TableMove_INTERP/1000             104800 ns       104798 ns         6719
BM_FakeLua_TableMove_INTERP/5000             589798 ns       589783 ns         1170
BM_Lua_TableSort/100                          28256 ns        28256 ns        24447
BM_Lua_TableSort/500                         167972 ns       167969 ns         4165
BM_Lua_TableSort/1000                        368101 ns       368079 ns         1912
BM_FakeLua_TableSort_INTERP/100               18169 ns        18138 ns        38351
BM_FakeLua_TableSort_INTERP/500               92839 ns        92823 ns         7457
BM_FakeLua_TableSort_INTERP/1000             197154 ns       196995 ns         3544
BM_Lua_TableCreate/1000                       26310 ns        26205 ns        27662
BM_Lua_TableCreate/3000                       75548 ns        75535 ns         9256
BM_Lua_TableCreate/5000                      130772 ns       130580 ns         5470
BM_FakeLua_TableCreate_INTERP/1000            70613 ns        70599 ns        10037
BM_FakeLua_TableCreate_INTERP/3000           236687 ns       236599 ns         3079
BM_FakeLua_TableCreate_INTERP/5000           432378 ns       432278 ns         1371
BM_Lua_HashInsert/100                         59467 ns        59432 ns        11755
BM_Lua_HashInsert/500                        308858 ns       308734 ns         2319
BM_Lua_HashInsert/1000                       600007 ns       599999 ns         1037
BM_FakeLua_HashInsert_INTERP/100              39912 ns        39911 ns        17196
BM_FakeLua_HashInsert_INTERP/500             198025 ns       198016 ns         3588
BM_FakeLua_HashInsert_INTERP/1000            449572 ns       449553 ns         1558
BM_Lua_HashLookup/100                         48837 ns        48833 ns        15309
BM_Lua_HashLookup/500                        250008 ns       249949 ns         2682
BM_Lua_HashLookup/1000                       490965 ns       490927 ns         1423
BM_FakeLua_HashLookup_INTERP/100              42890 ns        42887 ns        15922
BM_FakeLua_HashLookup_INTERP/500             216841 ns       216829 ns         3157
BM_FakeLua_HashLookup_INTERP/1000            434753 ns       434704 ns         1589
BM_Lua_NestedTable/1000                      246962 ns       246938 ns         2869
BM_Lua_NestedTable/10000                    2668007 ns      2667960 ns          274
BM_FakeLua_NestedTable_INTERP/1000          1089201 ns      1089025 ns          650
BM_FakeLua_NestedTable_INTERP/10000        10695225 ns     10692977 ns           67
```

---

## Appendix: GCC/TCC/C++ raw output (2026-08-14)


Below is the complete google benchmark output for all 51 scenarios from `benchmark_algo.cpp` / `benchmark_string.cpp` / `benchmark_table.cpp` / `benchmark_function.cpp` / `benchmark_gc.cpp` / `benchmark_math.cpp` (including TCC data):

> ⚠️ The eight lines for `BM_*_StringFindPattern` / `BM_*_StringGmatch` are old data from **before** the `[0-9]+` correction; the two sides have unequal workloads and should not be compared. Use the corrected values in the tables above.

```text
<string>:1561: warning: assignment of read-only location
Starting benchmarks...
2026-08-14T12:05:23+08:00
Running ./bin/bench_mark
Run on (2 X 2595.12 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x2)
  L1 Instruction 32 KiB (x2)
  L2 Unified 4096 KiB (x2)
  L3 Unified 16384 KiB (x1)
Load Average: 5.87, 5.69, 4.54
***WARNING*** ASLR is enabled, the results may have unreproducible noise in them.
----------------------------------------------------------------------------
Benchmark                                                  Time             CPU   Iterations
----------------------------------------------------------------------------
BM_CPP_BinarySearch/100                                      7,735 ns      4,893 ns    143,193
BM_CPP_BinarySearch/1000                                    94,468 ns     65,183 ns     10,769
BM_CPP_BinarySearch/500                                     53,068 ns     29,810 ns     23,430
BM_CPP_BubbleSort/100                                       98,467 ns     62,909 ns     11,127
BM_CPP_BubbleSort/200                                      388,207 ns    249,256 ns      2,800
BM_CPP_BubbleSort/50                                        26,237 ns     15,964 ns     43,992
BM_CPP_Closure/100                                             792 ns        551 ns  1,272,292
BM_CPP_Closure/1000                                          8,993 ns      5,466 ns    128,618
BM_CPP_EmptyCall/10000                                           9 ns          5 ns 127,122,452
BM_CPP_EmptyCall/100000                                          8 ns          5 ns 127,515,986
BM_CPP_FastPow/1234567/7654321/1000000007                      514 ns        297 ns  2,357,197
BM_CPP_FastPow/2/1000/1000000007                               261 ns        121 ns  5,799,249
BM_CPP_FastPow/7/1000000/1000000007                            373 ns        205 ns  3,429,130
BM_CPP_Fibonacci/20                                        342,994 ns    188,804 ns      3,720
BM_CPP_Fibonacci/25                                      3,686,904 ns  2,091,372 ns        333
BM_CPP_Fibonacci/30                                     37,599,358 ns 23,298,280 ns         30
BM_CPP_Fibonacci/32                                     99,812,719 ns 60,640,845 ns         12
BM_CPP_FloatPoly/1000000                                 7,930,843 ns  5,847,105 ns        120
BM_CPP_GCD/123456789/987654321                                  82 ns         44 ns 15,634,680
BM_CPP_GCD/2147483647/1073741823                                65 ns         39 ns 17,808,461
BM_CPP_GCD/832040/514229                                       270 ns        181 ns  3,855,731
BM_CPP_HashInsert/100                                       42,343 ns     25,099 ns     27,962
BM_CPP_HashInsert/1000                                     404,595 ns    288,678 ns      2,410
BM_CPP_HashInsert/500                                      224,686 ns    141,863 ns      4,922
BM_CPP_HashLookup/100                                      540,670 ns    308,545 ns      2,270
BM_CPP_HashLookup/1000                                     763,223 ns    492,689 ns      1,426
BM_CPP_HashLookup/500                                      609,514 ns    389,886 ns      1,793
BM_CPP_InsertionSort/100                                    51,359 ns     30,374 ns     23,049
BM_CPP_InsertionSort/200                                   204,452 ns    118,372 ns      5,882
BM_CPP_InsertionSort/50                                     13,938 ns      8,006 ns     88,502
BM_CPP_MatMul                                                  485 ns        247 ns  2,846,106
BM_CPP_MathExpLog/100000                                 7,276,256 ns  4,629,740 ns        151
BM_CPP_MathMinMax/100000                                 3,610,545 ns  2,396,188 ns        291
BM_CPP_MathSqrt/100000                                     906,821 ns    571,279 ns      1,234
BM_CPP_MathTrig/100000                                   5,398,646 ns  3,531,282 ns        199
BM_CPP_MixedAlloc/100                                       32,146 ns     23,078 ns     30,205
BM_CPP_MixedAlloc/1000                                     433,047 ns    234,244 ns      2,990
BM_CPP_MixedAlloc/500                                      200,939 ns    116,896 ns      5,985
BM_CPP_MultiReturn/1000                                     10,052 ns      5,480 ns    126,527
BM_CPP_MultiReturn/10000                                    91,075 ns     54,916 ns     12,734
BM_CPP_NestedTable/1000                                     91,243 ns     60,937 ns     11,457
BM_CPP_NestedTable/10000                                 1,132,805 ns    607,423 ns      1,155
BM_CPP_Popcount/1000                                        48,426 ns     32,558 ns     21,481
BM_CPP_Popcount/10000                                      649,786 ns    408,784 ns      1,712
BM_CPP_Popcount/100000                                   7,400,519 ns  5,004,960 ns        140
BM_CPP_PowMod/1234567/7654321/1000000007                       489 ns        297 ns  2,355,934
BM_CPP_PowMod/2/1000/1000000007                                192 ns        121 ns  5,817,991
BM_CPP_PowMod/7/1000000/1000000007                             329 ns        205 ns  3,395,045
BM_CPP_Recursion/10                                          2,623 ns      1,517 ns    461,648
BM_CPP_Recursion/20                                        306,905 ns    188,664 ns      3,708
BM_CPP_Recursion/25                                      3,584,836 ns  2,076,389 ns        337
BM_CPP_Sieve/100                                             2,194 ns      1,462 ns    476,278
BM_CPP_Sieve/1000                                           27,651 ns     15,057 ns     46,837
BM_CPP_Sieve/500                                            14,586 ns      7,367 ns     94,648
BM_CPP_Sieve/5000                                          125,191 ns     79,410 ns      8,797
BM_CPP_StringByte/10                                            30 ns         16 ns 41,418,033
BM_CPP_StringByte/100                                           27 ns         16 ns 41,401,382
BM_CPP_StringByte/1000                                          27 ns         16 ns 41,359,841
BM_CPP_StringChar/10                                           348 ns        208 ns  3,374,227
BM_CPP_StringChar/100                                        2,491 ns      1,459 ns    479,078
BM_CPP_StringChar/500                                       11,997 ns      6,820 ns    102,786
BM_CPP_StringChurn/100                                      52,805 ns     29,398 ns     23,705
BM_CPP_StringChurn/1000                                    572,837 ns    302,474 ns      2,282
BM_CPP_StringChurn/500                                     231,629 ns    152,176 ns      4,647
BM_CPP_StringFind/10                                           105 ns         66 ns 10,555,668
BM_CPP_StringFind/100                                          132 ns         66 ns 10,629,430
BM_CPP_StringFind/1000                                         123 ns         70 ns  9,901,078
BM_CPP_StringFind/10000                                        209 ns        130 ns  5,445,641
BM_CPP_StringFindPattern/1000                              359,361 ns    193,176 ns      3,608
BM_CPP_StringFormat/10                                         845 ns        522 ns  1,337,811
BM_CPP_StringFormat/100                                      7,618 ns      5,158 ns    132,433
BM_CPP_StringFormat/500                                     43,386 ns     28,435 ns     24,749
BM_CPP_StringGmatch/1000                                   437,710 ns    226,440 ns      3,092
BM_CPP_StringGsub/10                                           277 ns        184 ns  3,792,558
BM_CPP_StringGsub/100                                        2,760 ns      1,736 ns    407,043
BM_CPP_StringGsub/1000                                      27,888 ns     17,248 ns     40,720
BM_CPP_StringLen/10                                             27 ns         16 ns 41,401,789
BM_CPP_StringLen/100                                            26 ns         16 ns 41,582,886
BM_CPP_StringLen/1000                                           26 ns         16 ns 41,425,791
BM_CPP_StringLen/10000                                          23 ns         16 ns 41,620,113
BM_CPP_StringLower/10                                          306 ns        195 ns  3,601,028
BM_CPP_StringLower/100                                       2,325 ns      1,348 ns    521,413
BM_CPP_StringLower/1000                                     22,828 ns     12,466 ns     56,209
BM_CPP_StringLower/10000                                   195,537 ns    123,952 ns      5,665
BM_CPP_StringRep/10                                            739 ns        420 ns  1,655,571
BM_CPP_StringRep/100                                         5,540 ns      3,120 ns    225,710
BM_CPP_StringRep/1000                                       49,757 ns     30,123 ns     23,272
BM_CPP_StringReverse/10                                        175 ns        103 ns  6,811,517
BM_CPP_StringReverse/100                                       692 ns        450 ns  1,556,938
BM_CPP_StringReverse/1000                                    5,818 ns      3,468 ns    200,200
BM_CPP_StringReverse/10000                                  53,508 ns     34,013 ns     20,679
BM_CPP_StringSub/10                                            151 ns         91 ns  7,642,290
BM_CPP_StringSub/100                                           212 ns        134 ns  5,214,628
BM_CPP_StringSub/1000                                          211 ns        143 ns  4,924,983
BM_CPP_StringSub/10000                                         566 ns        369 ns  1,910,754
BM_CPP_StringUpper/10                                          331 ns        194 ns  3,595,139
BM_CPP_StringUpper/100                                       2,405 ns      1,347 ns    521,995
BM_CPP_StringUpper/1000                                     21,399 ns     12,455 ns     55,977
BM_CPP_StringUpper/10000                                   214,600 ns    123,125 ns      5,676
BM_CPP_Sum/10000                                            87,621 ns     54,921 ns     12,724
BM_CPP_Sum/100000                                          855,607 ns    548,039 ns      1,282
BM_CPP_Sum/1000000                                       8,922,555 ns  5,483,076 ns        128
BM_CPP_Sum/5000000                                      40,471,358 ns 27,449,499 ns         26
BM_CPP_TableChurn/100                                        8,172 ns      5,265 ns    131,011
BM_CPP_TableChurn/1000                                      76,141 ns     52,421 ns     13,634
BM_CPP_TableChurn/500                                       50,410 ns     26,018 ns     26,612
BM_CPP_TableConcat/100                                      12,265 ns      8,491 ns     82,644
BM_CPP_TableConcat/1000                                    124,859 ns     87,820 ns      7,873
BM_CPP_TableConcat/500                                      83,736 ns     43,958 ns     15,941
BM_CPP_TableCreate/1000                                     19,036 ns     11,735 ns     59,861
BM_CPP_TableCreate/3000                                     53,314 ns     34,985 ns     20,129
BM_CPP_TableCreate/5000                                     89,021 ns     58,477 ns     12,039
BM_CPP_TableInsert/100                                       1,047 ns        659 ns  1,075,111
BM_CPP_TableInsert/1000                                     11,124 ns      5,875 ns    120,021
BM_CPP_TableInsert/500                                       5,603 ns      2,984 ns    234,160
BM_CPP_TableInsert/5000                                     44,146 ns     28,972 ns     24,187
BM_CPP_TableMove/100                                         3,080 ns      1,905 ns    366,242
BM_CPP_TableMove/1000                                       25,244 ns     17,827 ns     39,325
BM_CPP_TableMove/500                                        15,080 ns      9,002 ns     77,536
BM_CPP_TableMove/5000                                      144,251 ns     88,188 ns      7,924
BM_CPP_TablePack/1                                              10 ns          5 ns 127,192,129
BM_CPP_TableRemove/100                                       3,206 ns      1,792 ns    389,950
BM_CPP_TableRemove/1000                                     32,342 ns     17,231 ns     40,355
BM_CPP_TableRemove/500                                      14,258 ns      8,737 ns     81,578
BM_CPP_TableRemove/5000                                    167,448 ns     85,880 ns      8,243
BM_CPP_TableSort/100                                         8,862 ns      5,155 ns    137,027
BM_CPP_TableSort/1000                                       87,764 ns     69,572 ns     10,079
BM_CPP_TableSort/500                                        53,222 ns     33,132 ns     21,144
BM_CPP_TailRecursion/100                                     1,049 ns        553 ns  1,267,372
BM_CPP_TailRecursion/1000                                    9,701 ns      5,491 ns    127,230
BM_CPP_TailRecursion/5000                                   45,927 ns     27,491 ns     25,469
BM_CPP_ToNumber/1                                              167 ns         96 ns  7,262,672
BM_CPP_ToString/10                                           1,205 ns        872 ns    802,717
BM_CPP_ToString/100                                         13,072 ns      8,462 ns     84,581
BM_CPP_ToString/500                                         70,978 ns     44,001 ns     16,041
BM_CPP_Variadic/1                                               18 ns         10 ns 64,199,090
BM_CPP_Vector3/10000                                        85,966 ns     52,871 ns     13,291
BM_CPP_Vector3/100000                                      984,296 ns    527,532 ns      1,322
BM_CPP_Vector3/1000000                                  10,423,974 ns  5,285,754 ns        133
BM_FakeLua_BinarySearch_GCC/100                             13,922 ns      8,156 ns     85,400
BM_FakeLua_BinarySearch_GCC/1000                           271,491 ns    156,553 ns      4,320
BM_FakeLua_BinarySearch_GCC/500                            122,929 ns     67,590 ns     10,333
BM_FakeLua_BinarySearch_TCC/100                             81,768 ns     52,935 ns     13,579
BM_FakeLua_BinarySearch_TCC/1000                         1,422,010 ns    874,824 ns        774
BM_FakeLua_BinarySearch_TCC/500                            660,936 ns    373,850 ns      1,911
BM_FakeLua_BubbleSort_GCC/100                              230,707 ns    175,479 ns      3,985
BM_FakeLua_BubbleSort_GCC/200                            1,244,348 ns    698,588 ns      1,007
BM_FakeLua_BubbleSort_GCC/50                                83,555 ns     44,675 ns     15,516
BM_FakeLua_BubbleSort_TCC/100                            1,454,647 ns    836,477 ns        816
BM_FakeLua_BubbleSort_TCC/200                            5,488,115 ns  3,256,166 ns        213
BM_FakeLua_BubbleSort_TCC/50                               293,114 ns    211,591 ns      3,390
BM_FakeLua_Closure_GCC/100                                  19,181 ns     11,046 ns     63,564
BM_FakeLua_Closure_GCC/1000                                211,068 ns    106,248 ns      6,492
BM_FakeLua_Closure_TCC/100                                  36,088 ns     21,576 ns     32,483
BM_FakeLua_Closure_TCC/1000                                354,499 ns    211,904 ns      3,347
BM_FakeLua_EmptyCall_GCC/10000                              56,352 ns     30,976 ns     23,093
BM_FakeLua_EmptyCall_GCC/100000                            557,461 ns    307,805 ns      2,283
BM_FakeLua_EmptyCall_TCC/10000                             367,650 ns    256,049 ns      2,730
BM_FakeLua_EmptyCall_TCC/100000                          5,327,379 ns  2,558,148 ns        272
BM_FakeLua_FastPow_GCC/1234567/7654321/1000000007            1,095 ns        729 ns    957,306
BM_FakeLua_FastPow_GCC/2/1000/1000000007                       919 ns        531 ns  1,320,878
BM_FakeLua_FastPow_GCC/7/1000000/1000000007                  1,008 ns        632 ns  1,110,756
BM_FakeLua_FastPow_TCC/1234567/7654321/1000000007            1,148 ns        818 ns    863,171
BM_FakeLua_FastPow_TCC/2/1000/1000000007                       864 ns        630 ns  1,111,709
BM_FakeLua_FastPow_TCC/7/1000000/1000000007                  1,241 ns        734 ns    956,152
BM_FakeLua_Fibonacci_GCC/20                                 45,729 ns     29,702 ns     24,017
BM_FakeLua_Fibonacci_GCC/25                                385,589 ns    271,393 ns      2,570
BM_FakeLua_Fibonacci_GCC/30                              5,624,615 ns  2,967,464 ns        241
BM_FakeLua_Fibonacci_GCC/32                             12,786,224 ns  7,590,283 ns         93
BM_FakeLua_Fibonacci_TCC/20                                123,444 ns     78,774 ns      9,099
BM_FakeLua_Fibonacci_TCC/25                              1,350,023 ns    855,121 ns        805
BM_FakeLua_Fibonacci_TCC/30                             12,850,953 ns  9,452,837 ns         74
BM_FakeLua_Fibonacci_TCC/32                             38,140,319 ns 24,861,804 ns         28
BM_FakeLua_FloatPoly_GCC/1000000                         4,232,156 ns  2,865,423 ns        245
BM_FakeLua_FloatPoly_TCC/1000000                        22,886,829 ns 13,789,552 ns         50
BM_FakeLua_GCD_GCC/123456789/987654321                         654 ns        423 ns  1,654,415
BM_FakeLua_GCD_GCC/2147483647/1073741823                       620 ns        421 ns  1,665,114
BM_FakeLua_GCD_GCC/832040/514229                               824 ns        520 ns  1,353,393
BM_FakeLua_GCD_TCC/123456789/987654321                         784 ns        490 ns  1,442,922
BM_FakeLua_GCD_TCC/2147483647/1073741823                       817 ns        478 ns  1,444,175
BM_FakeLua_GCD_TCC/832040/514229                               994 ns        615 ns  1,145,840
BM_FakeLua_HashInsert_GCC/100                               39,491 ns     24,481 ns     28,788
BM_FakeLua_HashInsert_GCC/1000                             394,557 ns    253,863 ns      2,777
BM_FakeLua_HashInsert_GCC/500                              209,448 ns    119,708 ns      5,816
BM_FakeLua_HashInsert_TCC/100                               62,986 ns     39,308 ns     17,965
BM_FakeLua_HashInsert_TCC/1000                             709,844 ns    432,322 ns      1,607
BM_FakeLua_HashInsert_TCC/500                              288,573 ns    189,151 ns      3,702
BM_FakeLua_HashLookup_GCC/100                               32,466 ns     22,404 ns     31,590
BM_FakeLua_HashLookup_GCC/1000                             363,678 ns    222,897 ns      3,149
BM_FakeLua_HashLookup_GCC/500                              161,127 ns    111,924 ns      6,177
BM_FakeLua_HashLookup_TCC/100                               50,594 ns     30,941 ns     22,728
BM_FakeLua_HashLookup_TCC/1000                             541,329 ns    316,970 ns      2,207
BM_FakeLua_HashLookup_TCC/500                              271,060 ns    156,821 ns      4,417
BM_FakeLua_InsertionSort_GCC/100                           154,078 ns     89,101 ns      7,854
BM_FakeLua_InsertionSort_GCC/200                           584,821 ns    345,628 ns      2,021
BM_FakeLua_InsertionSort_GCC/50                             44,420 ns     23,795 ns     29,465
BM_FakeLua_InsertionSort_TCC/100                           902,132 ns    532,541 ns      1,398
BM_FakeLua_InsertionSort_TCC/200                         3,914,237 ns  2,087,508 ns        346
BM_FakeLua_InsertionSort_TCC/50                            233,170 ns    139,290 ns      4,924
BM_FakeLua_MatMul_GCC                                        2,165 ns      1,260 ns    545,399
BM_FakeLua_MatMul_TCC                                        6,551 ns      4,132 ns    166,327
BM_FakeLua_MathExpLog_GCC/100000                         5,721,714 ns  4,046,541 ns        174
BM_FakeLua_MathExpLog_TCC/100000                        17,934,875 ns 11,444,546 ns         61
BM_FakeLua_MathMinMax_GCC/100000                         3,704,800 ns  2,159,112 ns        323
BM_FakeLua_MathMinMax_TCC/100000                        19,676,004 ns 11,465,920 ns         62
BM_FakeLua_MathSqrt_GCC/100000                             464,885 ns    275,825 ns      2,537
BM_FakeLua_MathSqrt_TCC/100000                           6,879,179 ns  4,166,763 ns        168
BM_FakeLua_MathTrig_GCC/100000                           5,560,194 ns  3,058,615 ns        229
BM_FakeLua_MathTrig_TCC/100000                          21,645,316 ns 11,484,684 ns         61
BM_FakeLua_MixedAlloc_GCC/100                               62,577 ns     40,070 ns     17,411
BM_FakeLua_MixedAlloc_GCC/1000                             536,218 ns    393,414 ns      1,779
BM_FakeLua_MixedAlloc_GCC/500                              310,651 ns    197,231 ns      3,526
BM_FakeLua_MixedAlloc_TCC/100                              102,126 ns     62,232 ns     11,196
BM_FakeLua_MixedAlloc_TCC/1000                             973,380 ns    628,448 ns      1,111
BM_FakeLua_MixedAlloc_TCC/500                              521,024 ns    314,661 ns      2,229
BM_FakeLua_MultiReturn_GCC/1000                              4,766 ns      2,915 ns    240,764
BM_FakeLua_MultiReturn_GCC/10000                            44,533 ns     25,062 ns     27,754
BM_FakeLua_MultiReturn_TCC/1000                              9,596 ns      5,775 ns    124,053
BM_FakeLua_MultiReturn_TCC/10000                            84,003 ns     52,909 ns     13,405
BM_FakeLua_NestedTable_GCC/1000                            138,830 ns     85,672 ns      8,199
BM_FakeLua_NestedTable_GCC/10000                         1,448,717 ns    845,455 ns        813
BM_FakeLua_NestedTable_TCC/1000                            769,763 ns    444,952 ns      1,569
BM_FakeLua_NestedTable_TCC/10000                         7,183,922 ns  4,455,904 ns        158
BM_FakeLua_Popcount_GCC/1000                                 6,777 ns      4,367 ns    160,915
BM_FakeLua_Popcount_GCC/10000                               89,012 ns     48,663 ns     14,499
BM_FakeLua_Popcount_GCC/100000                           1,016,120 ns    592,552 ns      1,160
BM_FakeLua_Popcount_TCC/1000                                21,598 ns     13,338 ns     52,591
BM_FakeLua_Popcount_TCC/10000                              268,037 ns    153,982 ns      4,337
BM_FakeLua_Popcount_TCC/100000                           2,767,183 ns  1,913,541 ns        375
BM_FakeLua_PowMod_GCC/1234567/7654321/1000000007             1,209 ns        759 ns    928,849
BM_FakeLua_PowMod_GCC/2/1000/1000000007                        830 ns        559 ns  1,262,622
BM_FakeLua_PowMod_GCC/7/1000000/1000000007                     957 ns        666 ns  1,059,056
BM_FakeLua_PowMod_TCC/1234567/7654321/1000000007             1,821 ns      1,084 ns    645,501
BM_FakeLua_PowMod_TCC/2/1000/1000000007                      1,222 ns        720 ns    977,040
BM_FakeLua_PowMod_TCC/7/1000000/1000000007                   1,492 ns        947 ns    733,551
BM_FakeLua_Recursion_GCC/10                                    928 ns        578 ns  1,197,261
BM_FakeLua_Recursion_GCC/20                                 48,722 ns     30,172 ns     23,410
BM_FakeLua_Recursion_GCC/25                                386,863 ns    269,108 ns      2,586
BM_FakeLua_Recursion_TCC/10                                  1,702 ns      1,089 ns    639,131
BM_FakeLua_Recursion_TCC/20                                138,307 ns     79,106 ns      8,834
BM_FakeLua_Recursion_TCC/25                              1,453,790 ns    857,782 ns        801
BM_FakeLua_Sieve_GCC/100                                     6,701 ns      3,979 ns    175,646
BM_FakeLua_Sieve_GCC/1000                                   63,689 ns     35,024 ns     19,847
BM_FakeLua_Sieve_GCC/500                                    29,989 ns     17,088 ns     40,695
BM_FakeLua_Sieve_GCC/5000                                  344,444 ns    213,124 ns      3,359
BM_FakeLua_Sieve_TCC/100                                    29,560 ns     17,487 ns     39,646
BM_FakeLua_Sieve_TCC/1000                                  263,604 ns    172,742 ns      4,041
BM_FakeLua_Sieve_TCC/500                                   143,477 ns     85,340 ns      8,216
BM_FakeLua_Sieve_TCC/5000                                1,780,537 ns  1,041,591 ns        679
BM_FakeLua_StringByte_GCC/10                                   939 ns        531 ns  1,304,736
BM_FakeLua_StringByte_GCC/100                                  951 ns        576 ns  1,226,213
BM_FakeLua_StringByte_GCC/1000                               1,003 ns        603 ns  1,162,886
BM_FakeLua_StringByte_TCC/10                                   983 ns        593 ns  1,206,309
BM_FakeLua_StringByte_TCC/100                                1,000 ns        638 ns  1,076,212
BM_FakeLua_StringByte_TCC/1000                               1,140 ns        666 ns  1,037,743
BM_FakeLua_StringChar_GCC/10                                 4,717 ns      2,785 ns    252,981
BM_FakeLua_StringChar_GCC/100                               26,150 ns     16,053 ns     43,522
BM_FakeLua_StringChar_GCC/500                              124,214 ns     72,588 ns      9,375
BM_FakeLua_StringChar_TCC/10                                 7,145 ns      4,461 ns    157,332
BM_FakeLua_StringChar_TCC/100                               56,569 ns     31,568 ns     22,304
BM_FakeLua_StringChar_TCC/500                              214,876 ns    144,492 ns      4,830
BM_FakeLua_StringChurn_GCC/100                              65,339 ns     44,656 ns     15,766
BM_FakeLua_StringChurn_GCC/1000                            826,591 ns    443,901 ns      1,576
BM_FakeLua_StringChurn_GCC/500                             365,738 ns    223,272 ns      3,136
BM_FakeLua_StringChurn_TCC/100                              90,760 ns     57,091 ns     12,523
BM_FakeLua_StringChurn_TCC/1000                          1,114,236 ns    571,149 ns      1,237
BM_FakeLua_StringChurn_TCC/500                             513,027 ns    283,598 ns      2,459
BM_FakeLua_StringFindPattern_GCC/1000                   11,580,621 ns  7,654,594 ns         91
BM_FakeLua_StringFindPattern_TCC/1000                   13,050,656 ns  7,914,296 ns         88
BM_FakeLua_StringFind_GCC/10                                 2,628 ns      1,647 ns    424,630
BM_FakeLua_StringFind_GCC/100                                2,637 ns      1,654 ns    421,490
BM_FakeLua_StringFind_GCC/1000                               2,950 ns      1,723 ns    410,605
BM_FakeLua_StringFind_GCC/10000                              3,955 ns      2,502 ns    276,271
BM_FakeLua_StringFind_TCC/10                                 2,650 ns      1,713 ns    409,465
BM_FakeLua_StringFind_TCC/100                                2,782 ns      1,721 ns    404,834
BM_FakeLua_StringFind_TCC/1000                               2,985 ns      1,771 ns    394,892
BM_FakeLua_StringFind_TCC/10000                              3,996 ns      2,588 ns    272,588
BM_FakeLua_StringFormat_GCC/10                               3,354 ns      1,954 ns    356,680
BM_FakeLua_StringFormat_GCC/100                             26,095 ns     15,567 ns     44,584
BM_FakeLua_StringFormat_GCC/500                            119,442 ns     77,076 ns      8,998
BM_FakeLua_StringFormat_TCC/10                               3,333 ns      2,324 ns    299,598
BM_FakeLua_StringFormat_TCC/100                             29,993 ns     18,834 ns     36,855
BM_FakeLua_StringFormat_TCC/500                            160,453 ns     93,895 ns      7,480
BM_FakeLua_StringGmatch_GCC/1000                         8,819,264 ns  5,272,718 ns        130
BM_FakeLua_StringGmatch_TCC/1000                         9,236,472 ns  5,559,361 ns        126
BM_FakeLua_StringGsub_GCC/10                                13,205 ns      8,969 ns     78,419
BM_FakeLua_StringGsub_GCC/100                              103,458 ns     68,161 ns     10,359
BM_FakeLua_StringGsub_GCC/1000                           1,101,351 ns    657,675 ns      1,070
BM_FakeLua_StringGsub_TCC/10                                13,795 ns      8,996 ns     77,772
BM_FakeLua_StringGsub_TCC/100                              114,274 ns     67,941 ns     10,349
BM_FakeLua_StringGsub_TCC/1000                           1,114,296 ns    658,608 ns      1,073
BM_FakeLua_StringLen_GCC/10                                    818 ns        492 ns  1,432,371
BM_FakeLua_StringLen_GCC/100                                   796 ns        540 ns  1,299,835
BM_FakeLua_StringLen_GCC/1000                                  866 ns        561 ns  1,246,589
BM_FakeLua_StringLen_GCC/10000                               1,914 ns      1,077 ns    647,127
BM_FakeLua_StringLen_TCC/10                                    877 ns        535 ns  1,273,369
BM_FakeLua_StringLen_TCC/100                                 1,032 ns        585 ns  1,189,078
BM_FakeLua_StringLen_TCC/1000                                  901 ns        603 ns  1,162,520
BM_FakeLua_StringLen_TCC/10000                               1,808 ns      1,112 ns    601,399
BM_FakeLua_StringLower_GCC/10                                1,048 ns        600 ns  1,152,148
BM_FakeLua_StringLower_GCC/100                               1,393 ns        702 ns  1,001,106
BM_FakeLua_StringLower_GCC/1000                              1,551 ns        846 ns    816,183
BM_FakeLua_StringLower_GCC/10000                             5,043 ns      2,822 ns    251,014
BM_FakeLua_StringLower_TCC/10                                1,073 ns        692 ns  1,016,288
BM_FakeLua_StringLower_TCC/100                               1,855 ns      1,212 ns    583,384
BM_FakeLua_StringLower_TCC/1000                              8,751 ns      5,402 ns    128,947
BM_FakeLua_StringLower_TCC/10000                            86,072 ns     48,207 ns     14,542
BM_FakeLua_StringRep_GCC/10                                  1,033 ns        671 ns  1,049,153
BM_FakeLua_StringRep_GCC/100                                 1,806 ns      1,100 ns    630,280
BM_FakeLua_StringRep_GCC/1000                                8,126 ns      4,888 ns    141,355
BM_FakeLua_StringRep_TCC/10                                  1,328 ns        752 ns    927,065
BM_FakeLua_StringRep_TCC/100                                 2,358 ns      1,389 ns    505,237
BM_FakeLua_StringRep_TCC/1000                               10,189 ns      7,129 ns     98,349
BM_FakeLua_StringReverse_GCC/10                              1,109 ns        604 ns  1,148,807
BM_FakeLua_StringReverse_GCC/100                             1,397 ns        765 ns    919,546
BM_FakeLua_StringReverse_GCC/1000                            2,668 ns      1,438 ns    477,188
BM_FakeLua_StringReverse_GCC/10000                          14,935 ns      8,698 ns     78,927
BM_FakeLua_StringReverse_TCC/10                                925 ns        681 ns  1,037,089
BM_FakeLua_StringReverse_TCC/100                             1,870 ns      1,066 ns    657,331
BM_FakeLua_StringReverse_TCC/1000                            6,110 ns      4,063 ns    172,547
BM_FakeLua_StringReverse_TCC/10000                          52,709 ns     34,603 ns     20,307
BM_FakeLua_StringSub_GCC/10                                  2,264 ns      1,345 ns    518,241
BM_FakeLua_StringSub_GCC/100                                 2,557 ns      1,430 ns    489,733
BM_FakeLua_StringSub_GCC/1000                                2,763 ns      1,490 ns    466,344
BM_FakeLua_StringSub_GCC/10000                               4,070 ns      2,268 ns    306,435
BM_FakeLua_StringSub_TCC/10                                  2,372 ns      1,390 ns    508,219
BM_FakeLua_StringSub_TCC/100                                 2,314 ns      1,485 ns    474,333
BM_FakeLua_StringSub_TCC/1000                                3,228 ns      1,543 ns    458,387
BM_FakeLua_StringSub_TCC/10000                               3,832 ns      2,334 ns    301,764
BM_FakeLua_StringUpper_GCC/10                                1,127 ns        601 ns  1,168,948
BM_FakeLua_StringUpper_GCC/100                               1,114 ns        702 ns    992,199
BM_FakeLua_StringUpper_GCC/1000                              1,386 ns        850 ns    820,082
BM_FakeLua_StringUpper_GCC/10000                             5,104 ns      2,832 ns    248,824
BM_FakeLua_StringUpper_TCC/10                                1,169 ns        696 ns  1,006,148
BM_FakeLua_StringUpper_TCC/100                               2,031 ns      1,214 ns    575,272
BM_FakeLua_StringUpper_TCC/1000                             10,406 ns      5,421 ns    129,932
BM_FakeLua_StringUpper_TCC/10000                            81,506 ns     48,151 ns     14,489
BM_FakeLua_Sum_GCC/10000                                     6,452 ns      3,508 ns    199,582
BM_FakeLua_Sum_GCC/100000                                   50,566 ns     31,381 ns     22,327
BM_FakeLua_Sum_GCC/1000000                                 578,521 ns    309,437 ns      2,250
BM_FakeLua_Sum_GCC/5000000                               3,098,218 ns  1,547,859 ns        453
BM_FakeLua_Sum_TCC/10000                                    37,857 ns     24,934 ns     27,821
BM_FakeLua_Sum_TCC/100000                                  417,183 ns    246,397 ns      2,850
BM_FakeLua_Sum_TCC/1000000                               4,479,399 ns  2,452,786 ns        283
BM_FakeLua_Sum_TCC/5000000                              18,728,703 ns 12,321,859 ns         56
BM_FakeLua_TableChurn_GCC/100                               11,409 ns      6,971 ns    100,756
BM_FakeLua_TableChurn_GCC/1000                             119,994 ns     65,155 ns     10,754
BM_FakeLua_TableChurn_GCC/500                               43,896 ns     33,036 ns     21,217
BM_FakeLua_TableChurn_TCC/100                               57,082 ns     36,778 ns     19,044
BM_FakeLua_TableChurn_TCC/1000                             466,370 ns    354,670 ns      1,972
BM_FakeLua_TableChurn_TCC/500                              281,725 ns    182,147 ns      3,900
BM_FakeLua_TableConcat_GCC/100                              58,710 ns     38,108 ns     18,314
BM_FakeLua_TableConcat_GCC/1000                            708,780 ns    365,628 ns      1,886
BM_FakeLua_TableConcat_GCC/500                             309,868 ns    185,102 ns      3,781
BM_FakeLua_TableConcat_TCC/100                             101,629 ns     54,515 ns     13,123
BM_FakeLua_TableConcat_TCC/1000                            914,385 ns    523,737 ns      1,328
BM_FakeLua_TableConcat_TCC/500                             487,979 ns    262,511 ns      2,665
BM_FakeLua_TableCreate_GCC/1000                             33,890 ns     20,427 ns     34,468
BM_FakeLua_TableCreate_GCC/3000                            117,469 ns     69,451 ns     10,059
BM_FakeLua_TableCreate_GCC/5000                            251,652 ns    128,429 ns      5,479
BM_FakeLua_TableCreate_TCC/1000                            193,548 ns    117,177 ns      6,020
BM_FakeLua_TableCreate_TCC/3000                            605,946 ns    392,114 ns      1,794
BM_FakeLua_TableCreate_TCC/5000                          1,235,125 ns    705,496 ns        988
BM_FakeLua_TableInsert_GCC/100                               5,796 ns      3,230 ns    217,272
BM_FakeLua_TableInsert_GCC/1000                             37,577 ns     23,234 ns     30,067
BM_FakeLua_TableInsert_GCC/500                              18,496 ns     12,007 ns     57,879
BM_FakeLua_TableInsert_GCC/5000                            230,109 ns    141,009 ns      4,927
BM_FakeLua_TableInsert_TCC/100                              25,990 ns     14,008 ns     51,081
BM_FakeLua_TableInsert_TCC/1000                            190,463 ns    121,881 ns      5,882
BM_FakeLua_TableInsert_TCC/500                             122,192 ns     62,347 ns     11,427
BM_FakeLua_TableInsert_TCC/5000                          1,128,122 ns    726,417 ns        929
BM_FakeLua_TableMove_GCC/100                                 8,264 ns      5,510 ns    123,002
BM_FakeLua_TableMove_GCC/1000                               65,516 ns     40,768 ns     17,565
BM_FakeLua_TableMove_GCC/500                                31,437 ns     20,959 ns     33,710
BM_FakeLua_TableMove_GCC/5000                              415,416 ns    250,364 ns      2,806
BM_FakeLua_TableMove_TCC/100                                40,035 ns     23,025 ns     30,393
BM_FakeLua_TableMove_TCC/1000                              291,431 ns    196,812 ns      3,556
BM_FakeLua_TableMove_TCC/500                               148,661 ns     99,540 ns      7,076
BM_FakeLua_TableMove_TCC/5000                            1,689,098 ns  1,175,769 ns        595
BM_FakeLua_TablePack_GCC/1                                   1,240 ns        784 ns    894,882
BM_FakeLua_TablePack_TCC/1                                   3,413 ns      1,830 ns    383,737
BM_FakeLua_TableRemove_GCC/100                              10,904 ns      5,496 ns    128,245
BM_FakeLua_TableRemove_GCC/1000                             84,466 ns     46,212 ns     15,196
BM_FakeLua_TableRemove_GCC/500                              37,355 ns     23,386 ns     28,737
BM_FakeLua_TableRemove_GCC/5000                            430,685 ns    256,491 ns      2,710
BM_FakeLua_TableRemove_TCC/100                              41,184 ns     23,119 ns     30,006
BM_FakeLua_TableRemove_TCC/1000                            419,694 ns    215,409 ns      3,251
BM_FakeLua_TableRemove_TCC/500                             177,077 ns    107,216 ns      6,561
BM_FakeLua_TableRemove_TCC/5000                          1,793,838 ns  1,185,962 ns        589
BM_FakeLua_TableSort_GCC/100                                44,298 ns     28,004 ns     25,024
BM_FakeLua_TableSort_GCC/1000                              562,565 ns    330,027 ns      2,138
BM_FakeLua_TableSort_GCC/500                               271,424 ns    154,615 ns      4,519
BM_FakeLua_TableSort_TCC/100                                66,261 ns     39,107 ns     17,856
BM_FakeLua_TableSort_TCC/1000                              730,477 ns    431,083 ns      1,617
BM_FakeLua_TableSort_TCC/500                               306,301 ns    206,069 ns      3,371
BM_FakeLua_TailRecursion_GCC/100                               777 ns        426 ns  1,648,769
BM_FakeLua_TailRecursion_GCC/1000                            1,205 ns        712 ns    979,895
BM_FakeLua_TailRecursion_GCC/5000                            3,796 ns      1,951 ns    358,052
BM_FakeLua_TailRecursion_TCC/100                             1,584 ns        927 ns    759,379
BM_FakeLua_TailRecursion_TCC/1000                            9,233 ns      5,203 ns    100,000
BM_FakeLua_TailRecursion_TCC/5000                           42,186 ns     24,371 ns     29,042
BM_FakeLua_ToNumber_GCC/1                                      802 ns        542 ns  1,304,733
BM_FakeLua_ToNumber_TCC/1                                    1,150 ns        693 ns  1,016,080
BM_FakeLua_ToString_GCC/10                                   1,194 ns        647 ns  1,097,100
BM_FakeLua_ToString_GCC/100                                    979 ns        646 ns  1,079,352
BM_FakeLua_ToString_GCC/500                                  1,059 ns        650 ns  1,080,277
BM_FakeLua_ToString_TCC/10                                   1,174 ns        701 ns  1,016,080
BM_FakeLua_ToString_TCC/100                                  1,203 ns        700 ns    987,485
BM_FakeLua_ToString_TCC/500                                    959 ns        701 ns    999,183
BM_FakeLua_Variadic_GCC/1                                    1,215 ns        754 ns    928,022
BM_FakeLua_Variadic_TCC/1                                    2,233 ns      1,224 ns    575,243
BM_FakeLua_Vector3_GCC/10000                               607,809 ns    309,974 ns      2,261
BM_FakeLua_Vector3_GCC/100000                            4,839,630 ns  3,056,228 ns        228
BM_FakeLua_Vector3_GCC/1000000                          47,773,627 ns 30,897,897 ns         22
BM_FakeLua_Vector3_TCC/10000                             3,344,400 ns  2,091,722 ns        331
BM_FakeLua_Vector3_TCC/100000                           42,147,447 ns 21,209,412 ns         33
BM_FakeLua_Vector3_TCC/1000000                          347,663,874 ns 210,547,754 ns          3
BM_Lua_BinarySearch/100                                     60,634 ns     37,029 ns     18,896
BM_Lua_BinarySearch/1000                                   959,697 ns    580,933 ns      1,221
BM_Lua_BinarySearch/500                                    440,300 ns    254,492 ns      2,758
BM_Lua_BubbleSort/100                                      594,801 ns    338,786 ns      2,093
BM_Lua_BubbleSort/200                                    2,086,556 ns  1,328,612 ns        534
BM_Lua_BubbleSort/50                                       164,153 ns     85,624 ns      8,213
BM_Lua_Closure/100                                          39,957 ns     22,696 ns     30,670
BM_Lua_Closure/1000                                        348,764 ns    226,293 ns      3,086
BM_Lua_EmptyCall/10000                                     722,370 ns    439,736 ns      1,597
BM_Lua_EmptyCall/100000                                  7,933,740 ns  4,354,814 ns        159
BM_Lua_FastPow/1234567/7654321/1000000007                    1,930 ns      1,152 ns    594,367
BM_Lua_FastPow/2/1000/1000000007                             1,039 ns        593 ns  1,199,681
BM_Lua_FastPow/7/1000000/1000000007                          1,725 ns        974 ns    722,218
BM_Lua_Fibonacci/20                                      1,569,773 ns    860,801 ns        824
BM_Lua_Fibonacci/25                                     15,600,871 ns  9,626,976 ns         72
BM_Lua_Fibonacci/30                                     168,261,430 ns 106,484,626 ns          7
BM_Lua_Fibonacci/32                                     503,047,483 ns 277,915,868 ns          3
BM_Lua_FloatPoly/1000000                                195,928,101 ns 99,944,530 ns          7
BM_Lua_GCD/123456789/987654321                                 337 ns        212 ns  3,268,053
BM_Lua_GCD/2147483647/1073741823                               370 ns        191 ns  3,676,078
BM_Lua_GCD/832040/514229                                     1,132 ns        730 ns    942,430
BM_Lua_HashInsert/100                                       87,286 ns     48,607 ns     14,714
BM_Lua_HashInsert/1000                                     786,406 ns    499,118 ns      1,378
BM_Lua_HashInsert/500                                      378,706 ns    241,489 ns      2,867
BM_Lua_HashLookup/100                                       68,701 ns     39,114 ns     17,775
BM_Lua_HashLookup/1000                                     649,131 ns    414,686 ns      1,713
BM_Lua_HashLookup/500                                      315,722 ns    205,240 ns      3,408
BM_Lua_InsertionSort/100                                   389,213 ns    228,283 ns      3,094
BM_Lua_InsertionSort/200                                 1,452,926 ns    894,653 ns        782
BM_Lua_InsertionSort/50                                     92,414 ns     59,708 ns     11,653
BM_Lua_MatMul                                                5,810 ns      3,698 ns    190,328
BM_Lua_MathExpLog/100000                                30,397,983 ns 17,460,595 ns         41
BM_Lua_MathMinMax/100000                                36,855,906 ns 22,431,932 ns         31
BM_Lua_MathSqrt/100000                                  11,202,131 ns  6,162,470 ns        112
BM_Lua_MathTrig/100000                                  28,341,267 ns 17,880,549 ns         39
BM_Lua_MixedAlloc/100                                      148,240 ns     87,270 ns      8,030
BM_Lua_MixedAlloc/1000                                   1,718,443 ns    897,348 ns        810
BM_Lua_MixedAlloc/500                                      703,958 ns    442,235 ns      1,559
BM_Lua_MultiReturn/1000                                     96,520 ns     59,128 ns     11,867
BM_Lua_MultiReturn/10000                                   926,960 ns    582,937 ns      1,229
BM_Lua_NestedTable/1000                                    466,508 ns    292,829 ns      2,366
BM_Lua_NestedTable/10000                                 5,063,234 ns  2,900,521 ns        240
BM_Lua_Popcount/1000                                       240,702 ns    144,200 ns      4,801
BM_Lua_Popcount/10000                                    2,783,275 ns  1,828,025 ns        382
BM_Lua_Popcount/100000                                  35,208,870 ns 22,105,500 ns         32
BM_Lua_PowMod/1234567/7654321/1000000007                     2,090 ns      1,299 ns    544,422
BM_Lua_PowMod/2/1000/1000000007                              1,028 ns        661 ns  1,072,320
BM_Lua_PowMod/7/1000000/1000000007                           2,075 ns      1,100 ns    631,710
BM_Lua_Recursion/10                                         13,957 ns      8,364 ns     81,503
BM_Lua_Recursion/20                                      1,412,171 ns  1,043,444 ns        689
BM_Lua_Recursion/25                                     17,663,822 ns 11,536,223 ns         61
BM_Lua_Sieve/100                                            15,105 ns      9,868 ns     70,790
BM_Lua_Sieve/1000                                          131,859 ns     75,406 ns      9,411
BM_Lua_Sieve/500                                            59,479 ns     39,012 ns     17,749
BM_Lua_Sieve/5000                                          537,121 ns    378,408 ns      1,865
BM_Lua_StringByte/10                                           498 ns        303 ns  2,260,816
BM_Lua_StringByte/100                                          760 ns        460 ns  1,530,782
BM_Lua_StringByte/1000                                         927 ns        624 ns  1,147,141
BM_Lua_StringChar/10                                         4,724 ns      3,004 ns    233,911
BM_Lua_StringChar/100                                       26,038 ns     17,295 ns     40,665
BM_Lua_StringChar/500                                      115,847 ns     75,332 ns      9,010
BM_Lua_StringChurn/100                                     124,238 ns     72,452 ns      9,398
BM_Lua_StringChurn/1000                                  1,713,625 ns    971,816 ns        707
BM_Lua_StringChurn/500                                     726,842 ns    474,138 ns      1,477
BM_Lua_StringFind/10                                           816 ns        457 ns  1,510,131
BM_Lua_StringFind/100                                        1,002 ns        571 ns  1,225,626
BM_Lua_StringFind/1000                                       1,076 ns        756 ns    933,030
BM_Lua_StringFind/10000                                      3,359 ns      2,139 ns    327,326
BM_Lua_StringFindPattern/1000                            1,536,606 ns    938,600 ns        753
BM_Lua_StringFormat/10                                       4,810 ns      2,816 ns    249,862
BM_Lua_StringFormat/100                                     45,648 ns     28,036 ns     25,220
BM_Lua_StringFormat/500                                    217,375 ns    138,971 ns      5,005
BM_Lua_StringGmatch/1000                                 1,797,588 ns  1,101,192 ns        634
BM_Lua_StringGsub/10                                         1,551 ns        942 ns    743,199
BM_Lua_StringGsub/100                                        7,349 ns      4,529 ns    155,040
BM_Lua_StringGsub/1000                                      67,386 ns     37,743 ns     18,523
BM_Lua_StringLen/10                                            255 ns        169 ns  4,208,529
BM_Lua_StringLen/100                                           512 ns        317 ns  2,210,217
BM_Lua_StringLen/1000                                          625 ns        491 ns  1,417,701
BM_Lua_StringLen/10000                                       2,719 ns      1,979 ns    358,597
BM_Lua_StringLower/10                                          596 ns        322 ns  2,189,914
BM_Lua_StringLower/100                                       1,225 ns        747 ns    942,770
BM_Lua_StringLower/1000                                      3,564 ns      2,181 ns    320,649
BM_Lua_StringLower/10000                                    29,966 ns     17,111 ns     41,238
BM_Lua_StringRep/10                                            737 ns        408 ns  1,705,699
BM_Lua_StringRep/100                                         1,822 ns        997 ns    694,622
BM_Lua_StringRep/1000                                        9,142 ns      5,390 ns    126,264
BM_Lua_StringReverse/10                                        597 ns        307 ns  2,261,963
BM_Lua_StringReverse/100                                     1,182 ns        706 ns    961,826
BM_Lua_StringReverse/1000                                    3,323 ns      1,913 ns    365,656
BM_Lua_StringReverse/10000                                  24,703 ns     14,399 ns     48,160
BM_Lua_StringSub/10                                            617 ns        364 ns  1,942,516
BM_Lua_StringSub/100                                         1,105 ns        594 ns  1,177,842
BM_Lua_StringSub/1000                                        1,379 ns        906 ns    766,324
BM_Lua_StringSub/10000                                       4,908 ns      3,045 ns    231,995
BM_Lua_StringUpper/10                                          567 ns        323 ns  2,190,345
BM_Lua_StringUpper/100                                       1,342 ns        735 ns    951,815
BM_Lua_StringUpper/1000                                      3,109 ns      1,965 ns    355,243
BM_Lua_StringUpper/10000                                    27,760 ns     14,242 ns     48,029
BM_Lua_Sum/10000                                           180,892 ns     92,068 ns      7,391
BM_Lua_Sum/100000                                        1,380,171 ns    911,977 ns        771
BM_Lua_Sum/1000000                                      16,461,078 ns  9,233,507 ns         76
BM_Lua_Sum/5000000                                      74,960,840 ns 46,977,732 ns         15
BM_Lua_TableChurn/100                                       88,809 ns     61,258 ns     11,328
BM_Lua_TableChurn/1000                                     820,002 ns    602,970 ns      1,148
BM_Lua_TableChurn/500                                      477,274 ns    305,247 ns      2,384
BM_Lua_TableConcat/100                                      71,908 ns     36,736 ns     19,340
BM_Lua_TableConcat/1000                                    585,304 ns    361,357 ns      1,936
BM_Lua_TableConcat/500                                     325,897 ns    177,956 ns      3,874
BM_Lua_TableCreate/1000                                     47,111 ns     29,524 ns     23,831
BM_Lua_TableCreate/3000                                    138,745 ns     86,404 ns      8,046
BM_Lua_TableCreate/5000                                    257,284 ns    145,201 ns      4,849
BM_Lua_TableInsert/100                                      23,857 ns     13,362 ns     53,418
BM_Lua_TableInsert/1000                                    208,845 ns    116,643 ns      5,895
BM_Lua_TableInsert/500                                     107,318 ns     57,789 ns     11,379
BM_Lua_TableInsert/5000                                    928,408 ns    564,608 ns      1,170
BM_Lua_TableMove/100                                        12,817 ns      8,743 ns     81,064
BM_Lua_TableMove/1000                                       92,904 ns     50,879 ns     10,000
BM_Lua_TableMove/500                                        44,285 ns     28,002 ns     25,009
BM_Lua_TableMove/5000                                      355,241 ns    250,016 ns      2,787
BM_Lua_TablePack/1                                           2,840 ns      1,516 ns    462,846
BM_Lua_TableRemove/100                                      24,388 ns     17,343 ns     40,851
BM_Lua_TableRemove/1000                                    261,457 ns    152,441 ns      4,709
BM_Lua_TableRemove/500                                     126,597 ns     76,961 ns      9,300
BM_Lua_TableRemove/5000                                  1,354,665 ns    828,342 ns        831
BM_Lua_TableSort/100                                        42,572 ns     30,042 ns     23,123
BM_Lua_TableSort/1000                                      714,671 ns    398,060 ns      1,749
BM_Lua_TableSort/500                                       289,863 ns    184,677 ns      3,803
BM_Lua_TailRecursion/100                                     8,300 ns      4,805 ns    160,595
BM_Lua_TailRecursion/1000                                   73,087 ns     42,646 ns     15,958
BM_Lua_TailRecursion/5000                                  346,152 ns    209,783 ns      3,406
BM_Lua_ToNumber/1                                              529 ns        285 ns  2,447,960
BM_Lua_ToString/10                                             903 ns        478 ns  1,479,778
BM_Lua_ToString/100                                            922 ns        481 ns  1,460,211
BM_Lua_ToString/500                                            739 ns        477 ns  1,451,251
BM_Lua_Variadic/1                                            1,069 ns        685 ns  1,026,991
BM_Lua_Vector3/10000                                     2,302,920 ns  1,493,404 ns        464
BM_Lua_Vector3/100000                                   24,833,907 ns 14,947,118 ns         46
BM_Lua_Vector3/1000000                                  239,352,621 ns 149,734,385 ns          5
```
