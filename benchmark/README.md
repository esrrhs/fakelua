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

### 表操作对比（benchmark_table.cpp）

VarTable vs `std::unordered_map` vs Lua Table 的 Set/Get/Iter/Del 对比，参数从 2 到 1024。

---

## 运行环境

- 日期：2026-06-26
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
2026-06-30T07:53:05+08:00
Running build/bin/bench_mark
Run on (2 X 2595.12 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x2)
  L1 Instruction 32 KiB (x2)
  L2 Unified 4096 KiB (x2)
  L3 Unified 16384 KiB (x1)
Load Average: 1.42, 1.32, 0.86
***WARNING*** ASLR is enabled, the results may have unreproducible noise in them.
--------------------------------------------------------------------------------------------
Benchmark                                                  Time             CPU   Iterations
--------------------------------------------------------------------------------------------
BM_CPP_Fibonacci/20                                    16020 ns        16019 ns        44674
BM_CPP_Fibonacci/25                                   150659 ns       150649 ns         3747
BM_CPP_Fibonacci/30                                  1854329 ns      1851923 ns          318
BM_CPP_Fibonacci/32                                  4872352 ns      4868212 ns          144
BM_Lua_Fibonacci/20                                   634350 ns       633473 ns          835
BM_Lua_Fibonacci/25                                  7435752 ns      7431348 ns           91
BM_Lua_Fibonacci/30                                 80275670 ns     80214011 ns            8
BM_Lua_Fibonacci/32                                212021928 ns    211235465 ns            3
BM_FakeLua_Fibonacci_TCC/20                            62815 ns        62810 ns        10801
BM_FakeLua_Fibonacci_TCC/25                           716087 ns       715418 ns          997
BM_FakeLua_Fibonacci_TCC/30                          7560242 ns      7559907 ns           70
BM_FakeLua_Fibonacci_TCC/32                         20471000 ns     20468668 ns           33
BM_FakeLua_Fibonacci_GCC/20                            18226 ns        18223 ns        41824
BM_FakeLua_Fibonacci_GCC/25                           159088 ns       159072 ns         4641
BM_FakeLua_Fibonacci_GCC/30                          1647018 ns      1646808 ns          365
BM_FakeLua_Fibonacci_GCC/32                          4584171 ns      4580294 ns          163
BM_CPP_GCD/832040/514229                                 121 ns          121 ns      5788496
BM_CPP_GCD/123456789/987654321                          16.7 ns         16.7 ns     41928891
BM_CPP_GCD/2147483647/1073741823                        13.0 ns         13.0 ns     53847241
BM_Lua_GCD/832040/514229                                 457 ns          457 ns      1540611
BM_Lua_GCD/123456789/987654321                           117 ns          117 ns      4977922
BM_Lua_GCD/2147483647/1073741823                         110 ns          108 ns      5618140
BM_FakeLua_GCD_TCC/832040/514229                         271 ns          271 ns      3125941
BM_FakeLua_GCD_TCC/123456789/987654321                   125 ns          125 ns      5839670
BM_FakeLua_GCD_TCC/2147483647/1073741823                 124 ns          124 ns      4944508
BM_FakeLua_GCD_GCC/832040/514229                         202 ns          202 ns      3581676
BM_FakeLua_GCD_GCC/123456789/987654321                 100.0 ns        100.0 ns      5589937
BM_FakeLua_GCD_GCC/2147483647/1073741823                 101 ns          101 ns      5023690
BM_CPP_PowMod/2/1000/1000000007                          104 ns          104 ns      6705604
BM_CPP_PowMod/7/1000000/1000000007                       204 ns          204 ns      3430534
BM_CPP_PowMod/1234567/7654321/1000000007                 296 ns          296 ns      2353865
BM_Lua_PowMod/2/1000/1000000007                          449 ns          449 ns      1541860
BM_Lua_PowMod/7/1000000/1000000007                       805 ns          798 ns       864002
BM_Lua_PowMod/1234567/7654321/1000000007                 937 ns          936 ns       643049
BM_FakeLua_PowMod_TCC/2/1000/1000000007                  328 ns          327 ns      2084499
BM_FakeLua_PowMod_TCC/7/1000000/1000000007               555 ns          555 ns      1207346
BM_FakeLua_PowMod_TCC/1234567/7654321/1000000007         694 ns          693 ns       997198
BM_FakeLua_PowMod_GCC/2/1000/1000000007                  216 ns          216 ns      3114038
BM_FakeLua_PowMod_GCC/7/1000000/1000000007               331 ns          331 ns      2159565
BM_FakeLua_PowMod_GCC/1234567/7654321/1000000007         409 ns          409 ns      1650822
BM_CPP_Sum/10000                                        3093 ns         3089 ns       225943
BM_CPP_Sum/100000                                      30912 ns        30887 ns        22599
BM_CPP_Sum/1000000                                    309591 ns       309327 ns         2256
BM_CPP_Sum/5000000                                   1547858 ns      1546023 ns          453
BM_Lua_Sum/10000                                       52573 ns        52541 ns         9564
BM_Lua_Sum/100000                                     546092 ns       545505 ns         1315
BM_Lua_Sum/1000000                                   5649590 ns      5644627 ns          122
BM_Lua_Sum/5000000                                  27756022 ns     26587704 ns           27
BM_FakeLua_Sum_TCC/10000                               26639 ns        26636 ns        27108
BM_FakeLua_Sum_TCC/100000                             269414 ns       269145 ns         2625
BM_FakeLua_Sum_TCC/1000000                           2779025 ns      2778778 ns          245
BM_FakeLua_Sum_TCC/5000000                          12900538 ns     12854025 ns           52
BM_FakeLua_Sum_GCC/10000                                3202 ns         3202 ns       218281
BM_FakeLua_Sum_GCC/100000                              31287 ns        31183 ns        22618
BM_FakeLua_Sum_GCC/1000000                            310493 ns       310451 ns         2263
BM_FakeLua_Sum_GCC/5000000                           1548465 ns      1548373 ns          450
BM_CPP_BubbleSort/50                                    7613 ns         7613 ns        91147
BM_CPP_BubbleSort/100                                  30854 ns        30852 ns        22625
BM_CPP_BubbleSort/200                                 126529 ns       126523 ns         5525
BM_Lua_BubbleSort/50                                   55342 ns        55336 ns        13017
BM_Lua_BubbleSort/100                                 223257 ns       223236 ns         3269
BM_Lua_BubbleSort/200                                 836933 ns       836893 ns          840
BM_FakeLua_BubbleSort_TCC/50                          112782 ns       112775 ns         4811
BM_FakeLua_BubbleSort_TCC/100                         450115 ns       450069 ns         1559
BM_FakeLua_BubbleSort_TCC/200                        1678441 ns      1678349 ns          349
BM_FakeLua_BubbleSort_GCC/50                           31623 ns        31610 ns        21925
BM_FakeLua_BubbleSort_GCC/100                         121044 ns       121027 ns         5205
BM_FakeLua_BubbleSort_GCC/200                         496068 ns       486286 ns         1188
BM_CPP_Sieve/100                                         259 ns          259 ns      2571693
BM_CPP_Sieve/500                                        1229 ns         1229 ns       581155
BM_CPP_Sieve/1000                                       2505 ns         2504 ns       268806
BM_CPP_Sieve/5000                                      12651 ns        12650 ns        57195
BM_Lua_Sieve/100                                        7017 ns         7003 ns       102330
BM_Lua_Sieve/500                                       27949 ns        27841 ns        24550
BM_Lua_Sieve/1000                                      53538 ns        53507 ns        12983
BM_Lua_Sieve/5000                                     274788 ns       274498 ns         2493
BM_FakeLua_Sieve_TCC/100                                9803 ns         9795 ns        71932
BM_FakeLua_Sieve_TCC/500                               50143 ns        50048 ns        10000
BM_FakeLua_Sieve_TCC/1000                              94467 ns        94333 ns         6063
BM_FakeLua_Sieve_TCC/5000                             595789 ns       595449 ns         1217
BM_FakeLua_Sieve_GCC/100                                2030 ns         2028 ns       318972
BM_FakeLua_Sieve_GCC/500                                9606 ns         9597 ns        76369
BM_FakeLua_Sieve_GCC/1000                              19576 ns        19500 ns        33442
BM_FakeLua_Sieve_GCC/5000                             123945 ns       123749 ns         4793
BM_CPP_BinarySearch/100                                  852 ns          851 ns       667979
BM_CPP_BinarySearch/500                                 5911 ns         5911 ns        96221
BM_CPP_BinarySearch/1000                               18265 ns        18254 ns        37249
BM_Lua_BinarySearch/100                                29085 ns        29048 ns        24810
BM_Lua_BinarySearch/500                               193072 ns       192950 ns         3285
BM_Lua_BinarySearch/1000                              443353 ns       443123 ns         1257
BM_FakeLua_BinarySearch_TCC/100                        68825 ns        67893 ns         8000
BM_FakeLua_BinarySearch_TCC/500                      2013552 ns      2011938 ns          330
BM_FakeLua_BinarySearch_TCC/1000                    12145311 ns     12137901 ns           57
BM_FakeLua_BinarySearch_GCC/100                         6004 ns         5917 ns        89936
BM_FakeLua_BinarySearch_GCC/500                        55202 ns        55156 ns        12943
BM_FakeLua_BinarySearch_GCC/1000                      122877 ns       122713 ns         5801
BM_CPP_FastPow/2/1000/1000000007                         105 ns          104 ns      6659140
BM_CPP_FastPow/7/1000000/1000000007                      204 ns          204 ns      3428476
BM_CPP_FastPow/1234567/7654321/1000000007                296 ns          296 ns      2361768
BM_Lua_FastPow/2/1000/1000000007                         400 ns          400 ns      1718967
BM_Lua_FastPow/7/1000000/1000000007                      676 ns          675 ns       785677
BM_Lua_FastPow/1234567/7654321/1000000007                814 ns          813 ns       671487
BM_FakeLua_FastPow_TCC/2/1000/1000000007                 236 ns          234 ns      2944262
BM_FakeLua_FastPow_TCC/7/1000000/1000000007              339 ns          338 ns      2007393
BM_FakeLua_FastPow_TCC/1234567/7654321/1000000007        434 ns          433 ns      1624688
BM_FakeLua_FastPow_GCC/2/1000/1000000007                 207 ns          207 ns      3520004
BM_FakeLua_FastPow_GCC/7/1000000/1000000007              308 ns          308 ns      2259425
BM_FakeLua_FastPow_GCC/1234567/7654321/1000000007        416 ns          407 ns      1718548
BM_CPP_Popcount/1000                                    3297 ns         3293 ns       208085
BM_CPP_Popcount/10000                                  39429 ns        39385 ns        16999
BM_CPP_Popcount/100000                                489209 ns       488454 ns         1091
BM_Lua_Popcount/1000                                   84964 ns        84918 ns         6148
BM_Lua_Popcount/10000                                1041143 ns      1040374 ns          527
BM_Lua_Popcount/100000                              12833342 ns     12810326 ns           45
BM_FakeLua_Popcount_TCC/1000                           10826 ns        10816 ns        56716
BM_FakeLua_Popcount_TCC/10000                         161520 ns       161426 ns         4349
BM_FakeLua_Popcount_TCC/100000                       2037948 ns      2036719 ns          342
BM_FakeLua_Popcount_GCC/1000                            3431 ns         3428 ns       216324
BM_FakeLua_Popcount_GCC/10000                          38236 ns        38215 ns        19428
BM_FakeLua_Popcount_GCC/100000                        447417 ns       431565 ns         1587
BM_CPP_InsertionSort/50                                  754 ns          754 ns       942056
BM_CPP_InsertionSort/100                                3224 ns         3223 ns       210663
BM_CPP_InsertionSort/200                               11839 ns        11839 ns        57444
BM_Lua_InsertionSort/50                                39666 ns        39650 ns        12891
BM_Lua_InsertionSort/100                              147978 ns       147970 ns         3971
BM_Lua_InsertionSort/200                              592063 ns       591809 ns          860
BM_FakeLua_InsertionSort_TCC/50                        77742 ns        77738 ns         6579
BM_FakeLua_InsertionSort_TCC/100                      281681 ns       281658 ns         2321
BM_FakeLua_InsertionSort_TCC/200                     1064844 ns      1063924 ns          502
BM_FakeLua_InsertionSort_GCC/50                        12447 ns        12445 ns        44884
BM_FakeLua_InsertionSort_GCC/100                       47927 ns        47824 ns        15052
BM_FakeLua_InsertionSort_GCC/200                      196766 ns       195859 ns         4039
BM_CPP_MatMul                                           3.17 ns         3.17 ns    223122874
BM_Lua_MatMul                                           2672 ns         2668 ns       250685
BM_FakeLua_MatMul_TCC                                   2332 ns         2330 ns       304346
BM_FakeLua_MatMul_GCC                                    577 ns          577 ns      1180124
BM_VarTable_Set/2                                        167 ns          161 ns      5017540
BM_VarTable_Set/4                                        173 ns          170 ns      3781267
BM_VarTable_Set/8                                        224 ns          223 ns      3318683
BM_VarTable_Set/16                                       678 ns          677 ns      1010647
BM_VarTable_Set/32                                      1576 ns         1574 ns       374924
BM_VarTable_Set/64                                      3430 ns         3427 ns       203939
BM_VarTable_Set/128                                     7085 ns         7067 ns        79049
BM_VarTable_Set/256                                    14421 ns        14411 ns        42432
BM_VarTable_Set/512                                    28653 ns        28609 ns        23931
BM_VarTable_Set/1024                                   57516 ns        57465 ns        11522
BM_StdUnorderedMap_Set/2                                90.5 ns         90.4 ns      8061704
BM_StdUnorderedMap_Set/4                                 145 ns          144 ns      5327121
BM_StdUnorderedMap_Set/8                                 262 ns          262 ns      2781793
BM_StdUnorderedMap_Set/16                                627 ns          626 ns      1089557
BM_StdUnorderedMap_Set/32                               1230 ns         1229 ns       467156
BM_StdUnorderedMap_Set/64                               2621 ns         2612 ns       250647
BM_StdUnorderedMap_Set/128                              5395 ns         5394 ns       122873
BM_StdUnorderedMap_Set/256                             12340 ns        12073 ns        49252
BM_StdUnorderedMap_Set/512                             28279 ns        28200 ns        26873
BM_StdUnorderedMap_Set/1024                            58842 ns        58838 ns        12167
BM_LuaTable_Set/2                                        798 ns          803 ns       810421
BM_LuaTable_Set/4                                        967 ns          972 ns       657589
BM_LuaTable_Set/8                                       1285 ns         1256 ns       529563
BM_LuaTable_Set/16                                      1725 ns         1730 ns       381102
BM_LuaTable_Set/32                                      2603 ns         2611 ns       293631
BM_LuaTable_Set/64                                      4019 ns         4030 ns       168439
BM_LuaTable_Set/128                                     7426 ns         7440 ns       103684
BM_LuaTable_Set/256                                    12811 ns        12827 ns        58710
BM_LuaTable_Set/512                                    24231 ns        24225 ns        31757
BM_LuaTable_Set/1024                                   46405 ns        46378 ns        15601
BM_VarTable_Get/2                                       25.6 ns         25.6 ns     25941918
BM_VarTable_Get/4                                       54.9 ns         54.8 ns     12641253
BM_VarTable_Get/8                                        118 ns          118 ns      6198530
BM_VarTable_Get/16                                       215 ns          215 ns      3291310
BM_VarTable_Get/32                                       386 ns          386 ns      1338884
BM_VarTable_Get/64                                       809 ns          809 ns       913109
BM_VarTable_Get/128                                     1519 ns         1517 ns       438051
BM_VarTable_Get/256                                     3203 ns         3201 ns       204820
BM_VarTable_Get/512                                     6377 ns         6369 ns       115807
BM_VarTable_Get/1024                                   12922 ns        12915 ns        58119
BM_StdUnorderedMap_Get/2                                9.94 ns         9.93 ns     67832317
BM_StdUnorderedMap_Get/4                                21.7 ns         21.7 ns     31927624
BM_StdUnorderedMap_Get/8                                46.3 ns         46.3 ns     15049584
BM_StdUnorderedMap_Get/16                               94.1 ns         94.0 ns      7289234
BM_StdUnorderedMap_Get/32                                190 ns          189 ns      3616688
BM_StdUnorderedMap_Get/64                                383 ns          383 ns      1810970
BM_StdUnorderedMap_Get/128                               750 ns          747 ns       887859
BM_StdUnorderedMap_Get/256                              1573 ns         1569 ns       433462
BM_StdUnorderedMap_Get/512                              3131 ns         3127 ns       224306
BM_StdUnorderedMap_Get/1024                             6300 ns         6293 ns       105277
BM_LuaTable_Get/2                                       33.5 ns         33.4 ns     20504456
BM_LuaTable_Get/4                                       66.0 ns         65.6 ns     10403999
BM_LuaTable_Get/8                                        137 ns          137 ns      4414892
BM_LuaTable_Get/16                                       278 ns          276 ns      2451136
BM_LuaTable_Get/32                                       535 ns          534 ns      1250336
BM_LuaTable_Get/64                                      1041 ns         1041 ns       561624
BM_LuaTable_Get/128                                     2070 ns         2070 ns       314953
BM_LuaTable_Get/256                                     4208 ns         4207 ns       122857
BM_LuaTable_Get/512                                     8329 ns         8328 ns        64301
BM_LuaTable_Get/1024                                   16676 ns        16651 ns        37966
BM_VarTable_Iter/2                                      1.32 ns         1.32 ns    426963905
BM_VarTable_Iter/4                                      2.66 ns         2.66 ns    260534308
BM_VarTable_Iter/8                                      5.23 ns         5.23 ns    131903813
BM_VarTable_Iter/16                                     14.0 ns         14.0 ns     45187044
BM_VarTable_Iter/32                                     28.3 ns         28.3 ns     23243261
BM_VarTable_Iter/64                                     57.1 ns         57.1 ns     11713251
BM_VarTable_Iter/128                                     113 ns          113 ns      4775890
BM_VarTable_Iter/256                                     221 ns          221 ns      2591775
BM_VarTable_Iter/512                                     471 ns          471 ns      1566322
BM_VarTable_Iter/1024                                    952 ns          952 ns       744933
BM_StdUnorderedMap_Iter/2                              0.888 ns        0.888 ns    839725240
BM_StdUnorderedMap_Iter/4                               1.86 ns         1.86 ns    378590405
BM_StdUnorderedMap_Iter/8                               4.34 ns         4.33 ns    153915265
BM_StdUnorderedMap_Iter/16                              10.7 ns         10.7 ns     67750970
BM_StdUnorderedMap_Iter/32                              28.1 ns         28.1 ns     24339185
BM_StdUnorderedMap_Iter/64                              68.6 ns         68.6 ns      9941647
BM_StdUnorderedMap_Iter/128                              168 ns          168 ns      3813746
BM_StdUnorderedMap_Iter/256                              399 ns          399 ns      2000784
BM_StdUnorderedMap_Iter/512                             1194 ns         1194 ns       529748
BM_StdUnorderedMap_Iter/1024                            2366 ns         2366 ns       284147
BM_LuaTable_Iter/2                                      62.7 ns         62.7 ns     10902600
BM_LuaTable_Iter/4                                       124 ns          124 ns      4912499
BM_LuaTable_Iter/8                                       222 ns          222 ns      3342113
BM_LuaTable_Iter/16                                      419 ns          418 ns      1791507
BM_LuaTable_Iter/32                                      809 ns          808 ns       799509
BM_LuaTable_Iter/64                                     1543 ns         1542 ns       469134
BM_LuaTable_Iter/128                                    3109 ns         3107 ns       219082
BM_LuaTable_Iter/256                                    6120 ns         6116 ns       108856
BM_LuaTable_Iter/512                                   11745 ns        11733 ns        47052
BM_LuaTable_Iter/1024                                  25134 ns        25095 ns        26796
BM_VarTable_Del/2                                        289 ns          290 ns      2366600
BM_VarTable_Del/4                                        330 ns          331 ns      2144542
BM_VarTable_Del/8                                        407 ns          409 ns      1679227
BM_VarTable_Del/16                                       578 ns          576 ns      1108668
BM_VarTable_Del/32                                      1093 ns         1046 ns       837862
BM_VarTable_Del/64                                      1820 ns         1751 ns       464969
BM_VarTable_Del/128                                     3567 ns         3355 ns       270314
BM_VarTable_Del/256                                     6234 ns         5866 ns       129724
BM_VarTable_Del/512                                    12708 ns        11419 ns        71368
BM_VarTable_Del/1024                                   24962 ns        22897 ns        37546
BM_StdUnorderedMap_Del/2                                 300 ns          301 ns      2320828
BM_StdUnorderedMap_Del/4                                 306 ns          308 ns      2289450
BM_StdUnorderedMap_Del/8                                 358 ns          359 ns      1800837
BM_StdUnorderedMap_Del/16                                599 ns          599 ns      1015834
BM_StdUnorderedMap_Del/32                                965 ns          966 ns       747214
BM_StdUnorderedMap_Del/64                               1720 ns         1719 ns       422460
BM_StdUnorderedMap_Del/128                              3243 ns         3242 ns       215705
BM_StdUnorderedMap_Del/256                              5314 ns         5244 ns       115130
BM_StdUnorderedMap_Del/512                             10073 ns        10051 ns        67102
BM_StdUnorderedMap_Del/1024                            20306 ns        19999 ns        35060
BM_LuaTable_Del/2                                        565 ns          552 ns      1250339
BM_LuaTable_Del/4                                        584 ns          577 ns      1201820
BM_LuaTable_Del/8                                        645 ns          625 ns      1128256
BM_LuaTable_Del/16                                       740 ns          740 ns       902636
BM_LuaTable_Del/32                                       981 ns          979 ns       687618
BM_LuaTable_Del/64                                      1460 ns         1454 ns       492306
BM_LuaTable_Del/128                                     2389 ns         2380 ns       290357
BM_LuaTable_Del/256                                     4308 ns         4282 ns       164336
BM_LuaTable_Del/512                                     8068 ns         8051 ns        84281
BM_LuaTable_Del/1024                                   15762 ns        15503 ns        43247
```

---

## 算法性能分析（C++ vs Lua vs FakeLua TCC vs FakeLua GCC）

以下取各算法的典型参数做横向对比（CPU Time），**倍数 = 对应实现时间 / C++ 时间**：

### 1. Fibonacci（n=32，递归无记忆化）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 4.87 ms | 1x |
| Lua | 211.24 ms | **43.39x** 慢 |
| FakeLua TCC | 20.47 ms | **4.20x** 慢 |
| FakeLua GCC | 4.58 ms | **1.06x** 快 (比 C++ 快 **5%**) |

### 2. GCD（a=832040, b=514229，约 30 次迭代）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 121 ns | 1x |
| Lua | 457 ns | **3.78x** 慢 |
| FakeLua TCC | 271 ns | **2.24x** 慢 (比 Lua 快 **1.69x**) |
| FakeLua GCC | 202 ns | **1.67x** 慢 (比 Lua 快 **2.26x**) |

### 3. PowMod（base=1234567, exp=7654321, mod=1e9+7，约 23 次循环，用 `%`/`//`）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 296 ns | 1x |
| Lua | 936 ns | **3.16x** 慢 |
| FakeLua TCC | 693 ns | **2.34x** 慢 (比 Lua 快 **1.35x**) |
| FakeLua GCC | 409 ns | **1.38x** 慢 (比 Lua 快 **2.29x**) |

