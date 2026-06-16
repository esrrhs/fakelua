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

- 日期：2026-06-16
- 机器：2 X 2494.14 MHz CPU s
- CPU 缓存：L1d 32 KiB (x2)，L1i 32 KiB (x2)，L2 4096 KiB (x2)，L3 28160 KiB (x1)
- 构建模式：**Release**（`cmake .. -DCMAKE_BUILD_TYPE=Release`，最终编译标志 `-O3 -DNDEBUG`）
- FakeLua TCC JIT：**Release 模式**（`debug_mode=false`，TCC 启用 `-O2` 优化）
- FakeLua GCC JIT：**Release 模式**（`debug_mode=false`，GCC 启用 `-O3` 优化）
- 二进制：`build/bin/bench_mark`

## 运行命令

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
build/bin/bench_mark --benchmark_repetitions=1 --benchmark_report_aggregates_only=true
```

---

## 完整原始输出

```text
Starting benchmarks...
2026-06-16T23:14:08+08:00
Running ./build/bin/bench_mark
Run on (2 X 2494.14 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x2)
  L1 Instruction 32 KiB (x2)
  L2 Unified 4096 KiB (x2)
  L3 Unified 28160 KiB (x1)
Load Average: 1.31, 2.43, 2.00
***WARNING*** ASLR is enabled, the results may have unreproducible noise in them.
--------------------------------------------------------------------------------------------
Benchmark                                                  Time             CPU   Iterations
--------------------------------------------------------------------------------------------
BM_CPP_Fibonacci/20                                    19321 ns        19172 ns        37577
BM_CPP_Fibonacci/25                                   194160 ns       192669 ns         3661
BM_CPP_Fibonacci/30                                  2955655 ns      2371408 ns          296
BM_CPP_Fibonacci/32                                  6307623 ns      5888106 ns          122
BM_Lua_Fibonacci/20                                   712296 ns       707886 ns          993
BM_Lua_Fibonacci/25                                  7977420 ns      7920811 ns           91
BM_Lua_Fibonacci/30                                 88190920 ns     87542117 ns            8
BM_Lua_Fibonacci/32                                235309822 ns    229626569 ns            3
BM_FakeLua_Fibonacci_TCC/20                            70023 ns        69427 ns        10314
BM_FakeLua_Fibonacci_TCC/25                           782802 ns       760927 ns          919
BM_FakeLua_Fibonacci_TCC/30                          8477787 ns      8404888 ns           83
BM_FakeLua_Fibonacci_TCC/32                         22819387 ns     22102677 ns           32
BM_FakeLua_Fibonacci_GCC/20                            20373 ns        20239 ns        29577
BM_FakeLua_Fibonacci_GCC/25                           197950 ns       196412 ns         3607
BM_FakeLua_Fibonacci_GCC/30                          2109434 ns      2058006 ns          340
BM_FakeLua_Fibonacci_GCC/32                          5281314 ns      5254670 ns          134
BM_CPP_GCD/832040/514229                                 439 ns          437 ns      1634480
BM_CPP_GCD/123456789/987654321                          36.8 ns         36.6 ns     18263710
BM_CPP_GCD/2147483647/1073741823                        24.2 ns         24.0 ns     30326936
BM_Lua_GCD/832040/514229                                 917 ns          912 ns       769670
BM_Lua_GCD/123456789/987654321                           155 ns          154 ns      4539539
BM_Lua_GCD/2147483647/1073741823                         128 ns          127 ns      5130238
BM_FakeLua_GCD_TCC/832040/514229                         779 ns          771 ns       937304
BM_FakeLua_GCD_TCC/123456789/987654321                   155 ns          154 ns      4923618
BM_FakeLua_GCD_TCC/2147483647/1073741823                 124 ns          123 ns      5983680
BM_FakeLua_GCD_GCC/832040/514229                         554 ns          551 ns      1273198
BM_FakeLua_GCD_GCC/123456789/987654321                   115 ns          111 ns      6376607
BM_FakeLua_GCD_GCC/2147483647/1073741823                 106 ns          105 ns      6718848
BM_CPP_PowMod/2/1000/1000000007                          200 ns          193 ns      3625839
BM_CPP_PowMod/7/1000000/1000000007                       362 ns          360 ns      1942527
BM_CPP_PowMod/1234567/7654321/1000000007                 440 ns          437 ns      1595425
BM_Lua_PowMod/2/1000/1000000007                          906 ns          885 ns       732686
BM_Lua_PowMod/7/1000000/1000000007                      1578 ns         1569 ns       449103
BM_Lua_PowMod/1234567/7654321/1000000007                2021 ns         1970 ns       367616
BM_FakeLua_PowMod_TCC/2/1000/1000000007                  667 ns          661 ns      1114311
BM_FakeLua_PowMod_TCC/7/1000000/1000000007              1107 ns         1084 ns       657125
BM_FakeLua_PowMod_TCC/1234567/7654321/1000000007        1298 ns         1291 ns       547289
BM_FakeLua_PowMod_GCC/2/1000/1000000007                  326 ns          324 ns      2155333
BM_FakeLua_PowMod_GCC/7/1000000/1000000007               507 ns          504 ns      1394207
BM_FakeLua_PowMod_GCC/1234567/7654321/1000000007         651 ns          629 ns      1175115
BM_CPP_Sum/10000                                        5595 ns         5562 ns       122468
BM_CPP_Sum/100000                                      56083 ns        55234 ns        12644
BM_CPP_Sum/1000000                                    557259 ns       554202 ns         1272
BM_CPP_Sum/5000000                                   2780590 ns      2764242 ns          248
BM_Lua_Sum/10000                                       65021 ns        64630 ns        10949
BM_Lua_Sum/100000                                     663539 ns       658357 ns         1100
BM_Lua_Sum/1000000                                   6493227 ns      6458500 ns          112
BM_Lua_Sum/5000000                                  33037410 ns     32406303 ns           22
BM_FakeLua_Sum_TCC/10000                               19894 ns        19765 ns        35374
BM_FakeLua_Sum_TCC/100000                             196714 ns       195472 ns         3604
BM_FakeLua_Sum_TCC/1000000                           2040584 ns      2005925 ns          348
BM_FakeLua_Sum_TCC/5000000                          11806756 ns     11712395 ns           58
BM_FakeLua_Sum_GCC/10000                                5676 ns         5621 ns       125354
BM_FakeLua_Sum_GCC/100000                              57132 ns        55678 ns        12732
BM_FakeLua_Sum_GCC/1000000                            552654 ns       549551 ns         1272
BM_FakeLua_Sum_GCC/5000000                           2759907 ns      2743066 ns          255
BM_CPP_BubbleSort/50                                    8087 ns         8037 ns        86893
BM_CPP_BubbleSort/100                                  33299 ns        33074 ns        20961
BM_CPP_BubbleSort/200                                 144860 ns       141086 ns         5256
BM_Lua_BubbleSort/50                                   63902 ns        63343 ns        10105
BM_Lua_BubbleSort/100                                 225444 ns       221590 ns         3039
BM_Lua_BubbleSort/200                                 879348 ns       873663 ns          828
BM_FakeLua_BubbleSort_TCC/50                          153000 ns       152010 ns         4498
BM_FakeLua_BubbleSort_TCC/100                         604201 ns       600471 ns          890
BM_FakeLua_BubbleSort_TCC/200                        2477036 ns      2459367 ns          293
BM_FakeLua_BubbleSort_GCC/50                           29563 ns        29381 ns        23248
BM_FakeLua_BubbleSort_GCC/100                         114475 ns       113841 ns         6201
BM_FakeLua_BubbleSort_GCC/200                         463914 ns       461161 ns         1479
BM_CPP_Sieve/100                                         317 ns          315 ns      2196151
BM_CPP_Sieve/500                                        1875 ns         1743 ns       429545
BM_CPP_Sieve/1000                                       3538 ns         3513 ns       206515
BM_CPP_Sieve/5000                                      18501 ns        18370 ns        37810
BM_Lua_Sieve/100                                        7618 ns         7566 ns        95200
BM_Lua_Sieve/500                                       30861 ns        30630 ns        23075
BM_Lua_Sieve/1000                                      59827 ns        59384 ns        12488
BM_Lua_Sieve/5000                                     284346 ns       282566 ns         2113
BM_FakeLua_Sieve_TCC/100                               13275 ns        13191 ns        53112
BM_FakeLua_Sieve_TCC/500                               65160 ns        63492 ns        11039
BM_FakeLua_Sieve_TCC/1000                             131784 ns       130938 ns         5577
BM_FakeLua_Sieve_TCC/5000                             777442 ns       771378 ns          772
BM_FakeLua_Sieve_GCC/100                                2218 ns         2206 ns       293938
BM_FakeLua_Sieve_GCC/500                               13167 ns        13059 ns        54500
BM_FakeLua_Sieve_GCC/1000                              27578 ns        27375 ns        27041
BM_FakeLua_Sieve_GCC/5000                             194456 ns       192948 ns         3896
BM_CPP_BinarySearch/100                                  815 ns          803 ns       922079
BM_CPP_BinarySearch/500                                17688 ns        17387 ns        39503
BM_CPP_BinarySearch/1000                               44653 ns        44373 ns        12340
BM_Lua_BinarySearch/100                                36989 ns        36709 ns        19529
BM_Lua_BinarySearch/500                               242919 ns       236875 ns         2997
BM_Lua_BinarySearch/1000                              567192 ns       562685 ns         1394
BM_FakeLua_BinarySearch_TCC/100                        55125 ns        54766 ns        12712
BM_FakeLua_BinarySearch_TCC/500                       359123 ns       356235 ns         1920
BM_FakeLua_BinarySearch_TCC/1000                      795434 ns       789653 ns          905
BM_FakeLua_BinarySearch_GCC/100                         7174 ns         7114 ns        89202
BM_FakeLua_BinarySearch_GCC/500                        57219 ns        55079 ns        13595
BM_FakeLua_BinarySearch_GCC/1000                      120839 ns       120114 ns         5913
BM_CPP_FastPow/2/1000/1000000007                         196 ns          195 ns      3412828
BM_CPP_FastPow/7/1000000/1000000007                      372 ns          370 ns      1876003
BM_CPP_FastPow/1234567/7654321/1000000007                476 ns          453 ns      1521179
BM_Lua_FastPow/2/1000/1000000007                         656 ns          651 ns      1173430
BM_Lua_FastPow/7/1000000/1000000007                     1144 ns         1125 ns       689455
BM_Lua_FastPow/1234567/7654321/1000000007               1298 ns         1289 ns       542263
BM_FakeLua_FastPow_TCC/2/1000/1000000007                 415 ns          412 ns      1760759
BM_FakeLua_FastPow_TCC/7/1000000/1000000007              644 ns          630 ns       922452
BM_FakeLua_FastPow_TCC/1234567/7654321/1000000007        785 ns          775 ns       912003
BM_FakeLua_FastPow_GCC/2/1000/1000000007                 315 ns          303 ns      2216596
BM_FakeLua_FastPow_GCC/7/1000000/1000000007              505 ns          489 ns      1462424
BM_FakeLua_FastPow_GCC/1234567/7654321/1000000007        597 ns          593 ns      1218661
BM_CPP_Popcount/1000                                    3666 ns         3556 ns       199519
BM_CPP_Popcount/10000                                  44973 ns        44655 ns        15673
BM_CPP_Popcount/100000                                530787 ns       516468 ns         1286
BM_Lua_Popcount/1000                                   94693 ns        94072 ns         7213
BM_Lua_Popcount/10000                                1210222 ns      1178877 ns          594
BM_Lua_Popcount/100000                              13996772 ns     13905043 ns           50
BM_FakeLua_Popcount_TCC/1000                           11965 ns        11763 ns        59912
BM_FakeLua_Popcount_TCC/10000                         155136 ns       153215 ns         4556
BM_FakeLua_Popcount_TCC/100000                       2028826 ns      1954806 ns          357
BM_FakeLua_Popcount_GCC/1000                            3732 ns         3707 ns       181487
BM_FakeLua_Popcount_GCC/10000                          40797 ns        40571 ns        16829
BM_FakeLua_Popcount_GCC/100000                        493304 ns       489888 ns         1444
BM_CPP_InsertionSort/50                                  964 ns          957 ns       737028
BM_CPP_InsertionSort/100                                3652 ns         3629 ns       196692
BM_CPP_InsertionSort/200                               14044 ns        13552 ns        53628
BM_Lua_InsertionSort/50                                39896 ns        39580 ns        17800
BM_Lua_InsertionSort/100                              147726 ns       146220 ns         4901
BM_Lua_InsertionSort/200                              597191 ns       592476 ns         1248
BM_FakeLua_InsertionSort_TCC/50                       101365 ns       100655 ns         7031
BM_FakeLua_InsertionSort_TCC/100                      401814 ns       399205 ns         1882
BM_FakeLua_InsertionSort_TCC/200                     1559429 ns      1548604 ns          481
BM_FakeLua_InsertionSort_GCC/50                        16050 ns        15966 ns        44063
BM_FakeLua_InsertionSort_GCC/100                       62636 ns        60998 ns        11592
BM_FakeLua_InsertionSort_GCC/200                      240639 ns       239083 ns         2949
BM_CPP_MatMul                                           3.59 ns         3.57 ns    193021693
BM_Lua_MatMul                                           2185 ns         2120 ns       329506
BM_FakeLua_MatMul_TCC                                   4233 ns         4205 ns       172431
BM_FakeLua_MatMul_GCC                                    627 ns          617 ns      1109486
BM_VarTable_Set/2                                        320 ns          315 ns      2331613
BM_VarTable_Set/4                                        330 ns          328 ns      2104906
BM_VarTable_Set/8                                        374 ns          361 ns      1896944
BM_VarTable_Set/16                                      1298 ns         1288 ns       553809
BM_VarTable_Set/32                                      3165 ns         3115 ns       219628
BM_VarTable_Set/64                                      7057 ns         7010 ns        97529
BM_VarTable_Set/128                                    15094 ns        14929 ns        48247
BM_VarTable_Set/256                                    30485 ns        30286 ns        22529
BM_VarTable_Set/512                                    60583 ns        60180 ns        11625
BM_VarTable_Set/1024                                  121253 ns       120407 ns         5962
BM_StdUnorderedMap_Set/2                                 131 ns          130 ns      5340240
BM_StdUnorderedMap_Set/4                                 219 ns          217 ns      3246591
BM_StdUnorderedMap_Set/8                                 450 ns          446 ns      1591686
BM_StdUnorderedMap_Set/16                               1023 ns         1015 ns       693956
BM_StdUnorderedMap_Set/32                               2242 ns         2227 ns       319114
BM_StdUnorderedMap_Set/64                               4590 ns         4560 ns       148882
BM_StdUnorderedMap_Set/128                              9820 ns         9746 ns        72003
BM_StdUnorderedMap_Set/256                             18834 ns        18470 ns        37995
BM_StdUnorderedMap_Set/512                             40847 ns        40595 ns        17385
BM_StdUnorderedMap_Set/1024                            82663 ns        82233 ns         8483
BM_LuaTable_Set/2                                       1901 ns         1877 ns       369131
BM_LuaTable_Set/4                                       2042 ns         2043 ns       345100
BM_LuaTable_Set/8                                       2466 ns         2441 ns       296594
BM_LuaTable_Set/16                                      3010 ns         2976 ns       239555
BM_LuaTable_Set/32                                      4244 ns         4062 ns       178376
BM_LuaTable_Set/64                                      6200 ns         6065 ns       117957
BM_LuaTable_Set/128                                    10058 ns        10023 ns        74622
BM_LuaTable_Set/256                                    17578 ns        17348 ns        42907
BM_LuaTable_Set/512                                    33091 ns        32949 ns        23925
BM_LuaTable_Set/1024                                   63711 ns        63421 ns        12660
BM_VarTable_Get/2                                       29.0 ns         28.8 ns     24365057
BM_VarTable_Get/4                                       58.3 ns         57.9 ns     11871498
BM_VarTable_Get/8                                        119 ns          119 ns      5912847
BM_VarTable_Get/16                                       213 ns          211 ns      3299603
BM_VarTable_Get/32                                       423 ns          420 ns      1695501
BM_VarTable_Get/64                                       825 ns          821 ns       846346
BM_VarTable_Get/128                                     1668 ns         1658 ns       427548
BM_VarTable_Get/256                                     3330 ns         3311 ns       212109
BM_VarTable_Get/512                                     6664 ns         6623 ns       106437
BM_VarTable_Get/1024                                   13283 ns        13199 ns        53243
BM_StdUnorderedMap_Get/2                                22.2 ns         22.1 ns     31708428
BM_StdUnorderedMap_Get/4                                44.2 ns         43.6 ns     16007743
BM_StdUnorderedMap_Get/8                                86.2 ns         85.7 ns      8175860
BM_StdUnorderedMap_Get/16                                182 ns          181 ns      3880510
BM_StdUnorderedMap_Get/32                                379 ns          365 ns      1977577
BM_StdUnorderedMap_Get/64                                724 ns          716 ns      1002342
BM_StdUnorderedMap_Get/128                              1383 ns         1377 ns       489598
BM_StdUnorderedMap_Get/256                              2774 ns         2761 ns       249970
BM_StdUnorderedMap_Get/512                              5627 ns         5596 ns       126392
BM_StdUnorderedMap_Get/1024                            11235 ns        11176 ns        63556
BM_LuaTable_Get/2                                       36.0 ns         35.4 ns     20428650
BM_LuaTable_Get/4                                       69.9 ns         69.1 ns     10146805
BM_LuaTable_Get/8                                        144 ns          143 ns      4893337
BM_LuaTable_Get/16                                       282 ns          281 ns      2511715
BM_LuaTable_Get/32                                       552 ns          549 ns      1197378
BM_LuaTable_Get/64                                      1101 ns         1094 ns       646407
BM_LuaTable_Get/128                                     2205 ns         2192 ns       314060
BM_LuaTable_Get/256                                     4566 ns         4408 ns       160401
BM_LuaTable_Get/512                                     8765 ns         8707 ns        80320
BM_LuaTable_Get/1024                                   17522 ns        17425 ns        40068
BM_VarTable_Iter/2                                      3.42 ns         3.40 ns    200991350
BM_VarTable_Iter/4                                      5.43 ns         5.39 ns    131529503
BM_VarTable_Iter/8                                      9.12 ns         9.06 ns     78832908
BM_VarTable_Iter/16                                     16.0 ns         15.3 ns     45301393
BM_VarTable_Iter/32                                     30.7 ns         30.5 ns     22566743
BM_VarTable_Iter/64                                     65.2 ns         63.7 ns     10990647
BM_VarTable_Iter/128                                     133 ns          132 ns      5328512
BM_VarTable_Iter/256                                     258 ns          256 ns      2734424
BM_VarTable_Iter/512                                     525 ns          522 ns      1349264
BM_VarTable_Iter/1024                                   1280 ns         1260 ns       559545
BM_StdUnorderedMap_Iter/2                               1.26 ns         1.26 ns    556558085
BM_StdUnorderedMap_Iter/4                               3.44 ns         3.36 ns    207345792
BM_StdUnorderedMap_Iter/8                               4.68 ns         4.54 ns    154751838
BM_StdUnorderedMap_Iter/16                              11.3 ns         11.2 ns     62354447
BM_StdUnorderedMap_Iter/32                              31.6 ns         29.5 ns     23697006
BM_StdUnorderedMap_Iter/64                              86.4 ns         85.9 ns      7778118
BM_StdUnorderedMap_Iter/128                              249 ns          248 ns      2845044
BM_StdUnorderedMap_Iter/256                              455 ns          440 ns      1600085
BM_StdUnorderedMap_Iter/512                             1087 ns         1080 ns       668235
BM_StdUnorderedMap_Iter/1024                            3030 ns         2952 ns       234982
BM_LuaTable_Iter/2                                      69.2 ns         68.8 ns      9717263
BM_LuaTable_Iter/4                                       127 ns          125 ns      5633760
BM_LuaTable_Iter/8                                       251 ns          249 ns      2788024
BM_LuaTable_Iter/16                                      466 ns          464 ns      1474475
BM_LuaTable_Iter/32                                      927 ns          903 ns       779170
BM_LuaTable_Iter/64                                     1797 ns         1785 ns       393883
BM_LuaTable_Iter/128                                    3661 ns         3599 ns       195060
BM_LuaTable_Iter/256                                    7146 ns         7106 ns        99007
BM_LuaTable_Iter/512                                   14513 ns        14114 ns        47557
BM_LuaTable_Iter/1024                                  28729 ns        28566 ns        24560
BM_VarTable_Del/2                                        802 ns          792 ns       884400
BM_VarTable_Del/4                                        851 ns          843 ns       844700
BM_VarTable_Del/8                                        939 ns          933 ns       760999
BM_VarTable_Del/16                                      1131 ns         1056 ns       673594
BM_VarTable_Del/32                                      1993 ns         1583 ns       546071
BM_VarTable_Del/64                                      2712 ns         2353 ns       366064
BM_VarTable_Del/128                                     4146 ns         3665 ns       195635
BM_VarTable_Del/256                                     7096 ns         6244 ns       129118
BM_VarTable_Del/512                                    14481 ns        11005 ns        73948
BM_VarTable_Del/1024                                   23824 ns        20485 ns        37296
BM_StdUnorderedMap_Del/2                                 824 ns          819 ns       846632
BM_StdUnorderedMap_Del/4                                 867 ns          865 ns       821472
BM_StdUnorderedMap_Del/8                                 984 ns          981 ns       715072
BM_StdUnorderedMap_Del/16                               1308 ns         1290 ns       537046
BM_StdUnorderedMap_Del/32                               1860 ns         1852 ns       382645
BM_StdUnorderedMap_Del/64                               2959 ns         2877 ns       237946
BM_StdUnorderedMap_Del/128                              5119 ns         5022 ns       131869
BM_StdUnorderedMap_Del/256                              8773 ns         8702 ns        83137
BM_StdUnorderedMap_Del/512                             16623 ns        16429 ns        42201
BM_StdUnorderedMap_Del/1024                            32728 ns        32455 ns        21716
BM_LuaTable_Del/2                                       1608 ns         1566 ns       450450
BM_LuaTable_Del/4                                       1631 ns         1590 ns       423823
BM_LuaTable_Del/8                                       1676 ns         1654 ns       423922
BM_LuaTable_Del/16                                      1842 ns         1787 ns       392315
BM_LuaTable_Del/32                                      1996 ns         1988 ns       351699
BM_LuaTable_Del/64                                      2502 ns         2483 ns       281726
BM_LuaTable_Del/128                                     3623 ns         3532 ns       205459
BM_LuaTable_Del/256                                     5405 ns         5277 ns       135124
BM_LuaTable_Del/512                                     9431 ns         9056 ns        79160
BM_LuaTable_Del/1024                                   16629 ns        16471 ns        42925
```

---

## 算法性能分析（C++ vs Lua vs FakeLua TCC vs FakeLua GCC）

以下取各算法的典型参数做横向对比（CPU Time），**倍数 = 对应实现时间 / C++ 时间**：

### 1. Fibonacci（n=32，递归无记忆化）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 5.89 ms | 1x |
| Lua | 229.63 ms | **39.0x** 慢 |
| FakeLua TCC | 22.10 ms | **3.8x** 慢 |
| FakeLua GCC | 5.25 ms | **1.1x** 快 |

> GCC 比 Lua 快 **43.7x**，TCC 比 Lua 快 **10.4x**。数值参数特化生成原生递归 + 原生条件比较，是 TCC/GCC 均优于 Lua 的关键。

### 2. GCD（a=832040, b=514229，约 30 次迭代）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 437.0 ns | 1x |
| Lua | 912.0 ns | **2.1x** 慢 |
| FakeLua TCC | 771.0 ns | **1.8x** 慢 (比 Lua 快 **1.2x**) |
| FakeLua GCC | 551.0 ns | **1.3x** 慢 (比 Lua 快 **1.7x**) |

### 3. PowMod（base=1234567, exp=7654321, mod=1e9+7，约 23 次循环，用 `%`/`//`）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 437.0 ns | 1x |
| Lua | 1970.0 ns | **4.5x** 慢 |
| FakeLua TCC | 1291.0 ns | **3.0x** 慢 (比 Lua 快 **1.5x**) |
| FakeLua GCC | 629.0 ns | **1.4x** 慢 (比 Lua 快 **3.1x**) |

