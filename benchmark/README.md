# Benchmark Results

本文件记录在本地以 **Release 模式**（`-O3 -DNDEBUG`）编译运行 `bench_mark` 的完整结果，以及对各算法的性能分析。

## 基准说明

### 算法对比（benchmark_algo.cpp）

将 C++、Lua 5.4、FakeLua（JIT_TCC / JIT_GCC）在同一文件中进行横向性能对比，覆盖 11 类算法：

| 算法 | 说明 | 参数规模 |
|------|------|---------|
| Fibonacci | 递归斐波那契（无记忆化） | n=20/25/30/32 |
| GCD | 欧几里得最大公约数 | 多组大整数对 |
| PowMod | 快速幂取模（用 `%`/`//`） | 多组底数/指数/模数 |
| Sum | 1..n 线性累加 | n=10000/100000/1000000/5000000 |
| BubbleSort | 冒泡排序（O(n²)，含表操作） | n=50/100/200 |
| Sieve | Eratosthenes 筛质数 | n=100/500/1000/5000 |
| BinarySearch | 二分查找（n 次） | n=100/500/1000 |
| FastPow | 快速幂取模（用 `&`/`>>`） | 多组底数/指数/模数 |
| Popcount | Brian Kernighan 位计数（求和） | n=1000/10000/100000 |
| InsertionSort | 插入排序（O(n²)，含表操作） | n=50/100/200 |
| MatMul | 单次 3×3 矩阵乘法（求迹） | 无参数，每次调用一次 |
| Vector3 | 三维坐标 x, y, z 在循环中累计读写 | n=10000/100000/1000000 |

---

## 运行环境

- 日期：2026-08-02
- 机器：2 X 2595.12 MHz CPU s
- CPU 缓存：L1d 32 KiB (x2)，L1i 32 KiB (x2)，L2 4096 KiB (x2)，L3 16384 KiB (x1)
- 构建模式：**Release**（`cmake .. -DCMAKE_BUILD_TYPE=Release`，最终编译标志 `-O3 -DNDEBUG`）
- FakeLua TCC JIT：**Release 模式**（`debug_mode=false`，TCC 启用 `-O2` 优化）
- FakeLua GCC JIT：**Release 模式**（`debug_mode=false`，GCC 启用 `-O3` 优化）
- 二进制：`build/bin/bench_mark`

## 运行命令

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
build/bin/bench_mark --benchmark_repetitions=1 --benchmark_report_aggregates_only=true
```

---

## 完整原始输出

```text
Starting benchmarks...
2026-08-02T19:08:19+08:00
Running build/bin/bench_mark
Run on (2 X 2595.12 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x2)
  L1 Instruction 32 KiB (x2)
  L2 Unified 4096 KiB (x2)
  L3 Unified 16384 KiB (x1)