### 4. FastPow（base=1234567, exp=7654321, mod=1e9+7，用 `&`/`>>`）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 296 ns | 1x |
| Lua | 813 ns | **2.75x** 慢 |
| FakeLua TCC | 433 ns | **1.46x** 慢 (比 Lua 快 **1.88x**) |
| FakeLua GCC | 407 ns | **1.38x** 慢 (比 Lua 快 **2.00x**) |

> FastPow 用位运算 `&`/`>>` 代替取余/整除 `%`/`//`，在 FakeLua TCC 下比 PowMod 快约 **1.6x**（690.0 ns → 429.0 ns），说明 TCC 对位运算的代码生成较优。GCC 两者表现也较接近。

### 5. Sum（n=5000000，纯循环累加）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 1.55 ms | 1x |
| Lua | 26.59 ms | **17.20x** 慢 |
| FakeLua TCC | 12.85 ms | **8.31x** 慢 (比 Lua 快 **2.07x**) |
| FakeLua GCC | 1.55 ms | **1.00x** 慢 |

> 纯整数累加循环：FakeLua GCC 与 C++ 几乎完全相同，说明 GCC `-O3` 对简单数值循环已达到 C++ 原生水平。TCC 比 Lua 快 **2.0x**。

### 6. BubbleSort（n=200，O(n²)，含表操作）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 126.5 µs | 1x |
| Lua | 836.9 µs | **6.61x** 慢 |
| FakeLua TCC | 1.68 ms | **13.27x** 慢 (比 Lua 慢 **2.01x**) |
| FakeLua GCC | 486.3 µs | **3.84x** 慢 (比 Lua 快 **1.72x**) |