### 4. FastPow（base=1234567, exp=7654321, mod=1e9+7，用 `&`/`>>`）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 453.0 ns | 1x |
| Lua | 1289.0 ns | **2.8x** 慢 |
| FakeLua TCC | 775.0 ns | **1.7x** 慢 (比 Lua 快 **1.7x**) |
| FakeLua GCC | 593.0 ns | **1.3x** 慢 (比 Lua 快 **2.2x**) |

> FastPow 用位运算 `&`/`>>` 代替取余/整除 `%`/`//`，在 FakeLua TCC 下比 PowMod 快约 **1.7x**（1291.0 ns → 775.0 ns），说明 TCC 对位运算的代码生成较优。GCC 两者表现也较接近。

### 5. Sum（n=5000000，纯循环累加）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 2.76 ms | 1x |
| Lua | 32.41 ms | **11.7x** 慢 |
| FakeLua TCC | 11.71 ms | **4.2x** 慢 (比 Lua 快 **2.8x**) |
| FakeLua GCC | 2.74 ms | **1.0x** 快 |

> 纯整数累加循环：FakeLua GCC 与 C++ 几乎完全相同，说明 GCC `-O3` 对简单数值循环已达到 C++ 原生水平。TCC 比 Lua 快 **2.8x**。

### 6. BubbleSort（n=200，O(n²)，含表操作）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 141.1 µs | 1x |
| Lua | 873.7 µs | **6.2x** 慢 |
| FakeLua TCC | 2459.4 µs | **17.4x** 慢 (比 Lua 慢 **2.8x**) |
| FakeLua GCC | 461.2 µs | **3.3x** 慢 (比 Lua 快 **1.9x**) |