Load Average: 1.59, 1.56, 0.80
***WARNING*** ASLR is enabled, the results may have unreproducible noise in them.
--------------------------------------------------------------------------------------------
Benchmark                                                  Time             CPU   Iterations
--------------------------------------------------------------------------------------------
BM_CPP_Fibonacci/20                                    15635 ns        15583 ns        46250
BM_CPP_Fibonacci/25                                   152267 ns       152113 ns         4699
BM_CPP_Fibonacci/30                                  1888682 ns      1883646 ns          378
BM_CPP_Fibonacci/32                                  4801340 ns      4797386 ns          150
BM_Lua_Fibonacci/20                                   639706 ns       638915 ns         1111
BM_Lua_Fibonacci/25                                  7036368 ns      7031955 ns           98
BM_Lua_Fibonacci/30                                 78677918 ns     78624190 ns            9
BM_Lua_Fibonacci/32                                206445190 ns    206203722 ns            3
BM_FakeLua_Fibonacci_TCC/20                            61500 ns        61459 ns        11608
BM_FakeLua_Fibonacci_TCC/25                           666136 ns       665558 ns         1038
BM_FakeLua_Fibonacci_TCC/30                          7472063 ns      7462124 ns           90
BM_FakeLua_Fibonacci_TCC/32                         20119442 ns     20047576 ns           35
BM_FakeLua_Fibonacci_GCC/20                            17172 ns        17153 ns        40404
BM_FakeLua_Fibonacci_GCC/25                           156009 ns       155910 ns         4536
BM_FakeLua_Fibonacci_GCC/30                          1703298 ns      1700907 ns          410
BM_FakeLua_Fibonacci_GCC/32                          4370245 ns      4366410 ns          164
BM_CPP_GCD/832040/514229                                 121 ns          121 ns      5732264
BM_CPP_GCD/123456789/987654321                          16.7 ns         16.7 ns     42009339
BM_CPP_GCD/2147483647/1073741823                        13.0 ns         13.0 ns     54043926
BM_Lua_GCD/832040/514229                                 501 ns          500 ns      1000000
BM_Lua_GCD/123456789/987654321                           124 ns          124 ns      5703864
BM_Lua_GCD/2147483647/1073741823                         107 ns          107 ns      6619970
BM_FakeLua_GCD_TCC/832040/514229                         250 ns          250 ns      3094509
BM_FakeLua_GCD_TCC/123456789/987654321                   124 ns          123 ns      5680009
BM_FakeLua_GCD_TCC/2147483647/1073741823                 122 ns          121 ns      5875098
BM_FakeLua_GCD_GCC/832040/514229                         202 ns          202 ns      3503856
BM_FakeLua_GCD_GCC/123456789/987654321                   106 ns          106 ns      6329455
BM_FakeLua_GCD_GCC/2147483647/1073741823                 104 ns          104 ns      6311426
BM_CPP_PowMod/2/1000/1000000007                          105 ns          105 ns      6719127
BM_CPP_PowMod/7/1000000/1000000007                       204 ns          204 ns      3427437
BM_CPP_PowMod/1234567/7654321/1000000007                 296 ns          296 ns      2361570
BM_Lua_PowMod/2/1000/1000000007                          474 ns          473 ns      1473092
BM_Lua_PowMod/7/1000000/1000000007                       853 ns          852 ns       821851
BM_Lua_PowMod/1234567/7654321/1000000007                1014 ns         1013 ns       685959
BM_FakeLua_PowMod_TCC/2/1000/1000000007                  326 ns          325 ns      2062032
BM_FakeLua_PowMod_TCC/7/1000000/1000000007               552 ns          552 ns      1253916
BM_FakeLua_PowMod_TCC/1234567/7654321/1000000007         702 ns          702 ns       991474
BM_FakeLua_PowMod_GCC/2/1000/1000000007                  222 ns          221 ns      3089493
BM_FakeLua_PowMod_GCC/7/1000000/1000000007               327 ns          327 ns      2113581
BM_FakeLua_PowMod_GCC/1234567/7654321/1000000007         426 ns          425 ns      1645901
BM_CPP_Sum/10000                                        3083 ns         3081 ns       227076
BM_CPP_Sum/100000                                      30901 ns        30868 ns        22661
BM_CPP_Sum/1000000                                    309196 ns       309003 ns         2266
BM_CPP_Sum/5000000                                   1546438 ns      1544860 ns          453
BM_Lua_Sum/10000                                       54110 ns        54075 ns        12546
BM_Lua_Sum/100000                                     548756 ns       548158 ns         1346
BM_Lua_Sum/1000000                                   5331634 ns      5325907 ns          129
BM_Lua_Sum/5000000                                  26798095 ns     26782335 ns           27
BM_FakeLua_Sum_TCC/10000                               19844 ns        19824 ns        54500
BM_FakeLua_Sum_TCC/100000                             255431 ns       255262 ns         2679
BM_FakeLua_Sum_TCC/1000000                           2126112 ns      2123329 ns          321
BM_FakeLua_Sum_TCC/5000000                          12636518 ns     12628908 ns           55
BM_FakeLua_Sum_GCC/10000                                3198 ns         3195 ns       219595
BM_FakeLua_Sum_GCC/100000                              31067 ns        31048 ns        22557
BM_FakeLua_Sum_GCC/1000000                            309593 ns       309236 ns         2264
BM_FakeLua_Sum_GCC/5000000                           1546852 ns      1545641 ns          453
BM_CPP_BubbleSort/50                                    7750 ns         7639 ns        92448
BM_CPP_BubbleSort/100                                  31084 ns        31059 ns        22534
BM_CPP_BubbleSort/200                                 125714 ns       125610 ns         5567
BM_Lua_BubbleSort/50                                   54119 ns        54057 ns        12762
BM_Lua_BubbleSort/100                                 212040 ns       211847 ns         3380
BM_Lua_BubbleSort/200                                 817055 ns       816011 ns          852
BM_FakeLua_BubbleSort_TCC/50                          114521 ns       114394 ns         6262
BM_FakeLua_BubbleSort_TCC/100                         444775 ns       444334 ns         1623
BM_FakeLua_BubbleSort_TCC/200                        1746415 ns      1744879 ns          405
BM_FakeLua_BubbleSort_GCC/50                           31890 ns        31847 ns        21867
BM_FakeLua_BubbleSort_GCC/100                         127221 ns       127111 ns         5593
BM_FakeLua_BubbleSort_GCC/200                         519207 ns       518505 ns         1390
BM_CPP_Sieve/100                                         256 ns          256 ns      2745646
BM_CPP_Sieve/500                                        1205 ns         1204 ns       575290
BM_CPP_Sieve/1000                                       2447 ns         2444 ns       280275
BM_CPP_Sieve/5000                                      12647 ns        12640 ns        55616
BM_Lua_Sieve/100                                        6803 ns         6794 ns       107474
BM_Lua_Sieve/500                                       27337 ns        27306 ns        26352
BM_Lua_Sieve/1000                                      51753 ns        51708 ns        10000
BM_Lua_Sieve/5000                                     265875 ns       265578 ns         2668
BM_FakeLua_Sieve_TCC/100                                9565 ns         9559 ns        71865
BM_FakeLua_Sieve_TCC/500                               50971 ns        50927 ns        10000
BM_FakeLua_Sieve_TCC/1000                              96332 ns        96211 ns         7432
BM_FakeLua_Sieve_TCC/5000                             578681 ns       578214 ns         1231
BM_FakeLua_Sieve_GCC/100                                2072 ns         2069 ns       343593
BM_FakeLua_Sieve_GCC/500                                9358 ns         9351 ns        71446
BM_FakeLua_Sieve_GCC/1000                              19528 ns        19517 ns        35739
BM_FakeLua_Sieve_GCC/5000                             124522 ns       124446 ns         5746
BM_CPP_BinarySearch/100                                  854 ns          853 ns       814327
BM_CPP_BinarySearch/500                                 6035 ns         6027 ns       115354
BM_CPP_BinarySearch/1000                               19027 ns        19008 ns        32366
BM_Lua_BinarySearch/100                                28437 ns        28402 ns        24281
BM_Lua_BinarySearch/500                               193592 ns       193469 ns         3517
BM_Lua_BinarySearch/1000                              437335 ns       436811 ns         1579
BM_FakeLua_BinarySearch_TCC/100                        40306 ns        40278 ns        17589
BM_FakeLua_BinarySearch_TCC/500                       283395 ns       282997 ns         2545
BM_FakeLua_BinarySearch_TCC/1000                      632304 ns       631917 ns         1100
BM_FakeLua_BinarySearch_GCC/100                         5924 ns         5919 ns       114774
BM_FakeLua_BinarySearch_GCC/500                        53551 ns        53493 ns        13328
BM_FakeLua_BinarySearch_GCC/1000                      128710 ns       128631 ns         5674
BM_CPP_FastPow/2/1000/1000000007                         105 ns          104 ns      6711579
BM_CPP_FastPow/7/1000000/1000000007                      204 ns          204 ns      3431512
BM_CPP_FastPow/1234567/7654321/1000000007                297 ns          296 ns      2356807
BM_Lua_FastPow/2/1000/1000000007                         425 ns          424 ns      1659666
BM_Lua_FastPow/7/1000000/1000000007                      757 ns          756 ns       934431
BM_Lua_FastPow/1234567/7654321/1000000007                889 ns          889 ns       773170
BM_FakeLua_FastPow_TCC/2/1000/1000000007                 229 ns          228 ns      3013074
BM_FakeLua_FastPow_TCC/7/1000000/1000000007              340 ns          340 ns      2064370
BM_FakeLua_FastPow_TCC/1234567/7654321/1000000007        433 ns          432 ns      1622204
BM_FakeLua_FastPow_GCC/2/1000/1000000007                 216 ns          216 ns      3235982
BM_FakeLua_FastPow_GCC/7/1000000/1000000007              318 ns          318 ns      2213632
BM_FakeLua_FastPow_GCC/1234567/7654321/1000000007        415 ns          415 ns      1699430
BM_CPP_Popcount/1000                                    3477 ns         3472 ns       206910
BM_CPP_Popcount/10000                                  39794 ns        39770 ns        17919
BM_CPP_Popcount/100000                                468521 ns       467981 ns         1484
BM_Lua_Popcount/1000                                   85875 ns        85824 ns         8362
BM_Lua_Popcount/10000                                1064557 ns      1063429 ns          659
BM_Lua_Popcount/100000                              13148758 ns     13137165 ns           56
BM_FakeLua_Popcount_TCC/1000                           12440 ns        12433 ns        57375
BM_FakeLua_Popcount_TCC/10000                         161567 ns       161372 ns         4562
BM_FakeLua_Popcount_TCC/100000                       2029707 ns      2028480 ns          343
BM_FakeLua_Popcount_GCC/1000                            3295 ns         3291 ns       210627
BM_FakeLua_Popcount_GCC/10000                          38455 ns        38422 ns        18951
BM_FakeLua_Popcount_GCC/100000                        436204 ns       435917 ns         1576
BM_CPP_InsertionSort/50                                  734 ns          734 ns       938614
BM_CPP_InsertionSort/100                                3181 ns         3178 ns       220391
BM_CPP_InsertionSort/200                               11601 ns        11593 ns        59893
BM_Lua_InsertionSort/50                                39213 ns        39161 ns        17507
BM_Lua_InsertionSort/100                              148986 ns       148820 ns         4723
BM_Lua_InsertionSort/200                              569398 ns       568720 ns         1167
BM_FakeLua_InsertionSort_TCC/50                        71522 ns        71473 ns         9988
BM_FakeLua_InsertionSort_TCC/100                      276817 ns       276639 ns         2517
BM_FakeLua_InsertionSort_TCC/200                     1083127 ns      1081968 ns          642
BM_FakeLua_InsertionSort_GCC/50                        13979 ns        13970 ns        49665
BM_FakeLua_InsertionSort_GCC/100                       53864 ns        53783 ns        12968
BM_FakeLua_InsertionSort_GCC/200                      200742 ns       200615 ns         3354
BM_CPP_MatMul                                           1.90 ns         1.90 ns    368088721
BM_Lua_MatMul                                           2780 ns         2779 ns       249699
BM_FakeLua_MatMul_TCC                                   2126 ns         2125 ns       334774
BM_FakeLua_MatMul_GCC                                    516 ns          516 ns      1375711
BM_CPP_Vector3/10000                                    4812 ns         4809 ns       142370
BM_CPP_Vector3/100000                                  48086 ns        48055 ns        14435
BM_CPP_Vector3/1000000                                481126 ns       480592 ns         1424
BM_Lua_Vector3/10000                                 1144099 ns      1143382 ns          608
BM_Lua_Vector3/100000                               11418599 ns     11410453 ns           60
BM_Lua_Vector3/1000000                             116910546 ns    116747545 ns            6
BM_FakeLua_Vector3_TCC/10000                         1373160 ns      1372328 ns          512
BM_FakeLua_Vector3_TCC/100000                       13706434 ns     13691575 ns           52
BM_FakeLua_Vector3_TCC/1000000                     136705941 ns    136618141 ns            5
BM_FakeLua_Vector3_GCC/10000                          236063 ns       235757 ns         3139
BM_FakeLua_Vector3_GCC/100000                        2312607 ns      2310603 ns          306
BM_FakeLua_Vector3_GCC/1000000                      22696590 ns     22675959 ns           31
```

---

## 算法性能分析（C++ vs Lua vs FakeLua TCC vs FakeLua GCC）

以下取各算法的典型参数做横向对比（CPU Time），**倍数 = 对应实现时间 / C++ 时间**：

### 1. Fibonacci（n=32，递归无记忆化）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 4.80 ms | 1x |
| Lua | 206.20 ms | **43.00x** 慢 |
| FakeLua TCC | 20.05 ms | **4.19x** 慢 |
| FakeLua GCC | 4.37 ms | **1.09x** 快 (比 C++ 快 **9%**) |

### 2. GCD（a=832040, b=514229，约 30 次迭代）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 121 ns | 1x |
| Lua | 500 ns | **4.14x** 慢 |
| FakeLua TCC | 250 ns | **2.07x** 慢 (比 Lua 快 **2.00x**) |
| FakeLua GCC | 202 ns | **1.67x** 慢 (比 Lua 快 **2.48x**) |

### 3. PowMod（base=1234567, exp=7654321, mod=1e9+7，约 23 次循环，用 `%`/`//`）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 296 ns | 1x |
| Lua | 1013 ns | **3.43x** 慢 |
| FakeLua TCC | 702 ns | **2.37x** 慢 (比 Lua 快 **1.44x**) |
| FakeLua GCC | 425 ns | **1.44x** 慢 (比 Lua 快 **2.38x**) |

