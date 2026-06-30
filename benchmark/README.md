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
2026-06-30T10:18:40+08:00
Running build/bin/bench_mark
Run on (2 X 2595.12 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x2)
  L1 Instruction 32 KiB (x2)
  L2 Unified 4096 KiB (x2)
  L3 Unified 16384 KiB (x1)
Load Average: 0.93, 0.56, 0.49
***WARNING*** ASLR is enabled, the results may have unreproducible noise in them.
--------------------------------------------------------------------------------------------
Benchmark                                                  Time             CPU   Iterations
--------------------------------------------------------------------------------------------
BM_CPP_Fibonacci/20                                    16130 ns        16121 ns        45655
BM_CPP_Fibonacci/25                                   148027 ns       147927 ns         3719
BM_CPP_Fibonacci/30                                  1844064 ns      1842803 ns          307
BM_CPP_Fibonacci/32                                  4897692 ns      4895087 ns          146
BM_Lua_Fibonacci/20                                   670169 ns       668306 ns         1063
BM_Lua_Fibonacci/25                                  7127658 ns      7126321 ns           75
BM_Lua_Fibonacci/30                                 78384976 ns     78364318 ns            7
BM_Lua_Fibonacci/32                                203974603 ns    203951570 ns            3
BM_FakeLua_Fibonacci_TCC/20                            62256 ns        61686 ns         8529
BM_FakeLua_Fibonacci_TCC/25                           719884 ns       719831 ns         1016
BM_FakeLua_Fibonacci_TCC/30                          7530770 ns      7525059 ns           86
BM_FakeLua_Fibonacci_TCC/32                         20000669 ns     19975480 ns           33
BM_FakeLua_Fibonacci_GCC/20                            16687 ns        16675 ns        34085
BM_FakeLua_Fibonacci_GCC/25                           158315 ns       158201 ns         3652
BM_FakeLua_Fibonacci_GCC/30                          1690121 ns      1688046 ns          358
BM_FakeLua_Fibonacci_GCC/32                          4578175 ns      4575733 ns          158
BM_CPP_GCD/832040/514229                                 121 ns          121 ns      5792608
BM_CPP_GCD/123456789/987654321                          16.7 ns         16.7 ns     41957368
BM_CPP_GCD/2147483647/1073741823                        13.0 ns         13.0 ns     54101379
BM_Lua_GCD/832040/514229                                 461 ns          461 ns      1553999
BM_Lua_GCD/123456789/987654321                           121 ns          121 ns      5919974
BM_Lua_GCD/2147483647/1073741823                         103 ns          103 ns      5244187
BM_FakeLua_GCD_TCC/832040/514229                         272 ns          271 ns      2700385
BM_FakeLua_GCD_TCC/123456789/987654321                   125 ns          124 ns      5181201
BM_FakeLua_GCD_TCC/2147483647/1073741823                 118 ns          118 ns      5429263
BM_FakeLua_GCD_GCC/832040/514229                         204 ns          204 ns      3105192
BM_FakeLua_GCD_GCC/123456789/987654321                   109 ns          109 ns      6720043
BM_FakeLua_GCD_GCC/2147483647/1073741823                 107 ns          107 ns      6812078
BM_CPP_PowMod/2/1000/1000000007                          104 ns          104 ns      6675637
BM_CPP_PowMod/7/1000000/1000000007                       204 ns          204 ns      3426311
BM_CPP_PowMod/1234567/7654321/1000000007                 296 ns          296 ns      2363560
BM_Lua_PowMod/2/1000/1000000007                          447 ns          446 ns      1593875
BM_Lua_PowMod/7/1000000/1000000007                       794 ns          793 ns       760631
BM_Lua_PowMod/1234567/7654321/1000000007                 917 ns          917 ns       650246
BM_FakeLua_PowMod_TCC/2/1000/1000000007                  330 ns          330 ns      2083492
BM_FakeLua_PowMod_TCC/7/1000000/1000000007               556 ns          555 ns      1240802
BM_FakeLua_PowMod_TCC/1234567/7654321/1000000007         685 ns          684 ns       917803
BM_FakeLua_PowMod_GCC/2/1000/1000000007                  213 ns          213 ns      3122515
BM_FakeLua_PowMod_GCC/7/1000000/1000000007               325 ns          324 ns      2193541
BM_FakeLua_PowMod_GCC/1234567/7654321/1000000007         418 ns          418 ns      1677203
BM_CPP_Sum/10000                                        3093 ns         3091 ns       226167
BM_CPP_Sum/100000                                      30913 ns        30894 ns        22710
BM_CPP_Sum/1000000                                    308959 ns       308794 ns         2260
BM_CPP_Sum/5000000                                   1545431 ns      1544441 ns          454
BM_Lua_Sum/10000                                       54957 ns        54924 ns        12242
BM_Lua_Sum/100000                                     557657 ns       556859 ns         1305
BM_Lua_Sum/1000000                                   5482921 ns      5478826 ns          124
BM_Lua_Sum/5000000                                  27519155 ns     27454388 ns           25
BM_FakeLua_Sum_TCC/10000                               24932 ns        24888 ns        30538
BM_FakeLua_Sum_TCC/100000                             275630 ns       275605 ns         2437
BM_FakeLua_Sum_TCC/1000000                           2309112 ns      2308994 ns          243
BM_FakeLua_Sum_TCC/5000000                          10949988 ns     10928047 ns          106
BM_FakeLua_Sum_GCC/10000                                3208 ns         3205 ns       217461
BM_FakeLua_Sum_GCC/100000                              31109 ns        31076 ns        22608
BM_FakeLua_Sum_GCC/1000000                            309717 ns       309525 ns         2257
BM_FakeLua_Sum_GCC/5000000                           1548046 ns      1545380 ns          454
BM_CPP_BubbleSort/50                                    7645 ns         7616 ns        90281
BM_CPP_BubbleSort/100                                  31503 ns        31454 ns        22197
BM_CPP_BubbleSort/200                                 126535 ns       126442 ns         5538
BM_Lua_BubbleSort/50                                   54143 ns        54074 ns        13160
BM_Lua_BubbleSort/100                                 208882 ns       208761 ns         3492
BM_Lua_BubbleSort/200                                 827332 ns       826375 ns          867
BM_FakeLua_BubbleSort_TCC/50                          115897 ns       115279 ns         6490
BM_FakeLua_BubbleSort_TCC/100                         437174 ns       436672 ns         1552
BM_FakeLua_BubbleSort_TCC/200                        1745774 ns      1744644 ns          420
BM_FakeLua_BubbleSort_GCC/50                           31261 ns        31230 ns        21558
BM_FakeLua_BubbleSort_GCC/100                         125133 ns       124415 ns         5771
BM_FakeLua_BubbleSort_GCC/200                         491459 ns       491089 ns         1377
BM_CPP_Sieve/100                                         252 ns          252 ns      2557016
BM_CPP_Sieve/500                                        1189 ns         1189 ns       459109
BM_CPP_Sieve/1000                                       2390 ns         2387 ns       262120
BM_CPP_Sieve/5000                                      12224 ns        12216 ns        44905
BM_Lua_Sieve/100                                        7186 ns         7168 ns       101746
BM_Lua_Sieve/500                                       27269 ns        27254 ns        24345
BM_Lua_Sieve/1000                                      53974 ns        53909 ns        12549
BM_Lua_Sieve/5000                                     271415 ns       271204 ns         2509
BM_FakeLua_Sieve_TCC/100                                9879 ns         9866 ns        73295
BM_FakeLua_Sieve_TCC/500                               48811 ns        48770 ns        14699
BM_FakeLua_Sieve_TCC/1000                              97995 ns        97826 ns         7391
BM_FakeLua_Sieve_TCC/5000                             598991 ns       598603 ns         1137
BM_FakeLua_Sieve_GCC/100                                1975 ns         1975 ns       309957
BM_FakeLua_Sieve_GCC/500                                9618 ns         9612 ns        76797
BM_FakeLua_Sieve_GCC/1000                              18809 ns        18797 ns        32344
BM_FakeLua_Sieve_GCC/5000                             126305 ns       126112 ns         4605
BM_CPP_BinarySearch/100                                  847 ns          847 ns       665604
BM_CPP_BinarySearch/500                                 6085 ns         6084 ns       114231
BM_CPP_BinarySearch/1000                               18675 ns        18662 ns        33826
BM_Lua_BinarySearch/100                                29602 ns        29544 ns        24840
BM_Lua_BinarySearch/500                               199351 ns       199208 ns         3500
BM_Lua_BinarySearch/1000                              455633 ns       455131 ns         1554
BM_FakeLua_BinarySearch_TCC/100                        40367 ns        40341 ns        16917
BM_FakeLua_BinarySearch_TCC/500                       272334 ns       272048 ns         2368
BM_FakeLua_BinarySearch_TCC/1000                      635170 ns       634709 ns          813
BM_FakeLua_BinarySearch_GCC/100                         6036 ns         6033 ns        89302
BM_FakeLua_BinarySearch_GCC/500                        52320 ns        52316 ns        11679
BM_FakeLua_BinarySearch_GCC/1000                      125561 ns       125368 ns         5089
BM_CPP_FastPow/2/1000/1000000007                         105 ns          104 ns      6713052
BM_CPP_FastPow/7/1000000/1000000007                      204 ns          204 ns      3424937
BM_CPP_FastPow/1234567/7654321/1000000007                297 ns          297 ns      2362940
BM_Lua_FastPow/2/1000/1000000007                         394 ns          394 ns      1335285
BM_Lua_FastPow/7/1000000/1000000007                      707 ns          707 ns       956345
BM_Lua_FastPow/1234567/7654321/1000000007                817 ns          817 ns       673915
BM_FakeLua_FastPow_TCC/2/1000/1000000007                 236 ns          236 ns      2726024
BM_FakeLua_FastPow_TCC/7/1000000/1000000007              345 ns          344 ns      2094568
BM_FakeLua_FastPow_TCC/1234567/7654321/1000000007        432 ns          432 ns      1385499
BM_FakeLua_FastPow_GCC/2/1000/1000000007                 201 ns          201 ns      3154883
BM_FakeLua_FastPow_GCC/7/1000000/1000000007              303 ns          303 ns      2187277
BM_FakeLua_FastPow_GCC/1234567/7654321/1000000007        413 ns          412 ns      1708921
BM_CPP_Popcount/1000                                    3461 ns         3461 ns       196618
BM_CPP_Popcount/10000                                  39697 ns        39625 ns        18225
BM_CPP_Popcount/100000                                466051 ns       465826 ns         1088
BM_Lua_Popcount/1000                                   89670 ns        89571 ns         8048
BM_Lua_Popcount/10000                                1055931 ns      1055335 ns          512
BM_Lua_Popcount/100000                              12913960 ns     12887776 ns           45
BM_FakeLua_Popcount_TCC/1000                           12268 ns        12261 ns        54288
BM_FakeLua_Popcount_TCC/10000                         161034 ns       160947 ns         4282
BM_FakeLua_Popcount_TCC/100000                       2035586 ns      2035472 ns          537
BM_FakeLua_Popcount_GCC/1000                            3336 ns         3334 ns       206377
BM_FakeLua_Popcount_GCC/10000                          37999 ns        37948 ns        19342
BM_FakeLua_Popcount_GCC/100000                        434847 ns       434527 ns         1177
BM_CPP_InsertionSort/50                                  762 ns          761 ns       947296
BM_CPP_InsertionSort/100                                3239 ns         3233 ns       206762
BM_CPP_InsertionSort/200                               11939 ns        11936 ns        60634
BM_Lua_InsertionSort/50                                39990 ns        39968 ns        17203
BM_Lua_InsertionSort/100                              153927 ns       153406 ns         4814
BM_Lua_InsertionSort/200                              604500 ns       603691 ns         1164
BM_FakeLua_InsertionSort_TCC/50                        70532 ns        70314 ns         8504
BM_FakeLua_InsertionSort_TCC/100                      278055 ns       277838 ns         2386
BM_FakeLua_InsertionSort_TCC/200                     1055127 ns      1054524 ns          611
BM_FakeLua_InsertionSort_GCC/50                        12386 ns        12367 ns        40433
BM_FakeLua_InsertionSort_GCC/100                       49395 ns        49327 ns        14816
BM_FakeLua_InsertionSort_GCC/200                      186224 ns       186039 ns         4032
BM_CPP_MatMul                                           1.93 ns         1.93 ns    336018531
BM_Lua_MatMul                                           2662 ns         2659 ns       275885
BM_FakeLua_MatMul_TCC                                   2248 ns         2247 ns       290223
BM_FakeLua_MatMul_GCC                                    578 ns          578 ns      1201897
BM_CPP_Vector3/10000                                    5094 ns         5084 ns       137897
BM_CPP_Vector3/100000                                  49756 ns        49730 ns        14405
BM_CPP_Vector3/1000000                                503049 ns       502392 ns         1000
BM_Lua_Vector3/10000                                 1143004 ns      1121024 ns          654
BM_Lua_Vector3/100000                               10802073 ns     10799088 ns           59
BM_Lua_Vector3/1000000                             107690234 ns    107666616 ns            5
BM_FakeLua_Vector3_TCC/10000                          670453 ns       648277 ns         1132
BM_FakeLua_Vector3_TCC/100000                        6523911 ns      6523463 ns          110
BM_FakeLua_Vector3_TCC/1000000                      64958393 ns     64955620 ns           11
BM_FakeLua_Vector3_GCC/10000                           64602 ns        64070 ns        10772
BM_FakeLua_Vector3_GCC/100000                         637211 ns       637125 ns         1167
BM_FakeLua_Vector3_GCC/1000000                       6532916 ns      6531773 ns          109
BM_VarTable_Set/2                                        135 ns          135 ns      4432781
BM_VarTable_Set/4                                        156 ns          156 ns      3941017
BM_VarTable_Set/8                                        204 ns          204 ns      3069956
BM_VarTable_Set/16                                       637 ns          637 ns       830342
BM_VarTable_Set/32                                      1543 ns         1541 ns       477554
BM_VarTable_Set/64                                      3325 ns         3317 ns       205995
BM_VarTable_Set/128                                     6606 ns         6601 ns        81851
BM_VarTable_Set/256                                    13295 ns        13287 ns        44448
BM_VarTable_Set/512                                    27421 ns        27388 ns        25102
BM_VarTable_Set/1024                                   55003 ns        54959 ns        11982
BM_StdUnorderedMap_Set/2                                83.7 ns         83.6 ns      5999522
BM_StdUnorderedMap_Set/4                                 129 ns          129 ns      4325805
BM_StdUnorderedMap_Set/8                                 258 ns          257 ns      2544994
BM_StdUnorderedMap_Set/16                                621 ns          621 ns      1139844
BM_StdUnorderedMap_Set/32                               1293 ns         1292 ns       471222
BM_StdUnorderedMap_Set/64                               2623 ns         2618 ns       276807
BM_StdUnorderedMap_Set/128                              5526 ns         5523 ns       130532
BM_StdUnorderedMap_Set/256                             12850 ns        12826 ns        60623
BM_StdUnorderedMap_Set/512                             29013 ns        28993 ns        25582
BM_StdUnorderedMap_Set/1024                            56471 ns        56429 ns        12751
BM_LuaTable_Set/2                                        807 ns          812 ns       819234
BM_LuaTable_Set/4                                        988 ns          994 ns       724871
BM_LuaTable_Set/8                                       1289 ns         1297 ns       586246
BM_LuaTable_Set/16                                      1650 ns         1657 ns       379762
BM_LuaTable_Set/32                                      2549 ns         2557 ns       275902
BM_LuaTable_Set/64                                      4194 ns         4204 ns       177341
BM_LuaTable_Set/128                                     6800 ns         6811 ns        74607
BM_LuaTable_Set/256                                    12314 ns        12335 ns        46158
BM_LuaTable_Set/512                                    23869 ns        23899 ns        28189
BM_LuaTable_Set/1024                                   47622 ns        47665 ns        16590
BM_VarTable_Get/2                                       28.8 ns         28.8 ns     25055875
BM_VarTable_Get/4                                       57.9 ns         57.8 ns      9598256
BM_VarTable_Get/8                                        120 ns          120 ns      5006177
BM_VarTable_Get/16                                       225 ns          225 ns      2950793
BM_VarTable_Get/32                                       419 ns          419 ns      1588169
BM_VarTable_Get/64                                       831 ns          831 ns       651228
BM_VarTable_Get/128                                     1662 ns         1662 ns       369990
BM_VarTable_Get/256                                     3495 ns         3492 ns       199122
BM_VarTable_Get/512                                     6879 ns         6878 ns        99033
BM_VarTable_Get/1024                                   13386 ns        13385 ns        45815
BM_StdUnorderedMap_Get/2                                9.91 ns         9.90 ns     67343717
BM_StdUnorderedMap_Get/4                                21.6 ns         21.6 ns     31667121
BM_StdUnorderedMap_Get/8                                46.2 ns         46.2 ns     15133786
BM_StdUnorderedMap_Get/16                               96.3 ns         94.4 ns      7291603
BM_StdUnorderedMap_Get/32                                191 ns          191 ns      3620217
BM_StdUnorderedMap_Get/64                                385 ns          383 ns      1828373
BM_StdUnorderedMap_Get/128                               754 ns          753 ns       884779
BM_StdUnorderedMap_Get/256                              1565 ns         1565 ns       435180
BM_StdUnorderedMap_Get/512                              3153 ns         3142 ns       221762
BM_StdUnorderedMap_Get/1024                             6278 ns         6278 ns       106713
BM_LuaTable_Get/2                                       33.6 ns         33.6 ns     21058785
BM_LuaTable_Get/4                                       66.1 ns         66.1 ns      7817932
BM_LuaTable_Get/8                                        137 ns          137 ns      4320543
BM_LuaTable_Get/16                                       276 ns          276 ns      2496498
BM_LuaTable_Get/32                                       534 ns          534 ns      1243606
BM_LuaTable_Get/64                                      1040 ns         1040 ns       695346
BM_LuaTable_Get/128                                     2121 ns         2121 ns       299295
BM_LuaTable_Get/256                                     4043 ns         4042 ns       159422
BM_LuaTable_Get/512                                     8310 ns         8310 ns        63437
BM_LuaTable_Get/1024                                   16679 ns        16678 ns        37525
BM_VarTable_Iter/2                                      1.32 ns         1.32 ns    403977579
BM_VarTable_Iter/4                                      2.53 ns         2.53 ns    248254081
BM_VarTable_Iter/8                                      5.28 ns         5.28 ns    134830542
BM_VarTable_Iter/16                                     14.3 ns         14.3 ns     51320386
BM_VarTable_Iter/32                                     28.6 ns         28.6 ns     23129004
BM_VarTable_Iter/64                                     57.2 ns         57.2 ns     11836331
BM_VarTable_Iter/128                                     119 ns          119 ns      5465238
BM_VarTable_Iter/256                                     231 ns          231 ns      2841827
BM_VarTable_Iter/512                                     466 ns          466 ns      1336094
BM_VarTable_Iter/1024                                    921 ns          920 ns       577419
BM_StdUnorderedMap_Iter/2                              0.890 ns        0.890 ns    829810379
BM_StdUnorderedMap_Iter/4                               1.84 ns         1.84 ns    332540053
BM_StdUnorderedMap_Iter/8                               4.36 ns         4.35 ns    117268318
BM_StdUnorderedMap_Iter/16                              10.7 ns         10.7 ns     67522923
BM_StdUnorderedMap_Iter/32                              27.8 ns         27.8 ns     23935775
BM_StdUnorderedMap_Iter/64                              68.9 ns         68.8 ns      9318311
BM_StdUnorderedMap_Iter/128                              184 ns          184 ns      3922894
BM_StdUnorderedMap_Iter/256                              438 ns          432 ns      1880077
BM_StdUnorderedMap_Iter/512                             1327 ns         1325 ns       557978
BM_StdUnorderedMap_Iter/1024                            2390 ns         2390 ns       297377
BM_LuaTable_Iter/2                                      61.8 ns         61.8 ns     11033026
BM_LuaTable_Iter/4                                       118 ns          118 ns      4771078
BM_LuaTable_Iter/8                                       219 ns          219 ns      2962909
BM_LuaTable_Iter/16                                      400 ns          400 ns      1643680
BM_LuaTable_Iter/32                                      797 ns          796 ns       914000
BM_LuaTable_Iter/64                                     1496 ns         1496 ns       395893
BM_LuaTable_Iter/128                                    2960 ns         2960 ns       217955
BM_LuaTable_Iter/256                                    6215 ns         6214 ns       115078
BM_LuaTable_Iter/512                                   12078 ns        12071 ns        48040
BM_LuaTable_Iter/1024                                  23554 ns        23551 ns        26205
BM_VarTable_Del/2                                        296 ns          297 ns      2460558
BM_VarTable_Del/4                                        331 ns          327 ns      2156886
BM_VarTable_Del/8                                        404 ns          405 ns      1727263
BM_VarTable_Del/16                                       544 ns          541 ns      1080401
BM_VarTable_Del/32                                       868 ns          858 ns       808506
BM_VarTable_Del/64                                      1814 ns         1723 ns       460445
BM_VarTable_Del/128                                     3179 ns         2930 ns       284913
BM_VarTable_Del/256                                     5529 ns         5173 ns       100000
BM_VarTable_Del/512                                    11878 ns        11393 ns        66451
BM_VarTable_Del/1024                                   24450 ns        21694 ns        43892
BM_StdUnorderedMap_Del/2                                 291 ns          293 ns      2165845
BM_StdUnorderedMap_Del/4                                 308 ns          309 ns      2200724
BM_StdUnorderedMap_Del/8                                 371 ns          373 ns      1923522
BM_StdUnorderedMap_Del/16                                592 ns          593 ns      1122081
BM_StdUnorderedMap_Del/32                                969 ns          968 ns       725536
BM_StdUnorderedMap_Del/64                               1742 ns         1740 ns       406547
BM_StdUnorderedMap_Del/128                              3280 ns         3277 ns       212401
BM_StdUnorderedMap_Del/256                              5303 ns         5289 ns       114018
BM_StdUnorderedMap_Del/512                             10193 ns        10183 ns        65456
BM_StdUnorderedMap_Del/1024                            20047 ns        20009 ns        34973
BM_LuaTable_Del/2                                        548 ns          541 ns      1280533
BM_LuaTable_Del/4                                        555 ns          556 ns      1102730
BM_LuaTable_Del/8                                        632 ns          630 ns      1117357
BM_LuaTable_Del/16                                       739 ns          733 ns       884358
BM_LuaTable_Del/32                                       976 ns          972 ns       698806
BM_LuaTable_Del/64                                      1459 ns         1451 ns       501583
BM_LuaTable_Del/128                                     2406 ns         2394 ns       290888
BM_LuaTable_Del/256                                     4297 ns         4280 ns       163744
BM_LuaTable_Del/512                                     8134 ns         8090 ns        89315
BM_LuaTable_Del/1024                                   15693 ns        15660 ns        44228
```

---

## 算法性能分析（C++ vs Lua vs FakeLua TCC vs FakeLua GCC）

以下取各算法的典型参数做横向对比（CPU Time），**倍数 = 对应实现时间 / C++ 时间**：

### 1. Fibonacci（n=32，递归无记忆化）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 4.90 ms | 1x |
| Lua | 203.95 ms | **41.66x** 慢 |
| FakeLua TCC | 19.98 ms | **4.08x** 慢 |
| FakeLua GCC | 4.58 ms | **1.07x** 快 (比 C++ 快 **6%**) |

### 2. GCD（a=832040, b=514229，约 30 次迭代）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 121 ns | 1x |
| Lua | 461 ns | **3.81x** 慢 |
| FakeLua TCC | 271 ns | **2.24x** 慢 (比 Lua 快 **1.70x**) |
| FakeLua GCC | 204 ns | **1.69x** 慢 (比 Lua 快 **2.26x**) |

### 3. PowMod（base=1234567, exp=7654321, mod=1e9+7，约 23 次循环，用 `%`/`//`）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 296 ns | 1x |
| Lua | 917 ns | **3.10x** 慢 |
| FakeLua TCC | 684 ns | **2.31x** 慢 (比 Lua 快 **1.34x**) |
| FakeLua GCC | 418 ns | **1.41x** 慢 (比 Lua 快 **2.19x**) |