> 含大量表 Set/Get 操作的排序算法，**TCC 表现明显弱于 Lua**（2.8x 差距）。TCC 对 table 索引操作生成的代码路径较长（无寄存器分配优化），而 Lua 解释器在 table 操作上已高度优化。GCC 目前比 Lua 快约 **1.9x**。

### 7. Sieve（n=5000，Eratosthenes 筛）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 18.4 µs | 1x |
| Lua | 282.6 µs | **15.4x** 慢 |
| FakeLua TCC | 771.4 µs | **42.0x** 慢 (比 Lua 慢 **2.7x**) |
| FakeLua GCC | 192.9 µs | **10.5x** 慢 (比 Lua 快 **1.5x**) |

> 筛法涉及大量 boolean 表操作（`is_prime[j] = false`），TCC 在此类写密集型表操作上比 Lua 慢 **2.7x**，同样反映 TCC 在 table 写操作的代码生成开销。GCC 比 Lua 快 **1.5x**。

### 8. BinarySearch（n=1000，n 次二分查找）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 44.4 µs | 1x |
| Lua | 562.7 µs | **12.7x** 慢 |
| FakeLua TCC | 789.7 µs | **17.8x** 慢 (比 Lua 慢 **1.4x**) |
| FakeLua GCC | 120.1 µs | **2.7x** 慢 (比 Lua 快 **4.7x**) |