### 4. FastPow（base=1234567, exp=7654321, mod=1e9+7，用 `&`/`>>`）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 297 ns | 1x |
| Lua | 889 ns | **2.99x** 慢 |
| FakeLua TCC | 432 ns | **1.46x** 慢 (比 Lua 快 **2.06x**) |
| FakeLua GCC | 415 ns | **1.40x** 慢 (比 Lua 快 **2.14x**) |

> FastPow 用位运算 `&`/`>>` 代替取余/整除 `%`/`//`，在 FakeLua TCC 下比 PowMod 快约 **1.6x**（702 ns → 432 ns），说明 TCC 对位运算的代码生成较优。GCC 两者表现也较接近。

### 5. Sum（n=5000000，纯循环累加）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 1.54 ms | 1x |
| Lua | 26.78 ms | **17.33x** 慢 |
| FakeLua TCC | 12.63 ms | **8.17x** 慢 (比 Lua 快 **2.12x**) |
| FakeLua GCC | 1.55 ms | **1.00x** 持平 |

> 纯整数累加循环：FakeLua GCC 与 C++ 几乎完全相同，说明 GCC `-O3` 对简单数值循环已达到 C++ 原生水平。TCC 比 Lua 快 **2.1x**。

### 6. BubbleSort（n=200，O(n²)，含表操作）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 125.7 µs | 1x |
| Lua | 816.0 µs | **6.50x** 慢 |
| FakeLua TCC | 1.74 ms | **13.89x** 慢 (比 Lua 慢 **2.14x**) |
| FakeLua GCC | 518.5 µs | **4.13x** 慢 (比 Lua 快 **1.57x**) |