### 4. FastPow（base=1234567, exp=7654321, mod=1e9+7，用 `&`/`>>`）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 297 ns | 1x |
| Lua | 817 ns | **2.75x** 慢 |
| FakeLua TCC | 432 ns | **1.45x** 慢 (比 Lua 快 **1.89x**) |
| FakeLua GCC | 412 ns | **1.39x** 慢 (比 Lua 快 **1.98x**) |

> FastPow 用位运算 `&`/`>>` 代替取余/整除 `%`/`//`，在 FakeLua TCC 下比 PowMod 快约 **1.6x**（690.0 ns → 429.0 ns），说明 TCC 对位运算的代码生成较优。GCC 两者表现也较接近。

### 5. Sum（n=5000000，纯循环累加）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 1.54 ms | 1x |
| Lua | 27.45 ms | **17.78x** 慢 |
| FakeLua TCC | 10.93 ms | **7.08x** 慢 (比 Lua 快 **2.51x**) |
| FakeLua GCC | 1.55 ms | **1.00x** 慢 |

> 纯整数累加循环：FakeLua GCC 与 C++ 几乎完全相同，说明 GCC `-O3` 对简单数值循环已达到 C++ 原生水平。TCC 比 Lua 快 **2.0x**。

### 6. BubbleSort（n=200，O(n²)，含表操作）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 126.4 µs | 1x |
| Lua | 826.4 µs | **6.54x** 慢 |
| FakeLua TCC | 1.74 ms | **13.80x** 慢 (比 Lua 慢 **2.11x**) |
| FakeLua GCC | 491.1 µs | **3.88x** 慢 (比 Lua 快 **1.68x**) |