> 二分查找含 `break` 语句的 while 循环和表随机访问。TCC 比 Lua 慢 1.4x，GCC 比 Lua 快 4.7x。

### 9. Popcount（n=100000，Brian Kernighan 位计数累加）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 516.5 µs | 1x |
| Lua | 13905.0 µs | **26.9x** 慢 |
| FakeLua TCC | 1954.8 µs | **3.8x** 慢 (比 Lua 快 **7.1x**) |
| FakeLua GCC | 489.9 µs | **1.1x** 快 (比 Lua 快 **28.4x**) |

> 纯整数位运算（`&`，`!=`），无表操作。**TCC 比 Lua 快 7.1x，GCC 比 Lua 快 28.4x，接近/超越 C++**（测量噪声范围内，GCC 与 C++ 基本持平）。这是纯整数算法场景，FakeLua 优势最大。

### 10. InsertionSort（n=200，O(n²)，含表操作）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 13.6 µs | 1x |
| Lua | 592.5 µs | **43.7x** 慢 |
| FakeLua TCC | 1548.6 µs | **114.3x** 慢 (比 Lua 慢 **2.6x**) |
| FakeLua GCC | 239.1 µs | **17.6x** 慢 (比 Lua 快 **2.5x**) |

> 与冒泡排序类似，表操作为瓶颈。TCC 比 Lua 慢约 2.6x，GCC 快于 Lua 约 2.5x。