> 表操作为瓶颈。TCC 比 Lua 慢约 2.1x，GCC 快于 Lua 约 1.6x。

### 7. Sieve（n=5000，Eratosthenes 筛质数）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 12.6 µs | 1x |
| Lua | 265.6 µs | **21.02x** 慢 |
| FakeLua TCC | 578.2 µs | **45.76x** 慢 (比 Lua 慢 **2.18x**) |
| FakeLua GCC | 124.4 µs | **9.85x** 慢 (比 Lua 快 **2.14x**) |

> 筛法涉及大量表写操作，TCC 表现最差；GCC 凭借优化仍快于 Lua 2.1x。

### 8. BinarySearch（n=1000，二分查找）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 19.0 µs | 1x |
| Lua | 436.8 µs | **22.98x** 慢 |
| FakeLua TCC | 631.9 µs | **33.23x** 慢 (比 Lua 慢 **1.45x**) |
| FakeLua GCC | 128.6 µs | **6.76x** 慢 (比 Lua 快 **3.40x**) |

### 9. Popcount（n=100000，Brian Kernighan 位计数）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 468.0 µs | 1x |
| Lua | 13.14 ms | **28.06x** 慢 |
| FakeLua TCC | 2.03 ms | **4.33x** 慢 (比 Lua 快 **6.48x**) |
| FakeLua GCC | 435.9 µs | **0.93x** 快 (比 C++ 快 **7%**) |