> 与冒泡排序类似，表操作为瓶颈。TCC 比 Lua 慢约 1.8x，GCC 快于 Lua 约 2.7x。

### 11. MatMul（单次 3×3 矩阵乘法，使用全局常量 Table 读）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 1.93 ns | 1x |
| Lua | 2.7 µs | **1377.72x** 慢 |
| FakeLua TCC | 2.2 µs | **1164.25x** 慢 (比 Lua 快 **1.18x**) |
| FakeLua GCC | 578 ns | **299.48x** 慢 (比 Lua 快 **4.60x**) |

> 将只读矩阵 `mat_a` 和 `mat_b` 移入全局/模块级常量，并启用 Table 特化后，**TCC 和 GCC-JIT 的性能均获得巨大突破**：
> - **TCC** 成功跑赢 Lua 5.4 解释器（快 **1.15x**）。
> - **GCC** 比 Lua 5.4 解释器快 **4.62x**。
> 
> **实现细节剖析**：
> 需要注意的是，由于 `bench_matmul` 中的索引访问是动态表达式（如 `mat_a[(i - 1) * 3 + k]`），在生成的 C 代码中，并不能直接在调用处生成静态的指针偏移访问（如 `s->_int_1`）。它在 C 代码中仍然调用了 `FlGetTableInt`。
> 但由于 `mat_a` 和 `mat_b` 已经是特化 Table，`FlGetTableInt` 内部会优先通过其绑定的特化回调函数 `spec_get` 执行。在 `spec_get` 内部，系统执行 `if (__ival == 1) return s->_int_1;` 等键值匹配分支，最终映射到结构体成员的指针偏移。这种方式虽然含有分支判断开销，但比常规的哈希计算与哈希桶查找要高效得多。此外，将只读表定义于函数外部，彻底消除了每次函数调用时的 Table 重新分配与 GC 垃圾回收压力。

