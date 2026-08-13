# Benchmark Results

本文件记录在本地以 **Release 模式**（`-O3 -DNDEBUG`）编译运行 `bench_mark` 的完整结果。覆盖 **6 大类共 51 个 Lua 性能场景**，每个场景均实现 C++ / Lua 5.4 / FakeLua GCC 三种横向对比（原始输出含 TCC，本分析聚焦 GCC vs Lua、GCC vs C++）。

## 运行环境

- 日期：2026-08-12
- 机器：AMD EPYC 7K62 48-Core Processor，2 X 2595.12 MHz CPU s
- CPU 缓存：L1d 32 KiB (x2)，L1i 32 KiB (x2)，L2 4096 KiB (x2)，L3 16384 KiB (x1)
- 构建模式：**Release**（`-O3 -DNDEBUG`），GCC 15.1.0
- FakeLua GCC JIT：**Release 模式**（`debug_mode=false`，GCC `-O3` 优化）
- 二进制：`build/bin/bench_mark`

## 运行命令

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target bench_mark --parallel
build/bin/bench_mark --benchmark_repetitions=1 --benchmark_report_aggregates_only=true
```

---

## 结论

下表取各场景最大参数（最具代表性），**GCC vs Lua** 列表示 FakeLua GCC 相对 Lua 5.4 的加速倍数（>1 表示更快），**GCC vs C++** 列表示 FakeLua GCC 与手写 C++ 的比值（<1 表示 GCC 更快）。

### 算法（algo）

| 场景 | 参数 | GCC vs Lua | GCC vs C++ | 备注 |
|------|------|-----------|-----------|------|
| Fibonacci | n=32 | **36.4x** | 0.13 | 数值特化，接近 C++ |
| GCD | 2147483647/1073741823 | 0.43x | 11.6x | 小循环受调用开销主导 |
| PowMod | 1234567/7654321/1e9+7 | 1.8x | 2.6x | |
| Sum | n=5M | **29.0x** | 0.06 | GCC 向量化，远快于 C++ |
| BubbleSort | n=200 | 2.1x | 2.5x | 表操作拖累 |
| Sieve | n=5000 | 1.9x | 2.5x | |
| BinarySearch | n=1000 | 3.9x | 2.4x | |
| FastPow | 1234567/7654321/1e9+7 | 1.5x | 2.5x | |
| Popcount | n=100K | **37.3x** | 0.12 | 位运算极致优化 |
| InsertionSort | n=200 | 2.6x | 2.9x | |
| MatMul | 3×3 | 3.3x | 4.4x | 动态索引走 spec_get |
| Vector3 | n=1M | 4.9x | 5.7x | 表特化为结构体，指针偏移 |
| **FloatPoly** (新) | n=1M | **34.7x** | **0.49** | 浮点特化，GCC 2x 快于 C++ |

### 字符串（string）

| 场景 | 参数 | GCC vs Lua | GCC vs C++ | 备注 |
|------|------|-----------|-----------|------|
| StringLen | n=10K | 1.8x | 66.7x | 调用开销主导 |
| StringSub | n=10K | 1.4x | 6.5x | |
| StringRep | n=1000 | 0.17x | 1.05x | Lua 原生 C 实现更优 |
| StringReverse | n=10K | 0.39x | 1.06x | |
| StringLower | n=10K | 0.26x | 0.53x | |
| StringUpper | n=10K | 0.22x | 0.54x | |
| StringByte | n=1000 | 0.47x | 74.8x | |
| StringChar | n=500 | 0.04x | 306x | 脚本层逐字符拼接 |
| StringFormat | n=500 | 0.29x | 17.3x | 格式解析在脚本层 |
| StringFind | n=10K | 0.85x | 20.2x | |
| StringGsub | n=1000 | 0.09x | 38.9x | 走 `std::regex`，本例仅替换字面量 `"a"` |
| ToNumber | n=1 | 0.18x | 15.7x | |
| ToString | n=500 | 0.47x | 0.02x | |
| **StringFindPattern** † | n=1000 | **0.006x** | 1111x | 正则匹配，GCC 比 Lua 慢 **169x** |
| **StringGmatch** † | n=1000 | **0.011x** | 694x | 迭代器 + 正则，GCC 比 Lua 慢 **94x** |

> **† 修正说明**：这两个场景此前记录的 0.18x / 0.21x 是**无效数据**。它们的脚本使用 `string.find(s, "%d+")` 这种 Lua pattern 写法，而 fakelua 的 `string` 匹配函数走的是 ECMAScript 正则——`%d+` 在正则里表示「字面量 `%` 后跟一个或多个 `d`」，在测试串 `"abc123def456ghi789"` 中永远匹配不到。结果是 Lua 每轮找到 3 个匹配，fakelua 一个都找不到、立即跳出循环，两边做的工作量根本不对等（而且 fakelua 在只做 1/4 工作量的情况下依然更慢）。
>
> 脚本已改用 `[0-9]+`（在 Lua pattern 与 ECMAScript 正则中语义一致），并为 Lua / TCC / GCC 三个变体补上了 `VerifyEqual` 校验，确保三者返回值相同、工作量对等。上表两行为修正后重新测得。
>
> 注意：这两行的绝对倍数在**另一台机器**上测得（4 × 2400 MHz，L3 328 MB），与本文件其余数据的运行环境不同，仅倍数关系可比。原始数据：
>
> ```text
> BM_CPP_StringFindPattern/1000            46581 ns
> BM_Lua_StringFindPattern/1000           306955 ns
> BM_FakeLua_StringFindPattern_TCC/1000 51900084 ns
> BM_FakeLua_StringFindPattern_GCC/1000 51738697 ns
> BM_CPP_StringGmatch/1000                 55571 ns
> BM_Lua_StringGmatch/1000                411230 ns
> BM_FakeLua_StringGmatch_TCC/1000      38922642 ns
> BM_FakeLua_StringGmatch_GCC/1000      38596136 ns
> ```

### 表操作（table）

| 场景 | 参数 | GCC vs Lua | GCC vs C++ | 备注 |
|------|------|-----------|-----------|------|
| TableInsert | n=5K | 0.02x | 1019x | 严重瓶颈 |
| TableRemove | n=5K | 0.03x | 348x | |
| TableConcat | n=1000 | 0.05x | 76.5x | |
| TablePack | n=1 | 2.1x | 127x | |
| TableMove | n=5K | ≈0x | 1608x | 最慢表操作 |
| TableSort | n=1000 | 0.03x | 168x | 排序在脚本层 |
| TableCreate | n=5K | 1.1x | 2.16x | |
| HashInsert | n=1000 | 0.88x | 1.99x | |
| HashLookup | n=1000 | 0.76x | 1.11x | |
| NestedTable | n=10K | 3.3x | 1.42x | 表层指针偏移遍历 |

### 函数调用（function）

| 场景 | 参数 | GCC vs Lua | GCC vs C++ | 备注 |
|------|------|-----------|-----------|------|
| EmptyCall | n=100K | **13.0x** | 55422x | C++ 内联为 0 |
| Recursion | n=25 | **44.0x** | 0.13 | 数值特化 |
| Variadic | n=5 | 0.12x | 536x | vararg 构建开销大 |
| MultiReturn | n=10K | **24.1x** | 0.45 | |
| Closure | n=1000 | 3.9x | 10.9x | |
| TailRecursion | n=5K | **106.8x** | 0.07 | 尾调用转循环+向量化 |

### GC 与内存压力（gc）

| 场景 | 参数 | GCC vs Lua | GCC vs C++ | 备注 |
|------|------|-----------|-----------|------|
| TableChurn | n=1000 | **13.9x** | 0.85 | 内存池分配器优势 |
| StringChurn | n=1000 | 0.76x | 4.19x | 字符串分配是弱项 |
| MixedAlloc | n=1000 | 1.2x | 3.22x | |

### 数学函数（math，新增）

| 场景 | 参数 | GCC vs Lua | GCC vs C++ | 备注 |
|------|------|-----------|-----------|------|
| MathTrig (sin+cos) | n=100K | **5.9x** | 0.87 | 接近 C++ 原生速度 |
| **MathSqrt** | n=100K | **22.7x** | **0.49** | GCC 2x 快于 C++ |
| **MathExpLog** (新) | n=100K | **0.14x** | **28.1x** | ⚠️ GCC 远慢于 C++ 甚至慢于 Lua |
| MathMinMax | n=100K | **10.6x** | 0.90 | |

### 核心发现

1. **FakeLua GCC 在纯数值场景全面领先 Lua 5.4**（1.5x ~ 107x），Sum/Popcount/Fibonacci/Recursion/TailRecursion 均接近甚至超越手写 C++ —— 数值特化 + GCC `-O3` 向量化是核心优势。

2. **浮点特化同样出色**：FloatPoly（Horner 多项式求值）GCC 为 C++ 的 **2.0x 倍速**（34.7x vs Lua），证明 double 特化路径与 int 特化一样高效。

3. **math 函数表现分化严重**：sin/cos/sqrt 表现优异（sqrt 2x 快于 C++），但 **exp/log 是反常的性能陷阱** —— MathExpLog GCC 为 C++ 的 **28x 慢**，甚至比 Lua 还慢 6 倍。这表明 fakelua 对 exp/log 的代码生成或内联存在特定问题，值得专项排查。注意：`math.atan2`/`math.pow` 在 Lua 5.5 中已降为 compat-only 函数（需 `LUA_COMPAT_MATHLIB`），为避免在 vanilla Lua 5.5 上崩溃，已移出 benchmark。

4. **表标准库函数仍是最大短板**：table.insert/move/sort/concat 在 fakelua 中比 Lua 慢 2~3 个数量级（GCC/C++ 比最高达 1608x），因为这些函数在脚本层实现，走逐元素 CVar 装箱路径。

5. **正则匹配是最严重的短板**：fakelua 的 `string.find`/`match`/`gmatch`/`gsub` 由 `std::regex`（ECMAScript）实现，而 Lua 用的是专门优化过的轻量 pattern 引擎。在工作量对等的前提下重测，StringFindPattern GCC 比 Lua 慢 **169x**、比 C++ 慢 1111x，StringGmatch 比 Lua 慢 **94x**、比 C++ 慢 694x。主要开销来自每次调用都重新构造 `std::regex` 对象（没有编译结果缓存），以及 `std::regex` 本身相对笨重的实现。热路径上应优先使用 `string.find(s, pat, init, true)` 的 plain 子串查找。

6. **内存池分配器在表频繁创建场景优势明显**：TableChurn 场景 GCC 13.9x 快于 Lua，且与 C++ 持平（0.85x）。

---

## 完整原始输出

以下为 `benchmark_algo.cpp` / `benchmark_string.cpp` / `benchmark_table.cpp` / `benchmark_function.cpp` / `benchmark_gc.cpp` / `benchmark_math.cpp` 全部 51 个场景的完整 google benchmark 输出（含 TCC 数据，按 benchmark 名称排序）：

> ⚠️ 其中 `BM_*_StringFindPattern` / `BM_*_StringGmatch` 八行是 `[0-9]+` 修正**之前**的旧数据，两边工作量不对等，不可用于比较；以上文表格中的修正值为准。

```text
Starting benchmarks...
2026-08-12T15:08:29+08:00
Running ./bench_mark
Run on (2 X 2595.12 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x2)
  L1 Instruction 32 KiB (x2)
  L2 Unified 4096 KiB (x2)
  L3 Unified 16384 KiB (x1)