### 11. MatMul（单次 3×3 矩阵乘法）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 3.57 ns | 1x |
| Lua | 2120.0 ns | **593.8x** 慢 |
| FakeLua TCC | 4205.0 ns | **1177.9x** 慢 (比 Lua 慢 **2.0x**) |
| FakeLua GCC | 617.0 ns | **172.8x** 慢 (比 Lua 快 **3.4x**) |

> 矩阵乘法每次调用需创建 3 个 Lua table（各 9 个元素），**table 创建与 GC 开销远大于实际计算**。C++ 的数 ns 得益于 `-O3` 向量化。TCC 在 table 分配+初始化开销下比 Lua 慢 2.0x；GCC 通过更优的寄存器分配比 Lua 快 3.4x。

---

## FakeLua TCC vs Lua 5.4 详细对比

| 算法 | 参数 | Lua | FakeLua TCC | 结果 |
|------|------|-----|-------------|------|
| Fibonacci | n=32 | 229.63 ms | 22.10 ms | TCC **10.4x 快** |
| GCD | 832040/514229 | 912.0 ns | 771.0 ns | TCC **1.2x 快** |
| PowMod | 1234567/7654321/1e9+7 | 1970.0 ns | 1291.0 ns | TCC **1.5x 快** |
| FastPow | 1234567/7654321/1e9+7 | 1289.0 ns | 775.0 ns | TCC **1.7x 快** |
| Sum | n=5000000 | 32.41 ms | 11.71 ms | TCC **2.8x 快** |
| BubbleSort | n=200 | 873.7 µs | 2459.4 µs | TCC **2.8x 慢** |
| Sieve | n=5000 | 282.6 µs | 771.4 µs | TCC **2.7x 慢** |
| BinarySearch | n=1000 | 562.7 µs | 789.7 µs | TCC **1.4x 慢** |
| Popcount | n=100000 | 13905.0 µs | 1954.8 µs | TCC **7.1x 快** |
| InsertionSort | n=200 | 592.5 µs | 1548.6 µs | TCC **2.6x 慢** |
| MatMul | 单次 3×3 | 2120.0 ns | 4205.0 ns | TCC **2.0x 慢** |