> 纯位运算场景：FakeLua GCC 略超 C++，比 Lua 快 **30.1x**，展示 GCC 对位运算的极致优化。

### 10. InsertionSort（n=200，O(n²)，含表操作）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 11.6 µs | 1x |
| Lua | 568.7 µs | **49.08x** 慢 |
| FakeLua TCC | 1.08 ms | **93.36x** 慢 (比 Lua 慢 **1.90x**) |
| FakeLua GCC | 200.6 µs | **17.30x** 慢 (比 Lua 快 **2.84x**) |

### 11. MatMul（单次 3×3 矩阵乘法，使用全局常量 Table 读）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 1.90 ns | 1x |
| Lua | 2.78 µs | **1463.16x** 慢 |
| FakeLua TCC | 2.13 µs | **1118.95x** 慢 (比 Lua 快 **1.31x**) |
| FakeLua GCC | 516 ns | **271.58x** 慢 (比 Lua 快 **5.39x**) |

> 将只读矩阵 `mat_a` 和 `mat_b` 移入全局/模块级常量，并启用 Table 特化后，**TCC 和 GCC-JIT 的性能均获得巨大突破**：
> - **TCC** 成功跑赢 Lua 5.4 解释器（快 **1.31x**）。
> - **GCC** 比 Lua 5.4 解释器快 **5.39x**。
>
> **实现细节剖析**：
> 需要注意的是，由于 `bench_matmul` 中的索引访问是动态表达式（如 `mat_a[(i - 1) * 3 + k]`），在生成的 C 代码中，并不能直接在调用处生成静态的指针偏移访问（如 `s->_int_1`）。它在 C 代码中仍然调用了 `FlGetTableInt`。
> 但由于 `mat_a` 和 `mat_b` 已经是特化 Table，`FlGetTableInt` 内部会优先通过其绑定的特化回调函数 `spec_get` 执行。在 `spec_get` 内部，系统执行 `if (__ival == 1) return s->_int_1;` 等键值匹配分支，最终映射到结构体成员的指针偏移。这种方式虽然含有分支判断开销，但比常规的哈希计算与哈希桶查找要高效得多。此外，将只读表定义于函数外部，彻底消除了每次函数调用时的 Table 重新分配与 GC 垃圾回收压力。