Load Average: 2.28, 2.22, 2.27
***WARNING*** ASLR is enabled, the results may have unreproducible noise in them.
--------------------------------------------------------------------------------------------
Benchmark                                                  Time             CPU   Iterations
--------------------------------------------------------------------------------------------
BM_CPP_Fibonacci/20                                   244001 ns       188867 ns         3719
BM_CPP_Fibonacci/25                                  2604029 ns      2091419 ns          336
BM_CPP_Fibonacci/30                                 28892494 ns     23208447 ns           30
BM_CPP_Fibonacci/32                                 74868609 ns     60627917 ns           12
BM_Lua_Fibonacci/20                                  1095837 ns       867154 ns          804
BM_Lua_Fibonacci/25                                 12164314 ns      9615372 ns           73
BM_Lua_Fibonacci/30                                165708142 ns    108233930 ns            7
BM_Lua_Fibonacci/32                                397572480 ns    283813384 ns            2
BM_FakeLua_Fibonacci_TCC/20                           104678 ns        77972 ns         8790
BM_FakeLua_Fibonacci_TCC/25                          1259518 ns       871771 ns          803
BM_FakeLua_Fibonacci_TCC/30                         11481546 ns      9636264 ns           74
BM_FakeLua_Fibonacci_TCC/32                         32248861 ns     24995418 ns           28
BM_FakeLua_Fibonacci_GCC/20                            35802 ns        30050 ns        23569
BM_FakeLua_Fibonacci_GCC/25                           364210 ns       271531 ns         2527
BM_FakeLua_Fibonacci_GCC/30                          4324573 ns      2808106 ns          253
BM_FakeLua_Fibonacci_GCC/32                         10451699 ns      7806774 ns           97
BM_CPP_GCD/832040/514229                                 240 ns          183 ns      3836274
BM_CPP_GCD/123456789/987654321                          55.1 ns         44.5 ns     15783864
BM_CPP_GCD/2147483647/1073741823                        54.3 ns         39.0 ns     17930236
BM_Lua_GCD/832040/514229                                 934 ns          734 ns       953192
BM_Lua_GCD/123456789/987654321                           268 ns          215 ns      3227248
BM_Lua_GCD/2147483647/1073741823                         240 ns          193 ns      3630863
BM_FakeLua_GCD_TCC/832040/514229                         801 ns          608 ns      1175104
BM_FakeLua_GCD_TCC/123456789/987654321                   636 ns          478 ns      1466185
BM_FakeLua_GCD_TCC/2147483647/1073741823                 626 ns          474 ns      1477639
BM_FakeLua_GCD_GCC/832040/514229                         729 ns          548 ns      1281849
BM_FakeLua_GCD_GCC/123456789/987654321                   616 ns          453 ns      1551256
BM_FakeLua_GCD_GCC/2147483647/1073741823                 634 ns          451 ns      1558405
BM_CPP_PowMod/2/1000/1000000007                          164 ns          121 ns      5782772
BM_CPP_PowMod/7/1000000/1000000007                       294 ns          205 ns      3413145
BM_CPP_PowMod/1234567/7654321/1000000007                 389 ns          298 ns      2348296
BM_Lua_PowMod/2/1000/1000000007                          917 ns          683 ns      1023161
BM_Lua_PowMod/7/1000000/1000000007                      1638 ns         1174 ns       611965
BM_Lua_PowMod/1234567/7654321/1000000007                1902 ns         1378 ns       502604
BM_FakeLua_PowMod_TCC/2/1000/1000000007                  975 ns          700 ns       985972
BM_FakeLua_PowMod_TCC/7/1000000/1000000007              1237 ns          919 ns       750285
BM_FakeLua_PowMod_TCC/1234567/7654321/1000000007        1429 ns         1058 ns       660055
BM_FakeLua_PowMod_GCC/2/1000/1000000007                  777 ns          577 ns      1215902
BM_FakeLua_PowMod_GCC/7/1000000/1000000007               960 ns          683 ns      1025378
BM_FakeLua_PowMod_GCC/1234567/7654321/1000000007        1020 ns          776 ns       898082
BM_CPP_Sum/10000                                       73965 ns        55114 ns        12805
BM_CPP_Sum/100000                                     799820 ns       551704 ns         1278
BM_CPP_Sum/1000000                                   6272927 ns      5492065 ns          129
BM_CPP_Sum/5000000                                  32602101 ns     27451677 ns           26
BM_Lua_Sum/10000                                      112952 ns        91478 ns         7503
BM_Lua_Sum/100000                                    1183385 ns       915086 ns          758
BM_Lua_Sum/1000000                                  12257774 ns      8800271 ns           77
BM_Lua_Sum/5000000                                  63892439 ns     44934950 ns           15
BM_FakeLua_Sum_TCC/10000                               31886 ns        25183 ns        27483
BM_FakeLua_Sum_TCC/100000                             310590 ns       251022 ns         2765
BM_FakeLua_Sum_TCC/1000000                           2972264 ns      2450162 ns          276
BM_FakeLua_Sum_TCC/5000000                          14427243 ns     12287990 ns           57
BM_FakeLua_Sum_GCC/10000                                4687 ns         3569 ns       197130
BM_FakeLua_Sum_GCC/100000                              38516 ns        31500 ns        22186
BM_FakeLua_Sum_GCC/1000000                            386643 ns       310651 ns         2252
BM_FakeLua_Sum_GCC/5000000                           1929485 ns      1551341 ns          451
BM_CPP_BubbleSort/50                                   20561 ns        16033 ns        43414
BM_CPP_BubbleSort/100                                  80161 ns        62715 ns        11150
BM_CPP_BubbleSort/200                                 315338 ns       248914 ns         2803
BM_Lua_BubbleSort/50                                  111145 ns        86159 ns         8116
BM_Lua_BubbleSort/100                                 453567 ns       335743 ns         2084
BM_Lua_BubbleSort/200                                2027811 ns      1324344 ns          534
BM_FakeLua_BubbleSort_TCC/50                          257914 ns       187624 ns         3606
BM_FakeLua_BubbleSort_TCC/100                         875880 ns       746935 ns          949
BM_FakeLua_BubbleSort_TCC/200                        3514946 ns      3054451 ns          232
BM_FakeLua_BubbleSort_GCC/50                           58056 ns        40502 ns        17084
BM_FakeLua_BubbleSort_GCC/100                         231300 ns       157924 ns         4485
BM_FakeLua_BubbleSort_GCC/200                         745482 ns       627059 ns         1103
BM_CPP_Sieve/100                                        1716 ns         1469 ns       476034
BM_CPP_Sieve/500                                        8701 ns         7454 ns        94272
BM_CPP_Sieve/1000                                      18904 ns        15087 ns        46697
BM_CPP_Sieve/5000                                     100016 ns        79319 ns         8838
BM_Lua_Sieve/100                                       13394 ns         9878 ns        70439
BM_Lua_Sieve/500                                       48460 ns        39712 ns        17671
BM_Lua_Sieve/1000                                      94106 ns        75604 ns         9470
BM_Lua_Sieve/5000                                     441337 ns       381609 ns         1832
BM_FakeLua_Sieve_TCC/100                               18112 ns        14949 ns        47887
BM_FakeLua_Sieve_TCC/500                               82602 ns        70509 ns         9766
BM_FakeLua_Sieve_TCC/1000                             187862 ns       141592 ns         4961
BM_FakeLua_Sieve_TCC/5000                            1016307 ns       872919 ns          786
BM_FakeLua_Sieve_GCC/100                                4438 ns         3662 ns       190411
BM_FakeLua_Sieve_GCC/500                               19343 ns        15967 ns        43518
BM_FakeLua_Sieve_GCC/1000                              38241 ns        32052 ns        21714
BM_FakeLua_Sieve_GCC/5000                             280594 ns       196475 ns         3529
BM_CPP_BinarySearch/100                                 6412 ns         4872 ns       143452
BM_CPP_BinarySearch/500                                39543 ns        29901 ns        23466
BM_CPP_BinarySearch/1000                               88950 ns        65515 ns        10810
BM_Lua_BinarySearch/100                                49841 ns        39759 ns        17342
BM_Lua_BinarySearch/500                               333722 ns       273027 ns         2549
BM_Lua_BinarySearch/1000                              850654 ns       621516 ns         1161
BM_FakeLua_BinarySearch_TCC/100                        71410 ns        53885 ns        13075
BM_FakeLua_BinarySearch_TCC/500                       512519 ns       382369 ns         1833
BM_FakeLua_BinarySearch_TCC/1000                     1229219 ns       891469 ns          771
BM_FakeLua_BinarySearch_GCC/100                        10781 ns         8076 ns        87637
BM_FakeLua_BinarySearch_GCC/500                       102455 ns        67772 ns        10375
BM_FakeLua_BinarySearch_GCC/1000                      190968 ns       158817 ns         4423
BM_CPP_FastPow/2/1000/1000000007                         143 ns          122 ns      5773546
BM_CPP_FastPow/7/1000000/1000000007                      243 ns          207 ns      3379019
BM_CPP_FastPow/1234567/7654321/1000000007                355 ns          300 ns      2334171
BM_Lua_FastPow/2/1000/1000000007                         749 ns          611 ns      1177502
BM_Lua_FastPow/7/1000000/1000000007                     1541 ns          981 ns       713501
BM_Lua_FastPow/1234567/7654321/1000000007               1549 ns         1159 ns       601482
BM_FakeLua_FastPow_TCC/2/1000/1000000007                 755 ns          624 ns      1104775
BM_FakeLua_FastPow_TCC/7/1000000/1000000007              926 ns          723 ns       967899
BM_FakeLua_FastPow_TCC/1234567/7654321/1000000007        985 ns          806 ns       868314
BM_FakeLua_FastPow_GCC/2/1000/1000000007                 704 ns          566 ns      1234099
BM_FakeLua_FastPow_GCC/7/1000000/1000000007              831 ns          667 ns      1046521
BM_FakeLua_FastPow_GCC/1234567/7654321/1000000007       1118 ns          762 ns       918455
BM_CPP_Popcount/1000                                   44926 ns        32710 ns        21385
BM_CPP_Popcount/10000                                 503463 ns       409351 ns         1711
BM_CPP_Popcount/100000                               5876893 ns      5007660 ns          140
BM_Lua_Popcount/1000                                  186548 ns       145867 ns         4836
BM_Lua_Popcount/10000                                2453630 ns      1809095 ns          386
BM_Lua_Popcount/100000                              28290747 ns     22278137 ns           32
BM_FakeLua_Popcount_TCC/1000                           16270 ns        12688 ns        53993
BM_FakeLua_Popcount_TCC/10000                         191919 ns       151174 ns         4632
BM_FakeLua_Popcount_TCC/100000                       2332164 ns      1802909 ns          414
BM_FakeLua_Popcount_GCC/1000                            5728 ns         4595 ns       151917
BM_FakeLua_Popcount_GCC/10000                          76147 ns        49186 ns        14313
BM_FakeLua_Popcount_GCC/100000                        798193 ns       597600 ns         1171
BM_CPP_InsertionSort/50                                10845 ns         8120 ns        86142
BM_CPP_InsertionSort/100                               41131 ns        30781 ns        22721
BM_CPP_InsertionSort/200                              159839 ns       119814 ns         5838
BM_Lua_InsertionSort/50                                84623 ns        60257 ns        11664
BM_Lua_InsertionSort/100                              278283 ns       232025 ns         3076
BM_Lua_InsertionSort/200                             1067368 ns       906011 ns          770
BM_FakeLua_InsertionSort_TCC/50                       130134 ns       120322 ns         5911
BM_FakeLua_InsertionSort_TCC/100                      549659 ns       462438 ns         1500
BM_FakeLua_InsertionSort_TCC/200                     2242257 ns      1794866 ns          379
BM_FakeLua_InsertionSort_GCC/50                        30456 ns        23964 ns        29424
BM_FakeLua_InsertionSort_GCC/100                      120411 ns        90990 ns         7699
BM_FakeLua_InsertionSort_GCC/200                      461297 ns       351381 ns         1990
BM_CPP_MatMul                                            321 ns          248 ns      2819887
BM_Lua_MatMul                                           4407 ns         3642 ns       191864
BM_FakeLua_MatMul_TCC                                   4391 ns         3426 ns       204270
BM_FakeLua_MatMul_GCC                                   1470 ns         1100 ns       639194
BM_CPP_Vector3/10000                                   66859 ns        52702 ns        13251
BM_CPP_Vector3/100000                                 729243 ns       530514 ns         1328
BM_CPP_Vector3/1000000                               7187725 ns      5289390 ns          131
BM_Lua_Vector3/10000                                 1990411 ns      1499313 ns          462
BM_Lua_Vector3/100000                               20282472 ns     14966766 ns           47
BM_Lua_Vector3/1000000                             199163143 ns    149785755 ns            5
BM_FakeLua_Vector3_TCC/10000                         2711982 ns      2046746 ns          348
BM_FakeLua_Vector3_TCC/100000                       27786847 ns     20614034 ns           35
BM_FakeLua_Vector3_TCC/1000000                     244198035 ns    205447166 ns            3
BM_FakeLua_Vector3_GCC/10000                          388782 ns       305854 ns         2295
BM_FakeLua_Vector3_GCC/100000                        3977854 ns      3042764 ns          231
BM_FakeLua_Vector3_GCC/1000000                      41820437 ns     30362696 ns           23
BM_CPP_FloatPoly/1000000                             7640625 ns      5817973 ns          119
BM_Lua_FloatPoly/1000000                           140330396 ns     99455046 ns            7
BM_FakeLua_FloatPoly_TCC/1000000                    24121306 ns     14361104 ns           49
BM_FakeLua_FloatPoly_GCC/1000000                     3569147 ns      2870252 ns          244
BM_CPP_EmptyCall/10000                                  7.10 ns         5.50 ns    126963265
BM_CPP_EmptyCall/100000                                 6.71 ns         5.51 ns    126962922
BM_Lua_EmptyCall/10000                                537409 ns       395641 ns         1762
BM_Lua_EmptyCall/100000                              5222610 ns      3965114 ns          177
BM_FakeLua_EmptyCall_TCC/10000                        343108 ns       251721 ns         2768
BM_FakeLua_EmptyCall_TCC/100000                      3422245 ns      2515211 ns          281
BM_FakeLua_EmptyCall_GCC/10000                         41720 ns        30282 ns        22867
BM_FakeLua_EmptyCall_GCC/100000                       388070 ns       305375 ns         2290
BM_CPP_Recursion/10                                     2245 ns         1523 ns       459427
BM_CPP_Recursion/20                                   268446 ns       192172 ns         3633
BM_CPP_Recursion/25                                  2706553 ns      2101749 ns          327
BM_Lua_Recursion/10                                    11643 ns         8655 ns        80749
BM_Lua_Recursion/20                                  1267839 ns      1060900 ns          674
BM_Lua_Recursion/25                                 13497088 ns     11788694 ns           59
BM_FakeLua_Recursion_TCC/10                             1243 ns         1086 ns       645911
BM_FakeLua_Recursion_TCC/20                            97814 ns        79807 ns         8705
BM_FakeLua_Recursion_TCC/25                           991358 ns       874359 ns          799
BM_FakeLua_Recursion_GCC/10                              707 ns          599 ns      1161702
BM_FakeLua_Recursion_GCC/20                            40499 ns        29632 ns        23741
BM_FakeLua_Recursion_GCC/25                           396165 ns       268053 ns         2654
BM_CPP_Variadic/1                                       14.4 ns         11.0 ns     63803395
BM_Lua_Variadic/1                                        784 ns          680 ns      1016480
BM_FakeLua_Variadic_TCC/1                               7181 ns         6123 ns       116928
BM_FakeLua_Variadic_GCC/1                               6586 ns         5893 ns       118888
BM_CPP_MultiReturn/1000                                 6440 ns         5549 ns       127396
BM_CPP_MultiReturn/10000                               63605 ns        55276 ns        12623
BM_Lua_MultiReturn/1000                                73369 ns        60650 ns        11426
BM_Lua_MultiReturn/10000                              700380 ns       602970 ns         1140
BM_FakeLua_MultiReturn_TCC/1000                         7840 ns         5767 ns       119459
BM_FakeLua_MultiReturn_TCC/10000                       71207 ns        53355 ns        13265
BM_FakeLua_MultiReturn_GCC/1000                         4440 ns         2969 ns       235501
BM_FakeLua_MultiReturn_GCC/10000                       34759 ns        25036 ns        27825
BM_CPP_Closure/100                                       804 ns          555 ns      1259497
BM_CPP_Closure/1000                                     6754 ns         5520 ns       123555
BM_Lua_Closure/100                                     29972 ns        24180 ns        29185
BM_Lua_Closure/1000                                   297514 ns       238287 ns         2892
BM_FakeLua_Closure_TCC/100                             19931 ns        16992 ns        41135
BM_FakeLua_Closure_TCC/1000                           197950 ns       166231 ns         4234
BM_FakeLua_Closure_GCC/100                              9280 ns         6484 ns       107193
BM_FakeLua_Closure_GCC/1000                            86871 ns        60425 ns        11470
BM_CPP_TailRecursion/100                                 851 ns          553 ns      1266789
BM_CPP_TailRecursion/1000                               6550 ns         5496 ns       127347
BM_CPP_TailRecursion/5000                              33006 ns        27464 ns        25540
BM_Lua_TailRecursion/100                                5452 ns         4471 ns       156425
BM_Lua_TailRecursion/1000                              54899 ns        42810 ns        16210
BM_Lua_TailRecursion/5000                             303389 ns       213681 ns         3314
BM_FakeLua_TailRecursion_TCC/100                        1285 ns          918 ns       770917
BM_FakeLua_TailRecursion_TCC/1000                       7418 ns         5246 ns       135067
BM_FakeLua_TailRecursion_TCC/5000                      29759 ns        25266 ns        29155
BM_FakeLua_TailRecursion_GCC/100                         536 ns          463 ns      1512128
BM_FakeLua_TailRecursion_GCC/1000                        889 ns          757 ns       918265
BM_FakeLua_TailRecursion_GCC/5000                       2182 ns         2001 ns       343031
BM_CPP_TableChurn/100                                   6057 ns         5362 ns       129836
BM_CPP_TableChurn/500                                  36071 ns        26521 ns        25770
BM_CPP_TableChurn/1000                                 69228 ns        52824 ns        13320
BM_Lua_TableChurn/100                                  88920 ns        62584 ns        10900
BM_Lua_TableChurn/500                                 382283 ns       312124 ns         2276
BM_Lua_TableChurn/1000                                798672 ns       623987 ns         1137
BM_FakeLua_TableChurn_TCC/100                          27455 ns        24056 ns        29701
BM_FakeLua_TableChurn_TCC/500                         152607 ns       118942 ns         5847
BM_FakeLua_TableChurn_TCC/1000                        301820 ns       235828 ns         2943
BM_FakeLua_TableChurn_GCC/100                           6144 ns         5037 ns       135846
BM_FakeLua_TableChurn_GCC/500                          27835 ns        22605 ns        30319
BM_FakeLua_TableChurn_GCC/1000                         68913 ns        45037 ns        15801
BM_CPP_StringChurn/100                                 47994 ns        29712 ns        23489
BM_CPP_StringChurn/500                                182406 ns       153421 ns         4566
BM_CPP_StringChurn/1000                               382854 ns       311758 ns         2277
BM_Lua_StringChurn/100                                 96332 ns        75548 ns         9534
BM_Lua_StringChurn/500                                595908 ns       489138 ns         1425
BM_Lua_StringChurn/1000                              1444872 ns      1003632 ns          696
BM_FakeLua_StringChurn_TCC/100                        198281 ns       141653 ns         5028
BM_FakeLua_StringChurn_TCC/500                        906911 ns       715240 ns          979
BM_FakeLua_StringChurn_TCC/1000                      1701709 ns      1421517 ns          494
BM_FakeLua_StringChurn_GCC/100                        161297 ns       131602 ns         5352
BM_FakeLua_StringChurn_GCC/500                        835406 ns       660005 ns         1067
BM_FakeLua_StringChurn_GCC/1000                      1594786 ns      1306204 ns          530
BM_CPP_MixedAlloc/100                                  33552 ns        23049 ns        30324
BM_CPP_MixedAlloc/500                                 152138 ns       118122 ns         6001
BM_CPP_MixedAlloc/1000                                352953 ns       236465 ns         2947
BM_Lua_MixedAlloc/100                                 113150 ns        92271 ns         7676
BM_Lua_MixedAlloc/500                                 682426 ns       470793 ns         1485
BM_Lua_MixedAlloc/1000                               1243862 ns       943406 ns          745
BM_FakeLua_MixedAlloc_TCC/100                         115008 ns        95436 ns         7400
BM_FakeLua_MixedAlloc_TCC/500                         641732 ns       478158 ns         1469
BM_FakeLua_MixedAlloc_TCC/1000                       1279982 ns       951658 ns          747
BM_FakeLua_MixedAlloc_GCC/100                         156540 ns        74401 ns         9323
BM_FakeLua_MixedAlloc_GCC/500                         660755 ns       372074 ns         1888
BM_FakeLua_MixedAlloc_GCC/1000                        937336 ns       761541 ns          912
BM_CPP_MathTrig/100000                               4428913 ns      3545417 ns          197
BM_Lua_MathTrig/100000                              22585647 ns     18260022 ns           38
BM_FakeLua_MathTrig_TCC/100000                      14450561 ns     11594773 ns           60
BM_FakeLua_MathTrig_GCC/100000                       3815773 ns      3079505 ns          228
BM_CPP_MathSqrt/100000                                734133 ns       570643 ns         1238
BM_Lua_MathSqrt/100000                               7657403 ns      6274359 ns          113
BM_FakeLua_MathSqrt_TCC/100000                       5111395 ns      4115277 ns          171
BM_FakeLua_MathSqrt_GCC/100000                        338425 ns       276775 ns         2530
BM_CPP_MathExpLog/100000                             5912180 ns      4741305 ns          148
BM_Lua_MathExpLog/100000                            23017204 ns     19033686 ns           37
BM_FakeLua_MathExpLog_TCC/100000                   185797707 ns    137865272 ns            5
BM_FakeLua_MathExpLog_GCC/100000                   162874484 ns    133183547 ns            5
BM_CPP_MathMinMax/100000                             3494186 ns      2420676 ns          289
BM_Lua_MathMinMax/100000                            28660028 ns     22968714 ns           30
BM_FakeLua_MathMinMax_TCC/100000                    14167435 ns     11518015 ns           61
BM_FakeLua_MathMinMax_GCC/100000                     2707326 ns      2173469 ns          321
BM_CPP_StringLen/10                                     19.9 ns         16.8 ns     41674582
BM_CPP_StringLen/100                                    20.3 ns         16.8 ns     41698120
BM_CPP_StringLen/1000                                   20.2 ns         16.7 ns     41769863
BM_CPP_StringLen/10000                                  24.4 ns         16.8 ns     41937007
BM_Lua_StringLen/10                                      230 ns          187 ns      3746761
BM_Lua_StringLen/100                                     372 ns          343 ns      2053664
BM_Lua_StringLen/1000                                    689 ns          521 ns      1333793
BM_Lua_StringLen/10000                                  2741 ns         2003 ns       348503
BM_FakeLua_StringLen_TCC/10                              785 ns          552 ns      1271769
BM_FakeLua_StringLen_TCC/100                             941 ns          608 ns      1163935
BM_FakeLua_StringLen_TCC/1000                            964 ns          637 ns      1093728
BM_FakeLua_StringLen_TCC/10000                          1416 ns         1142 ns       607563
BM_FakeLua_StringLen_GCC/10                              785 ns          551 ns      1271884
BM_FakeLua_StringLen_GCC/100                             782 ns          602 ns      1143457
BM_FakeLua_StringLen_GCC/1000                            761 ns          629 ns      1110335
BM_FakeLua_StringLen_GCC/10000                          1374 ns         1121 ns       624707
BM_CPP_StringSub/10                                      118 ns         92.7 ns      7556968
BM_CPP_StringSub/100                                     183 ns          136 ns      5132732
BM_CPP_StringSub/1000                                    190 ns          144 ns      4857186
BM_CPP_StringSub/10000                                   491 ns          361 ns      1951418
BM_Lua_StringSub/10                                      741 ns          392 ns      1786635
BM_Lua_StringSub/100                                     943 ns          633 ns      1122708
BM_Lua_StringSub/1000                                   1268 ns          949 ns       718797
BM_Lua_StringSub/10000                                  4438 ns         3196 ns       219348
BM_FakeLua_StringSub_TCC/10                             1616 ns         1379 ns       510092
BM_FakeLua_StringSub_TCC/100                            1693 ns         1483 ns       473629
BM_FakeLua_StringSub_TCC/1000                           1788 ns         1531 ns       457399
BM_FakeLua_StringSub_TCC/10000                          2906 ns         2329 ns       300023
BM_FakeLua_StringSub_GCC/10                             1647 ns         1372 ns       510094
BM_FakeLua_StringSub_GCC/100                            1718 ns         1442 ns       485989
BM_FakeLua_StringSub_GCC/1000                           2301 ns         1518 ns       456373
BM_FakeLua_StringSub_GCC/10000                          2937 ns         2333 ns       301548
BM_CPP_StringRep/10                                      720 ns          420 ns      1645804
BM_CPP_StringRep/100                                    5664 ns         3071 ns       227801
BM_CPP_StringRep/1000                                  60314 ns        30033 ns        23192
BM_Lua_StringRep/10                                      705 ns          429 ns      1646933
BM_Lua_StringRep/100                                    2054 ns         1065 ns       639071
BM_Lua_StringRep/1000                                   9809 ns         5488 ns       117228
BM_FakeLua_StringRep_TCC/10                             2140 ns         1690 ns       417956
BM_FakeLua_StringRep_TCC/100                            5252 ns         4462 ns       157405
BM_FakeLua_StringRep_TCC/1000                          38221 ns        31596 ns        22129
BM_FakeLua_StringRep_GCC/10                             2005 ns         1688 ns       411427
BM_FakeLua_StringRep_GCC/100                            5176 ns         4444 ns       157613
BM_FakeLua_StringRep_GCC/1000                          47114 ns        31573 ns        22260
BM_CPP_StringReverse/10                                  150 ns          104 ns      6728966
BM_CPP_StringReverse/100                                 617 ns          455 ns      1533631
BM_CPP_StringReverse/1000                               4770 ns         3523 ns       198592
BM_CPP_StringReverse/10000                             45970 ns        34349 ns        20420
BM_Lua_StringReverse/10                                  502 ns          314 ns      2218709
BM_Lua_StringReverse/100                                1015 ns          724 ns       965645
BM_Lua_StringReverse/1000                               2476 ns         1917 ns       368622
BM_Lua_StringReverse/10000                             18670 ns        14211 ns        51314
BM_FakeLua_StringReverse_TCC/10                         1447 ns         1210 ns       588507
BM_FakeLua_StringReverse_TCC/100                        1799 ns         1650 ns       420761
BM_FakeLua_StringReverse_TCC/1000                       5434 ns         4787 ns       146279
BM_FakeLua_StringReverse_TCC/10000                     45719 ns        36459 ns        19062
BM_FakeLua_StringReverse_GCC/10                         1479 ns         1184 ns       589100
BM_FakeLua_StringReverse_GCC/100                        1971 ns         1645 ns       425514
BM_FakeLua_StringReverse_GCC/1000                       6496 ns         4756 ns       146607
BM_FakeLua_StringReverse_GCC/10000                     43227 ns        36576 ns        19274
BM_CPP_StringLower/10                                    354 ns          194 ns      3568595
BM_CPP_StringLower/100                                  1635 ns         1356 ns       516449
BM_CPP_StringLower/1000                                14338 ns        12548 ns        55892
BM_CPP_StringLower/10000                              144704 ns       124552 ns         5608
BM_Lua_StringLower/10                                    373 ns          327 ns      2140132
BM_Lua_StringLower/100                                   935 ns          757 ns       931277
BM_Lua_StringLower/1000                                 2591 ns         2219 ns       317137
BM_Lua_StringLower/10000                               21705 ns        17124 ns        41137
BM_FakeLua_StringLower_TCC/10                           1433 ns         1204 ns       586100
BM_FakeLua_StringLower_TCC/100                          2434 ns         1929 ns       360856
BM_FakeLua_StringLower_TCC/1000                        10045 ns         7725 ns        90314
BM_FakeLua_StringLower_TCC/10000                       82321 ns        66543 ns        10480
BM_FakeLua_StringLower_GCC/10                           1548 ns         1195 ns       585487
BM_FakeLua_StringLower_GCC/100                          2235 ns         1933 ns       360914
BM_FakeLua_StringLower_GCC/1000                        10801 ns         7773 ns        91180
BM_FakeLua_StringLower_GCC/10000                      100611 ns        66635 ns        10666
BM_CPP_StringUpper/10                                    254 ns          197 ns      3555340
BM_CPP_StringUpper/100                                  1803 ns         1371 ns       510412
BM_CPP_StringUpper/1000                                16573 ns        12648 ns        55404
BM_CPP_StringUpper/10000                              148923 ns       124852 ns         5602
BM_Lua_StringUpper/10                                    395 ns          318 ns      2199829
BM_Lua_StringUpper/100                                  1010 ns          724 ns       971687
BM_Lua_StringUpper/1000                                 2906 ns         2026 ns       346674
BM_Lua_StringUpper/10000                               19231 ns        14751 ns        47236
BM_FakeLua_StringUpper_TCC/10                           1876 ns         1188 ns       591885
BM_FakeLua_StringUpper_TCC/100                          2590 ns         1918 ns       366064
BM_FakeLua_StringUpper_TCC/1000                        10841 ns         7800 ns        90394
BM_FakeLua_StringUpper_TCC/10000                      101903 ns        67317 ns        10515
BM_FakeLua_StringUpper_GCC/10                           1425 ns         1195 ns       584536
BM_FakeLua_StringUpper_GCC/100                          2303 ns         1920 ns       363190
BM_FakeLua_StringUpper_GCC/1000                         9024 ns         7766 ns        89774
BM_FakeLua_StringUpper_GCC/10000                       77316 ns        66902 ns        10490
BM_CPP_StringByte/10                                    23.6 ns         16.9 ns     41168498
BM_CPP_StringByte/100                                   20.6 ns         16.9 ns     41551028
BM_CPP_StringByte/1000                                  22.0 ns         16.9 ns     41330430
BM_Lua_StringByte/10                                     356 ns          284 ns      2470813
BM_Lua_StringByte/100                                    557 ns          442 ns      1585153
BM_Lua_StringByte/1000                                   817 ns          619 ns      1123045
BM_FakeLua_StringByte_TCC/10                            1481 ns         1176 ns       592065
BM_FakeLua_StringByte_TCC/100                           1560 ns         1233 ns       564802
BM_FakeLua_StringByte_TCC/1000                          1557 ns         1261 ns       552768
BM_FakeLua_StringByte_GCC/10                            1822 ns         1177 ns       596420
BM_FakeLua_StringByte_GCC/100                           1773 ns         1231 ns       573251
BM_FakeLua_StringByte_GCC/1000                          1704 ns         1265 ns       553388
BM_CPP_StringChar/10                                     238 ns          206 ns      3414614
BM_CPP_StringChar/100                                   1974 ns         1448 ns       479190
BM_CPP_StringChar/500                                   8241 ns         6817 ns       103196
BM_Lua_StringChar/10                                    4482 ns         3085 ns       226181
BM_Lua_StringChar/100                                  20611 ns        17919 ns        38728
BM_Lua_StringChar/500                                  94589 ns        77815 ns         9370
BM_FakeLua_StringChar_TCC/10                           18737 ns        16069 ns        44164
BM_FakeLua_StringChar_TCC/100                         274684 ns       205036 ns         3423
BM_FakeLua_StringChar_TCC/500                        2976099 ns      2130511 ns          327
BM_FakeLua_StringChar_GCC/10                           19983 ns        15063 ns        46371
BM_FakeLua_StringChar_GCC/100                         262169 ns       195854 ns         3575
BM_FakeLua_StringChar_GCC/500                        2855694 ns      2089168 ns          337
BM_CPP_StringFormat/10                                   667 ns          536 ns      1300872
BM_CPP_StringFormat/100                                 6815 ns         5299 ns       133071
BM_CPP_StringFormat/500                                37191 ns        29447 ns        23740
BM_Lua_StringFormat/10                                  3685 ns         2953 ns       236796
BM_Lua_StringFormat/100                                36329 ns        28668 ns        24427
BM_Lua_StringFormat/500                               198469 ns       147611 ns         4799
BM_FakeLua_StringFormat_TCC/10                         14330 ns        10776 ns        65074
BM_FakeLua_StringFormat_TCC/100                       138036 ns       103169 ns         6762
BM_FakeLua_StringFormat_TCC/500                       678364 ns       515325 ns         1367
BM_FakeLua_StringFormat_GCC/10                         14179 ns        10652 ns        65386
BM_FakeLua_StringFormat_GCC/100                       143547 ns       102189 ns         6825
BM_FakeLua_StringFormat_GCC/500                       696456 ns       510061 ns         1370
BM_CPP_StringFind/10                                    88.3 ns         66.2 ns     10560864
BM_CPP_StringFind/100                                   90.5 ns         68.6 ns     10171823
BM_CPP_StringFind/1000                                   103 ns         70.5 ns      9951069
BM_CPP_StringFind/10000                                  190 ns          128 ns      5429439
BM_Lua_StringFind/10                                     617 ns          482 ns      1449480
BM_Lua_StringFind/100                                    751 ns          599 ns      1164241
BM_Lua_StringFind/1000                                  1086 ns          802 ns       879119
BM_Lua_StringFind/10000                                 3039 ns         2216 ns       314954
BM_FakeLua_StringFind_TCC/10                            2845 ns         1711 ns       412027
BM_FakeLua_StringFind_TCC/100                           2440 ns         1720 ns       406821
BM_FakeLua_StringFind_TCC/1000                          2129 ns         1785 ns       393471
BM_FakeLua_StringFind_TCC/10000                         3171 ns         2621 ns       263347
BM_FakeLua_StringFind_GCC/10                            1989 ns         1698 ns       411205
BM_FakeLua_StringFind_GCC/100                           2073 ns         1709 ns       409127
BM_FakeLua_StringFind_GCC/1000                          2130 ns         1763 ns       398758
BM_FakeLua_StringFind_GCC/10000                         3225 ns         2582 ns       268519
BM_CPP_StringGsub/10                                     257 ns          185 ns      3767363
BM_CPP_StringGsub/100                                   2461 ns         1735 ns       403615
BM_CPP_StringGsub/1000                                 23592 ns        17165 ns        40897
BM_Lua_StringGsub/10                                    1316 ns          973 ns       718553
BM_Lua_StringGsub/100                                   9011 ns         6512 ns       107387
BM_Lua_StringGsub/1000                                 80089 ns        57167 ns        12154
BM_FakeLua_StringGsub_TCC/10                           14776 ns        10508 ns        65956
BM_FakeLua_StringGsub_TCC/100                          86643 ns        70203 ns         9944
BM_FakeLua_StringGsub_TCC/1000                        836159 ns       665230 ns         1056
BM_FakeLua_StringGsub_GCC/10                           13436 ns        10466 ns        66438
BM_FakeLua_StringGsub_GCC/100                          89500 ns        70417 ns         9953
BM_FakeLua_StringGsub_GCC/1000                        866494 ns       667670 ns         1046
BM_CPP_ToNumber/1                                        119 ns         92.5 ns      7537568
BM_Lua_ToNumber/1                                        325 ns          269 ns      2589378
BM_FakeLua_ToNumber_TCC/1                               1885 ns         1465 ns       474466
BM_FakeLua_ToNumber_GCC/1                               1817 ns         1453 ns       478264
BM_CPP_ToString/10                                      1136 ns          891 ns       785820
BM_CPP_ToString/100                                    11054 ns         8745 ns        79803
BM_CPP_ToString/500                                    59570 ns        44771 ns        15725
BM_Lua_ToString/10                                       716 ns          510 ns      1356090
BM_Lua_ToString/100                                      665 ns          514 ns      1371493
BM_Lua_ToString/500                                      689 ns          513 ns      1357492
BM_FakeLua_ToString_TCC/10                              1361 ns         1046 ns       664452
BM_FakeLua_ToString_TCC/100                             1297 ns         1059 ns       664222
BM_FakeLua_ToString_TCC/500                             1295 ns         1059 ns       659709
BM_FakeLua_ToString_GCC/10                              1260 ns         1044 ns       672893
BM_FakeLua_ToString_GCC/100                             1272 ns         1054 ns       667884
BM_FakeLua_ToString_GCC/500                             1262 ns         1053 ns       663416
BM_CPP_StringFindPattern/1000                         239122 ns       191815 ns         3613
BM_Lua_StringFindPattern/1000                        1291275 ns       995323 ns          693
BM_FakeLua_StringFindPattern_TCC/1000                6775461 ns      5489633 ns          128
BM_FakeLua_StringFindPattern_GCC/1000                6868854 ns      5390184 ns          131
BM_CPP_StringGmatch/1000                              251438 ns       221912 ns         3161
BM_Lua_StringGmatch/1000                             1652116 ns      1248890 ns          572
BM_FakeLua_StringGmatch_TCC/1000                     6829500 ns      6271437 ns          111
BM_FakeLua_StringGmatch_GCC/1000                     7368825 ns      6091806 ns          115
BM_CPP_TableInsert/100                                   783 ns          664 ns      1058184
BM_CPP_TableInsert/500                                  3694 ns         3006 ns       231731
BM_CPP_TableInsert/1000                                 7167 ns         5920 ns       117633
BM_CPP_TableInsert/5000                                38704 ns        29040 ns        23949
BM_Lua_TableInsert/100                                 19830 ns        13680 ns        51014
BM_Lua_TableInsert/500                                 85396 ns        61844 ns        11878
BM_Lua_TableInsert/1000                               171401 ns       120782 ns         5680
BM_Lua_TableInsert/5000                               793507 ns       585246 ns         1156
BM_FakeLua_TableInsert_TCC/100                         66328 ns        54777 ns        12820
BM_FakeLua_TableInsert_TCC/500                       1451992 ns      1088750 ns          642
BM_FakeLua_TableInsert_TCC/1000                      5369852 ns      4322096 ns          164
BM_FakeLua_TableInsert_TCC/5000                    119662784 ns    105876177 ns            7
BM_FakeLua_TableInsert_GCC/100                         18588 ns        16270 ns        43526
BM_FakeLua_TableInsert_GCC/500                        423879 ns       328276 ns         2216
BM_FakeLua_TableInsert_GCC/1000                      1502031 ns      1295016 ns          537
BM_FakeLua_TableInsert_GCC/5000                     39966428 ns     29610920 ns           24
BM_CPP_TableRemove/100                                  2620 ns         1808 ns       387855
BM_CPP_TableRemove/500                                 13512 ns         8739 ns        80612
BM_CPP_TableRemove/1000                                19918 ns        17753 ns        39383
BM_CPP_TableRemove/5000                               103304 ns        87640 ns         7930
BM_Lua_TableRemove/100                                 23370 ns        19585 ns        35417
BM_Lua_TableRemove/500                                101572 ns        86839 ns         7911
BM_Lua_TableRemove/1000                               208276 ns       168288 ns         4071
BM_Lua_TableRemove/5000                              1098396 ns       917705 ns          758
BM_FakeLua_TableRemove_TCC/100                         72940 ns        61653 ns        11279
BM_FakeLua_TableRemove_TCC/500                       1510153 ns      1130615 ns          618
BM_FakeLua_TableRemove_TCC/1000                      5908257 ns      4337975 ns          162
BM_FakeLua_TableRemove_TCC/5000                    146538395 ns    104609798 ns            7
BM_FakeLua_TableRemove_GCC/100                         25293 ns        18198 ns        36974
BM_FakeLua_TableRemove_GCC/500                        448550 ns       324855 ns         2120
BM_FakeLua_TableRemove_GCC/1000                      1601957 ns      1242525 ns          564
BM_FakeLua_TableRemove_GCC/5000                     36387976 ns     30472920 ns           23
BM_CPP_TableConcat/100                                 10055 ns         8799 ns        80105
BM_CPP_TableConcat/500                                 52325 ns        44868 ns        15589
BM_CPP_TableConcat/1000                               106755 ns        90039 ns         7768
BM_Lua_TableConcat/100                                 51984 ns        38274 ns        18147
BM_Lua_TableConcat/500                                276820 ns       188649 ns         3722
BM_Lua_TableConcat/1000                               507283 ns       379999 ns         1832
BM_FakeLua_TableConcat_TCC/100                        225853 ns       192706 ns         3723
BM_FakeLua_TableConcat_TCC/500                       2424617 ns      2081939 ns          337
BM_FakeLua_TableConcat_TCC/1000                      7971335 ns      6957870 ns           99
BM_FakeLua_TableConcat_GCC/100                        209450 ns       184053 ns         3808
BM_FakeLua_TableConcat_GCC/500                       2373606 ns      2042856 ns          343
BM_FakeLua_TableConcat_GCC/1000                      8972538 ns      6884714 ns          102
BM_CPP_TablePack/1                                      7.62 ns         5.52 ns    126824477
BM_Lua_TablePack/1                                      2270 ns         1508 ns       453066
BM_FakeLua_TablePack_TCC/1                              1772 ns         1398 ns       504946
BM_FakeLua_TablePack_GCC/1                               949 ns          703 ns      1005029
BM_CPP_TableMove/100                                    2349 ns         1919 ns       364542
BM_CPP_TableMove/500                                   13773 ns         9112 ns        77038
BM_CPP_TableMove/1000                                  24574 ns        17941 ns        39076
BM_CPP_TableMove/5000                                 164542 ns        88971 ns         7911
BM_Lua_TableMove/100                                   11145 ns         9438 ns        75906
BM_Lua_TableMove/500                                   35022 ns        30325 ns        22956
BM_Lua_TableMove/1000                                  68190 ns        55370 ns        12679
BM_Lua_TableMove/5000                                 306408 ns       269300 ns         2588
BM_FakeLua_TableMove_TCC/100                          131860 ns       113604 ns         6153
BM_FakeLua_TableMove_TCC/500                         2036913 ns      1683879 ns          413
BM_FakeLua_TableMove_TCC/1000                        7436139 ns      6159088 ns          114
BM_FakeLua_TableMove_TCC/5000                      197092956 ns    143306119 ns            5
BM_FakeLua_TableMove_GCC/100                          149727 ns       107889 ns         6515
BM_FakeLua_TableMove_GCC/500                         2309838 ns      1678119 ns          416
BM_FakeLua_TableMove_GCC/1000                        8444337 ns      6185440 ns          113
BM_FakeLua_TableMove_GCC/5000                      223027760 ns    143053397 ns            5
BM_CPP_TableSort/100                                    7729 ns         5223 ns       133732
BM_CPP_TableSort/500                                   42171 ns        33348 ns        21094
BM_CPP_TableSort/1000                                  85949 ns        69875 ns        10057
BM_Lua_TableSort/100                                   38986 ns        30933 ns        22685
BM_Lua_TableSort/500                                  247574 ns       190351 ns         3664
BM_Lua_TableSort/1000                                 589248 ns       409169 ns         1709
BM_FakeLua_TableSort_TCC/100                          277771 ns       175282 ns         3973
BM_FakeLua_TableSort_TCC/500                         4214847 ns      3123057 ns          224
BM_FakeLua_TableSort_TCC/1000                       16585657 ns     11862535 ns           59
BM_FakeLua_TableSort_GCC/100                          195888 ns       165667 ns         4241
BM_FakeLua_TableSort_GCC/500                         3564204 ns      3069208 ns          228
BM_FakeLua_TableSort_GCC/1000                       13942763 ns     11756315 ns           59
BM_CPP_TableCreate/1000                                14341 ns        11812 ns        58980
BM_CPP_TableCreate/3000                                40850 ns        35176 ns        19834
BM_CPP_TableCreate/5000                                68488 ns        58292 ns        12135
BM_Lua_TableCreate/1000                                33272 ns        27710 ns        25386
BM_Lua_TableCreate/3000                                99579 ns        81255 ns         8612
BM_Lua_TableCreate/5000                               187385 ns       140588 ns         4949
BM_FakeLua_TableCreate_TCC/1000                       123928 ns        98272 ns         7157
BM_FakeLua_TableCreate_TCC/3000                       405145 ns       342909 ns         2053
BM_FakeLua_TableCreate_TCC/5000                       720608 ns       629690 ns         1108
BM_FakeLua_TableCreate_GCC/1000                        22936 ns        19493 ns        35938
BM_FakeLua_TableCreate_GCC/3000                        79501 ns        66952 ns        10688
BM_FakeLua_TableCreate_GCC/5000                       146031 ns       125736 ns         5564
BM_CPP_HashInsert/100                                  31074 ns        25627 ns        27321
BM_CPP_HashInsert/500                                 167382 ns       144563 ns         4831
BM_CPP_HashInsert/1000                                352005 ns       296431 ns         2358
BM_Lua_HashInsert/100                                  60447 ns        49093 ns        14224
BM_Lua_HashInsert/500                                 300879 ns       246976 ns         2835
BM_Lua_HashInsert/1000                                616094 ns       516227 ns         1347
BM_FakeLua_HashInsert_TCC/100                          82384 ns        69876 ns        10051
BM_FakeLua_HashInsert_TCC/500                         420580 ns       344089 ns         2028
BM_FakeLua_HashInsert_TCC/1000                        875961 ns       740259 ns          946
BM_FakeLua_HashInsert_GCC/100                          73095 ns        57537 ns        12208
BM_FakeLua_HashInsert_GCC/500                         372756 ns       286522 ns         2447
BM_FakeLua_HashInsert_GCC/1000                        778321 ns       591072 ns         1163
BM_CPP_HashLookup/100                                 460844 ns       317764 ns         2209
BM_CPP_HashLookup/500                                 543700 ns       401039 ns         1745
BM_CPP_HashLookup/1000                                803534 ns       503815 ns         1392
BM_Lua_HashLookup/100                                  47576 ns        40821 ns        17129
BM_Lua_HashLookup/500                                 237389 ns       209240 ns         3305
BM_Lua_HashLookup/1000                                483369 ns       424861 ns         1632
BM_FakeLua_HashLookup_TCC/100                          74025 ns        63641 ns        10944
BM_FakeLua_HashLookup_TCC/500                         372589 ns       323900 ns         2161
BM_FakeLua_HashLookup_TCC/1000                        787554 ns       655228 ns         1081
BM_FakeLua_HashLookup_GCC/100                          78154 ns        56178 ns        12749
BM_FakeLua_HashLookup_GCC/500                         381772 ns       283637 ns         2462
BM_FakeLua_HashLookup_GCC/1000                        652404 ns       560545 ns         1240
BM_CPP_NestedTable/1000                                83303 ns        61237 ns        11382
BM_CPP_NestedTable/10000                              750997 ns       608333 ns         1147
BM_Lua_NestedTable/1000                               332739 ns       291943 ns         2414
BM_Lua_NestedTable/10000                             3400317 ns      2887672 ns          241
BM_FakeLua_NestedTable_TCC/1000                       504239 ns       443705 ns         1571
BM_FakeLua_NestedTable_TCC/10000                     5211569 ns      4435698 ns          159
BM_FakeLua_NestedTable_GCC/1000                       116617 ns        87651 ns         8035
BM_FakeLua_NestedTable_GCC/10000                     1148141 ns       862546 ns          808
```