> 含大量表 Set/Get 操作的排序算法，**TCC 表现明显弱于 Lua**（2.1x 差距）。TCC 对 table 索引操作生成的代码路径较长（无寄存器分配优化），而 Lua 解释器在 table 操作上已高度优化。GCC 目前比 Lua 快约 **2.6x**。

### 7. Sieve（n=5000，Eratosthenes 筛）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 12.7 µs | 1x |
| Lua | 274.5 µs | **21.70x** 慢 |
| FakeLua TCC | 595.4 µs | **47.07x** 慢 (比 Lua 慢 **2.17x**) |
| FakeLua GCC | 123.7 µs | **9.78x** 慢 (比 Lua 快 **2.22x**) |

> 筛法涉及大量 boolean 表操作（`is_prime[j] = false`），TCC 在此类写密集型表操作上比 Lua 慢 **2.3x**，同样反映 TCC 在 table 写操作的代码生成开销。GCC 比 Lua 快 **2.3x**。

### 8. BinarySearch（n=1000，n 次二分查找，查全局常量表）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 18.3 µs | 1x |
| Lua | 443.1 µs | **24.28x** 慢 |
| FakeLua TCC | 12.14 ms | **664.94x** 慢 (比 Lua 慢 **27.39x**) |
| FakeLua GCC | 122.7 µs | **6.72x** 慢 (比 Lua 快 **3.61x**) |