---

### 12. Vector3（三维坐标 x, y, z 在循环中累计读写）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 480.6 µs | 1x |
| Lua | 116.75 ms | **242.99x** 慢 |
| FakeLua TCC | 136.62 ms | **284.14x** 慢 (比 Lua 慢 **1.17x**) |
| FakeLua GCC | 22.68 ms | **47.17x** 慢 |

> **实现细节剖析**：
> 在本测试中，`v1` 和 `v2` 是通过字面量键定义的局部 Vector3 表 `{x=10, y=20, z=30}`。由于键 `x`, `y`, `z` 均为静态已知的字符串字面量，JIT 编译器**成功将该 Table 构造特化为了静态的 C 结构体**。
> 同时，在循环体内对 `v1.x = v1.x + v2.x` 等成员的读写都是静态已知的字符串 Key 访问，因此 JIT 编译器**直接将其编译为了零查找开销的静态指针偏移访问**（类似于 C 语言结构体成员的直接读写 `v1.spec->x = v1.spec->x + v2.spec->x`）。
> 相比之下，Lua 5.4 在执行这 100 万次循环时，必须进行总共 900 万次基于字符串哈希的 Table Lookup。实验表明，FakeLua GCC 比标准 Lua 5.4 **快 5.15 倍**，充分展示了 Table 结构体特化和直接指针偏移读写的强悍性能。

---

## FakeLua TCC vs Lua 5.4 详细对比

| 算法 | 参数 | Lua | FakeLua TCC | 结果 |
|------|------|-----|-------------|------|
| Fibonacci | n=32 | 206.20 ms | 20.05 ms | TCC **10.28x 快** |
| GCD | 832040/514229 | 500 ns | 250 ns | TCC **2.00x 快** |
| PowMod | 1234567/7654321/1e9+7 | 1013 ns | 702 ns | TCC **1.44x 快** |
| FastPow | 1234567/7654321/1e9+7 | 889 ns | 432 ns | TCC **2.06x 快** |
| Sum | n=5000000 | 26.78 ms | 12.63 ms | TCC **2.12x 快** |
| BubbleSort | n=200 | 816.0 µs | 1.74 ms | TCC **2.14x 慢** |
| Sieve | n=5000 | 265.6 µs | 578.2 µs | TCC **2.18x 慢** |
| BinarySearch | n=1000 | 436.8 µs | 631.9 µs | TCC **1.45x 慢** |
| Popcount | n=100000 | 13.14 ms | 2.03 ms | TCC **6.48x 快** |
| InsertionSort | n=200 | 568.7 µs | 1.08 ms | TCC **1.90x 慢** |
| MatMul | 单次 3×3 | 2.78 µs | 2.13 µs | TCC **1.31x 快** |
| Vector3 | n=1000000 | 116.75 ms | 136.62 ms | TCC **1.17x 慢** |

> **结论**：在将部分 Table 静态只读部分常量化后，TCC 在 MatMul 这样的运算中超越了 Lua；在纯数值算法上也保持 2.0x ~ 10.3x 的巨大优势。表写操作依然是 TCC 的软肋，但表读与遍历性能表现优异。