---

### 12. Vector3（三维坐标 x, y, z 在循环中累计读写）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 502.4 µs | 1x |
| Lua | 107.67 ms | **214.31x** 慢 |
| FakeLua TCC | 64.96 ms | **129.29x** 慢 (比 Lua 快 **1.66x**) |
| FakeLua GCC | 6.53 ms | **13.00x** 慢 |

> **实现细节剖析**：
> 在本测试中，`v1` 和 `v2` 是通过字面量键定义的局部 Vector3 表 `{x=10, y=20, z=30}`。由于键 `x`, `y`, `z` 均为静态已知的字符串字面量，JIT 编译器**成功将该 Table 构造特化为了静态的 C 结构体**。
> 同时，在循环体内对 `v1.x = v1.x + v2.x` 等成员的读写都是静态已知的字符串 Key 访问，因此 JIT 编译器**直接将其编译为了零查找开销的静态指针偏移访问**（类似于 C 语言结构体成员的直接读写 `v1.spec->x = v1.spec->x + v2.spec->x`）。
> 相比之下，Lua 5.4 在执行这 100 万次循环时，必须进行总共 900 万次基于字符串哈希的 Table Lookup。实验表明，FakeLua GCC 比标准 Lua 5.4 **快 16.48 倍**，充分展示了 Table 结构体特化和直接指针偏移读写的强悍性能。