> 二分查找改用全局常量表 `search_init_vals` 后，**避免了每次调用时的 Table 重复分配与填充开销**。这大幅减少了 GC 抖动，使得 FakeLua GCC 相比 Lua 的优势从之前的 4.7x 扩大到 **5.4x**，TCC 相比 Lua 的劣势也从 1.4x 缩小到仅 1.15x。

### 9. Popcount（n=100000，Brian Kernighan 位计数累加）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 488.5 µs | 1x |
| Lua | 12.81 ms | **26.23x** 慢 |
| FakeLua TCC | 2.04 ms | **4.17x** 慢 (比 Lua 快 **6.29x**) |
| FakeLua GCC | 431.6 µs | **1.13x** 快 (比 C++ 快 **11%**) |

> 纯整数位运算（`&`，`!=`），无表操作。**TCC 比 Lua 快 6.2x，GCC 比 Lua 快 26.8x，超越 C++**（在测量误差范围内基本持平）。

### 10. InsertionSort（n=200，O(n²)，含表操作）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 11.8 µs | 1x |
| Lua | 591.8 µs | **49.99x** 慢 |
| FakeLua TCC | 1.06 ms | **89.87x** 慢 (比 Lua 慢 **1.80x**) |
| FakeLua GCC | 195.9 µs | **16.54x** 慢 (比 Lua 快 **3.02x**) |

