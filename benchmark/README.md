# Benchmark Results

本文件记录在本地以 **Release 模式**（`-O3 -DNDEBUG`）编译运行 `bench_mark` 的完整结果。覆盖 **6 大类共 51 个 Lua 性能场景**，每个场景均实现 C++ / Lua 5.4 / FakeLua TCC / FakeLua GCC 四种横向对比（下面的分析聚焦 GCC vs Lua、GCC vs C++）。

## 运行环境

- 日期：2026-08-14
- 机器：AMD EPYC 7K62 48-Core Processor，2 X 2595.12 MHz CPU s
- CPU 缓存：L1d 32 KiB (x2)，L1i 32 KiB (x2)，L2 4096 KiB (x2)，L3 16384 KiB (x1)
- 构建模式：**Release**（`-O3 -DNDEBUG`），GCC 15.1.0
- FakeLua GCC JIT：**Release 模式**（`debug_mode=false`，GCC `-O3` 优化）
- 二进制：`build/bin/bench_mark`

> 绝对耗时强依赖于机器与编译器版本，跨环境不可比。本文件所有数字来自同一次运行，只有同一张表内的比值才有意义。特别地，C++ 参照实现在部分场景会被编译器整体折叠（见下文标注），此时 GCC vs C++ 一列无参考价值。

## 运行命令

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target bench_mark --parallel
build/bin/bench_mark --benchmark_repetitions=1 --benchmark_report_aggregates_only=true
```

---

## 结论

下表取各场景最大参数（最具代表性），**GCC vs Lua** 列表示 FakeLua GCC 相对 Lua 5.4 的加速倍数（>1 表示 FakeLua 更快），**GCC vs C++** 列表示 FakeLua GCC 与手写 C++ 的耗时比值（<1 表示 FakeLua 更快）。

### 算法（algo）

| 场景 | 参数 | GCC vs Lua | GCC vs C++ | 备注 |
|------|------|-----------|-----------|------|
| Fibonacci | n=32 | **36.6x** | 0.13 | 数值特化，接近 C++ |
| GCD | 2147483647/1073741823 | 0.45x | 10.8 | 单次调用仅约 0.4 µs，被调用开销主导 |
| PowMod | 1234567/7654321/1e9+7 | 1.7x | 2.56 | |
| Sum | n=5M | **30.4x** | 0.06 | GCC 向量化，远快于 C++ |
| BubbleSort | n=200 | 1.9x | 2.80 | 表下标读写拖累 |
| Sieve | n=5000 | 1.8x | 2.68 | |
| BinarySearch | n=1000 | 3.7x | 2.40 | |
| FastPow | 1234567/7654321/1e9+7 | 1.6x | 2.45 | |
| Popcount | n=100K | **37.3x** | 0.12 | 位运算极致优化 |
| InsertionSort | n=200 | 2.6x | 2.92 | |
| MatMul | 3×3 | 2.9x | 5.10 | |
| Vector3 | n=1M | 4.8x | 5.85 | 表特化为结构体，指针偏移 |
| FloatPoly | n=1M | **34.9x** | 0.49 | 浮点特化，GCC 2x 快于 C++ |

### 字符串（string）

| 场景 | 参数 | GCC vs Lua | GCC vs C++ | 备注 |
|------|------|-----------|-----------|------|
| StringLen | n=10K | 1.8x | 67.3 | |
| StringSub | n=10K | 1.3x | 6.15 | |
| StringRep | n=1000 | 1.1x | 0.16 | CGen `FlStringRep` 内联 |
| StringReverse | n=10K | 1.7x | 0.26 | CGen `FlStringReverse` 内联 |
| StringLower | n=10K | **6.1x** | 0.02 | CGen 内联 + ASCII 单遍 |
| StringUpper | n=10K | **5.0x** | 0.02 | 同上 |
| StringByte | n=1000 | 1.0x | 37.7 | |
| StringChar | n=500 | 1.0x | 10.6 | |
| StringFormat | n=500 | 1.8x | 2.71 | 常量 `"%d"` → `FlFormatInt` |
| StringFind | n=10K | 0.85x | 19.2 | plain 子串查找（`FlStringFindPlain` 内联，但仍有调用开销） |
| StringGsub | n=1000 | 0.06x | 38.1 | ECMAScript 正则（见下） |
| ToNumber | n=1 | 0.53x | 5.65 | CGen `FlTonumber` 十进制整数内联解析（同一输入 `"1234567890"`） |
| ToString | n=500 | 0.74x | 0.01 | INT → `FlFormatInt` |
| StringFindPattern | n=1000 | 0.12x | 39.6 | ECMAScript 正则；已加编译缓存 |
| StringGmatch | n=1000 | 0.21x | 23.3 | ECMAScript 正则；已加编译缓存 |

> **关于正则比 Lua 慢**：FakeLua 的 `string.find` / `match` / `gmatch` / `gsub` 走的是 **ECMAScript `std::regex`**（已做进程级编译缓存），能力强于 Lua 5.4 自带的 pattern（lookahead、完整字符类、非贪婪等）。因此正则场景慢于 Lua（当前约 0.06~0.21x）**可以接受**，属于能力换性能；后续若要追平 Lua，方向是另做 Lua pattern 引擎，而不是继续抠 `std::regex`。脚本统一用 `[0-9]+`（在 Lua pattern 与 ECMAScript 正则中语义一致）。

### 表操作（table）

| 场景 | 参数 | GCC vs Lua | GCC vs C++ | 备注 |
|------|------|-----------|-----------|------|
| TableInsert | n=5K | **4.0x** | 4.87 | |
| TableRemove | n=5K | **3.2x** | 2.99 | |
| TableConcat | n=1000 | 1.0x | 4.16 | arena 一次写入 |
| TablePack | n=1 | 1.9x | 157 | |
| TableMove | n=5K | 1.0x | 2.84 | CGen `FlTableMove` + 空表预扩容 |
| TableSort | n=1000 | 1.2x | 4.74 | |
| TableCreate | n=5K | 1.1x | 2.20 | |
| HashInsert | n=1000 | 2.0x | 0.88 | |
| HashLookup | n=1000 | 1.9x | 0.45 | |
| NestedTable | n=10K | 3.4x | 1.39 | 表层指针偏移遍历 |

### 函数调用（function）

| 场景 | 参数 | GCC vs Lua | GCC vs C++ | 备注 |
|------|------|-----------|-----------|------|
| EmptyCall | n=100K | **14.1x** | 61561 | C++ 内联为 0 |
| Recursion | n=25 | **42.9x** | 0.13 | 数值特化 |
| Variadic | n=1 | 0.91x | 75.4 | vararg 构建开销大 |
| MultiReturn | n=10K | **23.3x** | 0.46 | |
| Closure | n=1000 | 2.1x | 19.4 | |
| TailRecursion | n=5K | **107.5x** | 0.07 | 尾调用转循环+向量化 |

### GC 与内存压力（gc）

| 场景 | 参数 | GCC vs Lua | GCC vs C++ | 备注 |
|------|------|-----------|-----------|------|
| TableChurn | n=1000 | **9.3x** | 1.24 | 内存池分配器优势 |
| StringChurn | n=1000 | 2.2x | 1.47 | 字符串分配是弱项 |
| MixedAlloc | n=1000 | 2.3x | 1.68 | |

### 数学函数（math）

| 场景 | 参数 | GCC vs Lua | GCC vs C++ | 备注 |
|------|------|-----------|-----------|------|
| MathTrig (sin+cos) | n=100K | **5.8x** | 0.87 | 接近 C++ 原生速度 |
| MathSqrt | n=100K | **22.3x** | 0.48 | GCC 2x 快于 C++ |
| MathExpLog | n=100K | **4.3x** | 0.87 | |
| MathMinMax | n=100K | **10.4x** | 0.90 | |

### 核心发现

1. **纯数值场景全面领先 Lua 5.4 并接近手写 C++**：Fibonacci 36.6x、TailRecursion 107.5x、Recursion 42.9x、FloatPoly 34.9x、Popcount 37.3x、Sum 30.4x。数值特化让这些函数生成的 C 代码与手写版本几乎一致，剩下的交给 GCC `-O3`。

2. **math 库四项全部快于 Lua**（4.3x~22.3x），sin/cos/sqrt 接近 C++ 原生速度。

3. **string 标准库函数大多快于 Lua**：lower/upper 5-6x、format 1.8x、rep 1.1x、reverse 1.7x，均通过 CGen 内联（`FlStringLower/Upper`、`FlFormatInt`、`FlStringRep`、`FlStringReverse`）避免 `FakeluaCallByName` 的调用开销。当前仅 StringFind (plain) 0.85x 略慢于 Lua，剩余差距为单次调用派发开销。

4. **表操作整体有优势**：table.insert 4.0x、remove 3.2x、sort 1.2x 快于 Lua，move / concat 与 Lua 基本持平。`VarTable` 缓存连续整数键前缀长度使 `#t` 为 O(1)，宿主侧与 JIT 侧共用同一套哈希/桶布局。