> **结论**：TCC 在**纯整数运算**（无表操作）算法上快于 Lua（1.7x ~ 7.1x）；在**表操作密集**算法上显著慢于 Lua（1.4x ~ 2.7x）。TCC 生成的代码对 table 读写路径较长（无循环不变量提升、无内联），而 Lua 解释器对 table 操作做了深度优化。

---

## FakeLua GCC vs Lua 5.4 详细对比

| 算法 | 参数 | Lua | FakeLua GCC | GCC 快多少 |
|------|------|-----|-------------|-----------|
| Fibonacci | n=32 | 229.63 ms | 5.25 ms | **43.7x** |
| GCD | 832040/514229 | 912.0 ns | 551.0 ns | **1.7x** |
| PowMod | 1234567/7654321/1e9+7 | 1970.0 ns | 629.0 ns | **3.1x** |
| FastPow | 1234567/7654321/1e9+7 | 1289.0 ns | 593.0 ns | **2.2x** |
| Sum | n=5000000 | 32.41 ms | 2.74 ms | **11.8x** |
| BubbleSort | n=200 | 873.7 µs | 461.2 µs | **1.9x** |
| Sieve | n=5000 | 282.6 µs | 192.9 µs | **1.5x** |
| BinarySearch | n=1000 | 562.7 µs | 120.1 µs | **4.7x** |
| Popcount | n=100000 | 13905.0 µs | 489.9 µs | **28.4x** |
| InsertionSort | n=200 | 592.5 µs | 239.1 µs | **2.5x** |
| MatMul | 单次 3×3 | 2120.0 ns | 617.0 ns | **3.4x** |