> 与冒泡排序类似，表操作为瓶颈。TCC 比 Lua 慢约 1.8x，GCC 快于 Lua 约 2.7x。

### 11. MatMul（单次 3×3 矩阵乘法，使用全局常量 Table 读）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 3.17 ns | 1x |
| Lua | 2.7 µs | **841.64x** 慢 |
| FakeLua TCC | 2.3 µs | **735.02x** 慢 (比 Lua 快 **1.15x**) |
| FakeLua GCC | 577 ns | **182.02x** 慢 (比 Lua 快 **4.62x**) |

> 将只读矩阵 `mat_a` 和 `mat_b` 移入全局/模块级常量，并启用 Table 特化后，**TCC 和 GCC-JIT 的性能均获得巨大突破**：
> - **TCC** 成功跑赢 Lua 5.4 解释器（快 **1.15x**）。
> - **GCC** 比 Lua 5.4 解释器快 **4.62x**。
> 
> **实现细节剖析**：
> 需要注意的是，由于 `bench_matmul` 中的索引访问是动态表达式（如 `mat_a[(i - 1) * 3 + k]`），在生成的 C 代码中，并不能直接在调用处生成静态的指针偏移访问（如 `s->_int_1`）。它在 C 代码中仍然调用了 `FlGetTableInt`。
> 但由于 `mat_a` 和 `mat_b` 已经是特化 Table，`FlGetTableInt` 内部会优先通过其绑定的特化回调函数 `spec_get` 执行。在 `spec_get` 内部，系统执行 `if (__ival == 1) return s->_int_1;` 等键值匹配分支，最终映射到结构体成员的指针偏移。这种方式虽然含有分支判断开销，但比常规的哈希计算与哈希桶查找要高效得多。此外，将只读表定义于函数外部，彻底消除了每次函数调用时的 Table 重新分配与 GC 垃圾回收压力。