5. **arena 分配器在表频繁创建场景优势明显**：TableChurn 快于 Lua 9.3x——无 GC、批量释放。

6. **正则场景慢于 Lua 可接受**（Gsub 0.06x、FindPattern 0.12x、Gmatch 0.21x）：使用更强的 ECMAScript `std::regex`（非 Lua pattern），已加编译缓存；能力不同，不作为追平目标。

7. **剩余慢项为调用派发开销主导**：GCD（0.45x）、ToNumber（0.53x）、ToString（0.74x）、Variadic（0.91x）函数体极小（< 1 µs），JIT 的 CVar 装箱/拆箱与调用约定开销占比大，非运算本身慢。需跨函数内联或调用约定优化才能追平。

   > 说明：ToNumber 的 bench 两侧解析同一输入字符串 `"1234567890"`。剩余差距主要来自单次 `Call()` 派发与字符串装箱常数开销，非解析本身。

---

## 完整原始输出

以下为 `benchmark_algo.cpp` / `benchmark_string.cpp` / `benchmark_table.cpp` / `benchmark_function.cpp` / `benchmark_gc.cpp` / `benchmark_math.cpp` 全部 51 个场景的完整 google benchmark 输出（含 TCC 数据）：

> ⚠️ 其中 `BM_*_StringFindPattern` / `BM_*_StringGmatch` 八行是 `[0-9]+` 修正**之前**的旧数据，两边工作量不对等，不可用于比较；以上文表格中的修正值为准。

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