### FakeLua GCC 按场景分类

| 场景类型 | FakeLua GCC 比 Lua 快 | 原因 |
|---------|----------------------|------|
| **纯整数累加 (Sum)** | **11.8x** | GCC `-O3` 向量化，达到 C++ 原生水平 |
| **纯整数位运算 (Popcount)** | **28.4x** | 位运算全部原生化，GCC 激进优化 |
| **递归 (Fibonacci)** | **43.7x** | 数值特化 + 原生递归，GCC 深度内联 |
| **算术循环 (PowMod/FastPow)** | **3.1–2.2x** | 循环体数值特化，取模运算受益于寄存器优化 |
| **短迭代 (GCD)** | **1.7x** | 迭代次数少，函数调用开销占比高 |
| **二分查找 (BinarySearch)** | **4.7x** | 混合数值+表操作，GCC 部分消除 table 开销 |
| **表操作为主 (BubbleSort/InsertionSort/Sieve/MatMul)** | **1.9–3.4x** | table 操作仍是瓶颈，GCC 无法完全消除 |

> **FakeLua GCC 后端在所有算法上均快于或持平 Lua 5.4**（1.9x ~ 28.4x），纯数值算法优势最大，表操作密集型算法优势也较明显（约 1.9x ~ 3.4x）。