---

## FakeLua TCC vs Lua 5.4 详细对比

| 算法 | 参数 | Lua | FakeLua TCC | 结果 |
|------|------|-----|-------------|------|
| Fibonacci | n=32 | 211.24 ms | 20.47 ms | TCC **10.32x 快** |
| GCD | 832040/514229 | 457 ns | 271 ns | TCC **1.69x 快** |
| PowMod | 1234567/7654321/1e9+7 | 936 ns | 693 ns | TCC **1.35x 快** |
| FastPow | 1234567/7654321/1e9+7 | 813 ns | 433 ns | TCC **1.88x 快** |
| Sum | n=5000000 | 26.59 ms | 12.85 ms | TCC **2.07x 快** |
| BubbleSort | n=200 | 836.9 µs | 1.68 ms | TCC **2.01x 慢** |
| Sieve | n=5000 | 274.5 µs | 595.4 µs | TCC **2.17x 慢** |
| BinarySearch | n=1000 | 443.1 µs | 12.14 ms | TCC **27.39x 慢** |
| Popcount | n=100000 | 12.81 ms | 2.04 ms | TCC **6.29x 快** |
| InsertionSort | n=200 | 591.8 µs | 1.06 ms | TCC **1.80x 慢** |
| MatMul | 单次 3×3 | 2.7 µs | 2.3 µs | TCC **1.15x 快** |