---

## FakeLua GCC vs Lua 5.4 详细对比

| 算法 | 参数 | Lua | FakeLua GCC | GCC 快多少 |
|------|------|-----|-------------|-----------|
| Fibonacci | n=32 | 206.20 ms | 4.37 ms | **47.19x** |
| GCD | 832040/514229 | 500 ns | 202 ns | **2.48x** |
| PowMod | 1234567/7654321/1e9+7 | 1013 ns | 425 ns | **2.38x** |
| FastPow | 1234567/7654321/1e9+7 | 889 ns | 415 ns | **2.14x** |
| Sum | n=5000000 | 26.78 ms | 1.55 ms | **17.32x** |
| BubbleSort | n=200 | 816.0 µs | 518.5 µs | **1.57x** |
| Sieve | n=5000 | 265.6 µs | 124.4 µs | **2.14x** |
| BinarySearch | n=1000 | 436.8 µs | 128.6 µs | **3.40x** |
| Popcount | n=100000 | 13.14 ms | 435.9 µs | **30.14x** |
| InsertionSort | n=200 | 568.7 µs | 200.6 µs | **2.84x** |
| MatMul | 单次 3×3 | 2.78 µs | 516 ns | **5.39x** |
| Vector3 | n=1000000 | 116.75 ms | 22.68 ms | **5.15x** |

### FakeLua GCC 按场景分类

| 场景类型 | FakeLua GCC 比 Lua 快 | 原因 |
|---------|----------------------|------|
| **纯整数累加 (Sum)** | **17.32x** | GCC `-O3` 向量化，达到 C++ 原生水平 |
| **纯整数位运算 (Popcount)** | **30.14x** | 位运算全部原生化，GCC 极进优化 |
| **递归 (Fibonacci)** | **47.19x** | 数值特化 + 原生递归，GCC 深度内联 |
| **算术循环 (PowMod/FastPow)** | **2.38x–2.14x** | 循环体数值特化，取模运算受益于寄存器优化 |
| **短迭代 (GCD)** | **2.48x** | 迭代次数少，函数调用开销占比高 |
| **二分查找 (BinarySearch)** | **3.40x** | 混合数值+表操作，GCC 部分消除 table 开销 |
| **表操作为主 (BubbleSort/InsertionSort/Sieve/MatMul/Vector3)** | **1.57x–5.39x** | 引入了 Table 结构体特化与读写，大幅提升读写效率 |

> **FakeLua GCC 后端在所有算法上均快于 Lua 5.4**（1.6x ~ 47.2x），特别是表常量化后，部分表操作场景的性能优势得到了显著提升。

---

## 总体结论

1. **C++ 最快**：在全部算法上领先，受益于 `-O3` 内联/展开/向量化。

2. **FakeLua GCC 全面超越 Lua，纯数值场景接近 C++ 原生**：
   - 纯整数运算（Sum）：GCC 与 C++ 完全持平，比 Lua 快 **17.3x**
   - 纯位运算（Popcount）：GCC 略超 C++，比 Lua 快 **30.1x**
   - 递归（Fibonacci）：比 Lua 快 **47.2x**，性能非常接近 C++（甚至略快 9%）
   - 算术循环（PowMod/FastPow）：比 Lua 快 **2.4–2.1x**
   - 表操作为主（BubbleSort/InsertionSort/Sieve/MatMul/Vector3）：比 Lua 快 **1.6–5.4x**

3. **FakeLua TCC 优缺点分明**：
   - 纯整数算法（Sum、Popcount、FastPow、Fibonacci）：比 Lua 快 **2.1x ~ 10.3x**
   - 表操作密集型算法（BubbleSort、Sieve、InsertionSort）：比 Lua 慢 **1.15x ~ 2.3x**，但只读静态 Table 的场景下已能够快于 Lua (**1.31x**)
   - TCC 生成的 C 代码对 table 读写路径较长，而 Lua 解释器对 table 操作深度优化，导致写密集表算法下 TCC 明显落后

4. **位运算 vs 取模**（FastPow `&`/`>>` vs PowMod `%`/`//`）：在 TCC 下位运算快（702 ns vs 432 ns），GCC 下两者也较接近，说明 FakeLua 已能对两种写法生成相近质量的代码。

> 注：ASLR 开启，结果有一定随机噪声；建议在 `--cpu-scaling-enabled=false` 环境下多重复后取均值。