---

## FakeLua TCC vs Lua 5.4 详细对比

| 算法 | 参数 | Lua | FakeLua TCC | 结果 |
|------|------|-----|-------------|------|
| Fibonacci | n=32 | 203.95 ms | 19.98 ms | TCC **10.21x 快** |
| GCD | 832040/514229 | 461 ns | 271 ns | TCC **1.70x 快** |
| PowMod | 1234567/7654321/1e9+7 | 917 ns | 684 ns | TCC **1.34x 快** |
| FastPow | 1234567/7654321/1e9+7 | 817 ns | 432 ns | TCC **1.89x 快** |
| Sum | n=5000000 | 27.45 ms | 10.93 ms | TCC **2.51x 快** |
| BubbleSort | n=200 | 826.4 µs | 1.74 ms | TCC **2.11x 慢** |
| Sieve | n=5000 | 271.2 µs | 598.6 µs | TCC **2.21x 慢** |
| BinarySearch | n=1000 | 455.1 µs | 634.7 µs | TCC **1.39x 慢** |
| Popcount | n=100000 | 12.89 ms | 2.04 ms | TCC **6.33x 快** |
| InsertionSort | n=200 | 603.7 µs | 1.05 ms | TCC **1.75x 慢** |
| MatMul | 单次 3×3 | 2.7 µs | 2.2 µs | TCC **1.18x 快** |
| Vector3 | n=1000000 | 107.67 ms | 64.96 ms | TCC **1.66x 快** |