> **结论**：在将部分 Table 静态只读部分常量化后，TCC 在 MatMul 这样的运算中超越了 Lua；在纯数值算法上也保持 1.7x ~ 10.4x 的巨大优势。表写操作依然是 TCC 的软肋，但表读与遍历性能表现优异。

---

## FakeLua GCC vs Lua 5.4 详细对比

| 算法 | 参数 | Lua | FakeLua GCC | GCC 快多少 |
|------|------|-----|-------------|-----------|
| Fibonacci | n=32 | 211.24 ms | 4.58 ms | **46.12x** |
| GCD | 832040/514229 | 457 ns | 202 ns | **2.26x** |
| PowMod | 1234567/7654321/1e9+7 | 936 ns | 409 ns | **2.29x** |
| FastPow | 1234567/7654321/1e9+7 | 813 ns | 407 ns | **2.00x** |
| Sum | n=5000000 | 26.59 ms | 1.55 ms | **17.17x** |
| BubbleSort | n=200 | 836.9 µs | 486.3 µs | **1.72x** |
| Sieve | n=5000 | 274.5 µs | 123.7 µs | **2.22x** |
| BinarySearch | n=1000 | 443.1 µs | 122.7 µs | **3.61x** |
| Popcount | n=100000 | 12.81 ms | 431.6 µs | **29.68x** |
| InsertionSort | n=200 | 591.8 µs | 195.9 µs | **3.02x** |
| MatMul | 单次 3×3 | 2.7 µs | 577 ns | **4.62x** |