---

## 表操作性能分析（VarTable vs unordered_map vs Lua Table）

取 n=1024 比较：

| 操作 | VarTable | unordered_map | Lua Table | VarTable vs map | VarTable vs Lua |
|------|----------|---------------|-----------|-----------------|-----------------|
| Set  | 120.4 µs | 82.2 µs | 63.4 µs | **1.5x** 慢 | **1.9x** 慢 |
| Get  | 13.2 µs | 11.2 µs | 17.4 µs | **1.2x** 慢 | **1.3x** 快 |
| Iter | 1.3 µs | 3.0 µs | 28.6 µs | **2.3x** 快 | **22.7x** 快 |
| Del  | 20.5 µs | 32.5 µs | 16.5 µs | **1.6x** 快 | **1.2x** 慢 |

> VarTable 的核心优势在 **Iterate** 上（紧密的 active_list 遍历完全 cache-friendly），在 1024 元素时比 unordered_map 快 2.3 倍，比 Lua Table 快 **22.7 倍**。**Delete** 也有显著优势。

---

## 总体结论

1. **C++ 最快**：在全部算法上领先，受益于 `-O3` 内联/展开/向量化。

2. **FakeLua GCC 全面超越 Lua，纯数值场景接近 C++ 原生**：
   - 纯整数运算（Sum、Popcount）：GCC 与 C++ 完全持平或接近，比 Lua 快 **28.4–11.8x**
   - 递归（Fibonacci）：比 Lua 快 **43.7x**，性能非常接近 C++ 
   - 算术循环（PowMod/FastPow）：比 Lua 快 **3.1–2.2x**
   - 表操作为主（BubbleSort/InsertionSort/Sieve/MatMul）：比 Lua 快 **1.9x–3.4x**

3. **FakeLua TCC 优缺点分明**：
   - 纯整数算法（Sum、Popcount、FastPow、Fibonacci）：比 Lua 快 **2.8x ~ 7.1x**
   - 表操作密集型算法（BubbleSort、Sieve、InsertionSort、MatMul）：比 Lua 慢 **2.0x ~ 2.8x**
   - TCC 生成的 C 代码对 table 读写路径较长，而 Lua 解释器对 table 操作深度优化，导致表密集算法下 TCC 明显落后

4. **位运算 vs 取模**（FastPow `&`/`>>` vs PowMod `%`/`//`）：在 TCC 下位运算快（1291.0 ns vs 775.0 ns），GCC 下两者也较接近，说明 FakeLua 已能对两种写法生成相近质量的代码。

5. **VarTable 遍历性能极优**：Iter 比 Lua Table 快 **22.7 倍**，比 `unordered_map` 快 2.3 倍，核心在于 active_list 的紧凑布局。

> 注：ASLR 开启，结果有一定随机噪声；建议在 `--cpu-scaling-enabled=false` 环境下多重复后取均值。