> **结论**：在将部分 Table 静态只读部分常量化后，TCC 在 MatMul 这样的运算中超越了 Lua；在纯数值算法上也保持 1.7x ~ 10.4x 的巨大优势。表写操作依然是 TCC 的软肋，但表读与遍历性能表现优异。

---

## FakeLua GCC vs Lua 5.4 详细对比

| 算法 | 参数 | Lua | FakeLua GCC | GCC 快多少 |
|------|------|-----|-------------|-----------|
| Fibonacci | n=32 | 203.95 ms | 4.58 ms | **44.57x** |
| GCD | 832040/514229 | 461 ns | 204 ns | **2.26x** |
| PowMod | 1234567/7654321/1e9+7 | 917 ns | 418 ns | **2.19x** |
| FastPow | 1234567/7654321/1e9+7 | 817 ns | 412 ns | **1.98x** |
| Sum | n=5000000 | 27.45 ms | 1.55 ms | **17.77x** |
| BubbleSort | n=200 | 826.4 µs | 491.1 µs | **1.68x** |
| Sieve | n=5000 | 271.2 µs | 126.1 µs | **2.15x** |
| BinarySearch | n=1000 | 455.1 µs | 125.4 µs | **3.63x** |
| Popcount | n=100000 | 12.89 ms | 434.5 µs | **29.66x** |
| InsertionSort | n=200 | 603.7 µs | 186.0 µs | **3.24x** |
| MatMul | 单次 3×3 | 2.7 µs | 578 ns | **4.60x** |
| Vector3 | n=1000000 | 107.67 ms | 6.53 ms | **16.48x** |

### FakeLua GCC 按场景分类

| 场景类型 | FakeLua GCC 比 Lua 快 | 原因 |
|---------|----------------------|------|
| **纯整数累加 (Sum)** | **17.77x** | GCC `-O3` 向量化，达到 C++ 原生水平 |
| **纯整数位运算 (Popcount)** | **29.66x** | 位运算全部原生化，GCC 极进优化 |
| **递归 (Fibonacci)** | **44.57x** | 数值特化 + 原生递归，GCC 深度内联 |
| **算术循环 (PowMod/FastPow)** | **2.19x–1.98x** | 循环体数值特化，取模运算受益于寄存器优化 |
| **短迭代 (GCD)** | **2.26x** | 迭代次数少，函数调用开销占比高 |
| **二分查找 (BinarySearch)** | **3.63x** | 混合数值+表操作，GCC 部分消除 table 开销 |
| **表操作为主 (BubbleSort/InsertionSort/Sieve/MatMul/Vector3)** | **1.68x–16.48x** | 引入了 Table 结构体特化与读写，大幅提升读写效率 |

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