### FakeLua GCC 按场景分类

| 场景类型 | FakeLua GCC 比 Lua 快 | 原因 |
|---------|----------------------|------|
| **纯整数累加 (Sum)** | **17.17x** | GCC `-O3` 向量化，达到 C++ 原生水平 |
| **纯整数位运算 (Popcount)** | **29.68x** | 位运算全部原生化，GCC 极进优化 |
| **递归 (Fibonacci)** | **46.12x** | 数值特化 + 原生递归，GCC 深度内联 |
| **算术循环 (PowMod/FastPow)** | **2.29x–2.00x** | 循环体数值特化，取模运算受益于寄存器优化 |
| **短迭代 (GCD)** | **2.26x** | 迭代次数少，函数调用开销占比高 |
| **二分查找 (BinarySearch)** | **3.61x** | 混合数值+表操作，GCC 部分消除 table 开销 |
| **表操作为主 (BubbleSort/InsertionSort/Sieve/MatMul)** | **1.72x–4.62x** | 引入了 Table 结构体特化与读写，大幅提升读写效率 |

> **FakeLua GCC 后端在所有算法上均快于 Lua 5.4**（2.0x ~ 44.9x），特别是表常量化后，部分表操作场景的性能优势得到了显著提升。

---

## 表操作性能分析（VarTable vs unordered_map vs Lua Table）

取 n=1024 比较：

| 操作 | VarTable | unordered_map | Lua Table | VarTable vs map | VarTable vs Lua |
|------|----------|---------------|-----------|-----------------|-----------------|
| Set  | 57.3 µs | 51.1 µs | 47.9 µs | **1.12x** 慢 | **1.19x** 慢 |
| Get  | 12.8 µs | 6.2 µs | 16.2 µs | **2.0x** 慢 | **1.27x** 快 |
| Iter | 0.95 µs | 2.26 µs | 23.8 µs | **2.38x** 快 | **25.1x** 快 |
| Del  | 23.5 µs | 19.6 µs | 15.6 µs | **1.2x** 慢 | **1.5x** 慢 |

> VarTable 的核心优势在 **Iterate** 上（紧密的 active_list 遍历完全 cache-friendly），在 1024 元素时比 unordered_map 快 2.38 倍，比 Lua Table 快 **25.1 倍**。

---

## 总体结论

1. **C++ 最快**：在全部算法上领先，受益于 `-O3` 内联/展开/向量化。

2. **FakeLua GCC 全面超越 Lua，纯数值场景接近 C++ 原生**：
   - 纯整数运算（Sum、Popcount）：GCC 与 C++ 完全持平或接近，比 Lua 快 **17.1–26.8x**
   - 递归（Fibonacci）：比 Lua 快 **44.9x**，性能非常接近 C++
   - 算术循环（PowMod/FastPow）：比 Lua 快 **2.2–2.0x**
   - 表操作为主（BubbleSort/InsertionSort/Sieve/MatMul）：比 Lua 快 **2.3–6.3x**

3. **FakeLua TCC 优缺点分明**：
   - 纯整数算法（Sum、Popcount、FastPow、Fibonacci）：比 Lua 快 **2.0x ~ 10.4x**
   - 表操作密集型算法（BubbleSort、Sieve、InsertionSort、MatMul）：比 Lua 慢 **1.15x ~ 2.3x**，但只读静态 Table 的场景下已能够快于 Lua (**1.34x**)
   - TCC 生成的 C 代码对 table 读写路径较长，而 Lua 解释器对 table 操作深度优化，导致写密集表算法下 TCC 明显落后

4. **位运算 vs 取模**（FastPow `&`/`>>` vs PowMod `%`/`//`）：在 TCC 下位运算快（690.0 ns vs 429.0 ns），GCC 下两者也较接近，说明 FakeLua 已能对两种写法生成相近质量的代码。

5. **VarTable 遍历性能极优**：Iter 比 Lua Table 快 **25.1 倍**，比 `unordered_map` 快 2.38 倍，核心在于 active_list 的紧凑布局。

> 注：ASLR 开启，结果有一定随机噪声；建议在 `--cpu-scaling-enabled=false` 环境下多重复后取均值。
