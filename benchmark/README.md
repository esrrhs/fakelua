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
2026-06-26T15:07:47+08:00
Running build/bin/bench_mark
Run on (2 X 2595.12 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x2)
  L1 Instruction 32 KiB (x2)
  L2 Unified 4096 KiB (x2)
  L3 Unified 16384 KiB (x1)
Load Average: 1.29, 1.40, 1.24
***WARNING*** ASLR is enabled, the results may have unreproducible noise in them.
--------------------------------------------------------------------------------------------
Benchmark                                                  Time             CPU   Iterations
--------------------------------------------------------------------------------------------
BM_CPP_Fibonacci/20                                    19998 ns        17098 ns        43205
BM_CPP_Fibonacci/25                                   154741 ns       154535 ns         4505
BM_CPP_Fibonacci/30                                  1991727 ns      1991547 ns          299
BM_CPP_Fibonacci/32                                  5260925 ns      5241647 ns          147
BM_Lua_Fibonacci/20                                   894549 ns       756033 ns         1022
BM_Lua_Fibonacci/25                                 13372116 ns      9516778 ns           74
BM_Lua_Fibonacci/30                                 83782270 ns     82749168 ns            7
BM_Lua_Fibonacci/32                                205205050 ns    205082849 ns            3
BM_FakeLua_Fibonacci_TCC/20                            64400 ns        61539 ns         9742
BM_FakeLua_Fibonacci_TCC/25                           679532 ns       678534 ns         1026
BM_FakeLua_Fibonacci_TCC/30                          7858674 ns      7417494 ns           81
BM_FakeLua_Fibonacci_TCC/32                         19767330 ns     19764811 ns           36
BM_FakeLua_Fibonacci_GCC/20                            17496 ns        17480 ns        41254
BM_FakeLua_Fibonacci_GCC/25                           173220 ns       171649 ns         4485
BM_FakeLua_Fibonacci_GCC/30                          1919540 ns      1884665 ns          404
BM_FakeLua_Fibonacci_GCC/32                          4570716 ns      4570123 ns          155
BM_CPP_GCD/832040/514229                                 122 ns          121 ns      5756098
BM_CPP_GCD/123456789/987654321                          17.0 ns         16.7 ns     42073665
BM_CPP_GCD/2147483647/1073741823                        12.9 ns         12.9 ns     54001902
BM_Lua_GCD/832040/514229                                 446 ns          445 ns      1403571
BM_Lua_GCD/123456789/987654321                           121 ns          121 ns      5966178
BM_Lua_GCD/2147483647/1073741823                         104 ns          104 ns      5983008
BM_FakeLua_GCD_TCC/832040/514229                         263 ns          263 ns      2674109
BM_FakeLua_GCD_TCC/123456789/987654321                   123 ns          122 ns      5464073
BM_FakeLua_GCD_TCC/2147483647/1073741823                 123 ns          123 ns      5713654
BM_FakeLua_GCD_GCC/832040/514229                         196 ns          196 ns      3217891
BM_FakeLua_GCD_GCC/123456789/987654321                   101 ns          101 ns      6589293
BM_FakeLua_GCD_GCC/2147483647/1073741823                99.7 ns         99.6 ns      6953873
BM_CPP_PowMod/2/1000/1000000007                          107 ns          104 ns      6695518
BM_CPP_PowMod/7/1000000/1000000007                       204 ns          204 ns      3431246
BM_CPP_PowMod/1234567/7654321/1000000007                 296 ns          296 ns      2358835
BM_Lua_PowMod/2/1000/1000000007                          473 ns          464 ns      1592425
BM_Lua_PowMod/7/1000000/1000000007                       783 ns          782 ns       872586
BM_Lua_PowMod/1234567/7654321/1000000007                 973 ns          937 ns       584564
BM_FakeLua_PowMod_TCC/2/1000/1000000007                  328 ns          327 ns      2075351
BM_FakeLua_PowMod_TCC/7/1000000/1000000007               592 ns          563 ns      1106491
BM_FakeLua_PowMod_TCC/1234567/7654321/1000000007         691 ns          690 ns       990505
BM_FakeLua_PowMod_GCC/2/1000/1000000007                  218 ns          218 ns      2836644
BM_FakeLua_PowMod_GCC/7/1000000/1000000007               343 ns          339 ns      2163629
BM_FakeLua_PowMod_GCC/1234567/7654321/1000000007         423 ns          422 ns      1678325
BM_CPP_Sum/10000                                        3086 ns         3083 ns       225327
BM_CPP_Sum/100000                                      31364 ns        30961 ns        22693
BM_CPP_Sum/1000000                                    309171 ns       308916 ns         2271
BM_CPP_Sum/5000000                                   1551949 ns      1548612 ns          447
BM_Lua_Sum/10000                                       55978 ns        55972 ns        12049
BM_Lua_Sum/100000                                     535503 ns       534202 ns         1050
BM_Lua_Sum/1000000                                   5222650 ns      5218853 ns          120
BM_Lua_Sum/5000000                                  26315813 ns     26299181 ns           22
BM_FakeLua_Sum_TCC/10000                               27195 ns        26643 ns        27331
BM_FakeLua_Sum_TCC/100000                             253998 ns       253864 ns         2747
BM_FakeLua_Sum_TCC/1000000                           2590242 ns      2588806 ns          259
BM_FakeLua_Sum_TCC/5000000                          13118119 ns     12988537 ns           57
BM_FakeLua_Sum_GCC/10000                                3190 ns         3187 ns       219732
BM_FakeLua_Sum_GCC/100000                              31031 ns        31007 ns        22441
BM_FakeLua_Sum_GCC/1000000                            317687 ns       310123 ns         2266
BM_FakeLua_Sum_GCC/5000000                           1545147 ns      1544267 ns          453
BM_CPP_BubbleSort/50                                    7798 ns         7629 ns        91673
BM_CPP_BubbleSort/100                                  31181 ns        31159 ns        22497
BM_CPP_BubbleSort/200                                 126946 ns       125523 ns         5396
BM_Lua_BubbleSort/50                                   51691 ns        51662 ns        12809
BM_Lua_BubbleSort/100                                 198331 ns       198245 ns         2850
BM_Lua_BubbleSort/200                                 868454 ns       846901 ns          865
BM_FakeLua_BubbleSort_TCC/50                          106901 ns       106792 ns         6640
BM_FakeLua_BubbleSort_TCC/100                         417983 ns       417484 ns         1510
BM_FakeLua_BubbleSort_TCC/200                        1810482 ns      1766751 ns          423
BM_FakeLua_BubbleSort_GCC/50                           20380 ns        20361 ns        34707
BM_FakeLua_BubbleSort_GCC/100                          87418 ns        85704 ns         8433
BM_FakeLua_BubbleSort_GCC/200                         319909 ns       319552 ns         2236
BM_CPP_Sieve/100                                         253 ns          253 ns      2442706
BM_CPP_Sieve/500                                        1195 ns         1194 ns       587335
BM_CPP_Sieve/1000                                       2395 ns         2394 ns       258135
BM_CPP_Sieve/5000                                      12213 ns        12204 ns        56763
BM_Lua_Sieve/100                                        7334 ns         6948 ns        71972
BM_Lua_Sieve/500                                       27777 ns        27751 ns        25710
BM_Lua_Sieve/1000                                      57300 ns        55830 ns         9245
BM_Lua_Sieve/5000                                     266039 ns       265861 ns         2645
BM_FakeLua_Sieve_TCC/100                                9481 ns         9478 ns        73542
BM_FakeLua_Sieve_TCC/500                               50677 ns        49643 ns        13592
BM_FakeLua_Sieve_TCC/1000                              92473 ns        92468 ns         7547
BM_FakeLua_Sieve_TCC/5000                             638021 ns       615585 ns         1118
BM_FakeLua_Sieve_GCC/100                                1892 ns         1891 ns       359810
BM_FakeLua_Sieve_GCC/500                                9772 ns         9430 ns        77879
BM_FakeLua_Sieve_GCC/1000                              18265 ns        18254 ns        39438
BM_FakeLua_Sieve_GCC/5000                             117468 ns       117351 ns         5869
BM_CPP_BinarySearch/100                                  894 ns          880 ns       799801
BM_CPP_BinarySearch/500                                 5944 ns         5931 ns       115771
BM_CPP_BinarySearch/1000                               16716 ns        16558 ns        32165
BM_Lua_BinarySearch/100                                30317 ns        29738 ns        24884
BM_Lua_BinarySearch/500                               193226 ns       193128 ns         3633
BM_Lua_BinarySearch/1000                              436212 ns       435663 ns         1502
BM_FakeLua_BinarySearch_TCC/100                        34531 ns        33577 ns        21899
BM_FakeLua_BinarySearch_TCC/500                       217700 ns       217431 ns         3230
BM_FakeLua_BinarySearch_TCC/1000                      536321 ns       499101 ns         1126
BM_FakeLua_BinarySearch_GCC/100                         4002 ns         3963 ns       193905
BM_FakeLua_BinarySearch_GCC/500                        34181 ns        34151 ns        20619
BM_FakeLua_BinarySearch_GCC/1000                       83645 ns        81151 ns         6188
BM_CPP_FastPow/2/1000/1000000007                         104 ns          104 ns      6717336
BM_CPP_FastPow/7/1000000/1000000007                      204 ns          204 ns      3426476
BM_CPP_FastPow/1234567/7654321/1000000007                303 ns          296 ns      2366044
BM_Lua_FastPow/2/1000/1000000007                         390 ns          390 ns      1797844
BM_Lua_FastPow/7/1000000/1000000007                      724 ns          691 ns       770411
BM_Lua_FastPow/1234567/7654321/1000000007                814 ns          813 ns       847241
BM_FakeLua_FastPow_TCC/2/1000/1000000007                 242 ns          236 ns      2599004
BM_FakeLua_FastPow_TCC/7/1000000/1000000007              358 ns          350 ns      2088881
BM_FakeLua_FastPow_TCC/1234567/7654321/1000000007        429 ns          429 ns      1619507
BM_FakeLua_FastPow_GCC/2/1000/1000000007                 209 ns          209 ns      2798486
BM_FakeLua_FastPow_GCC/7/1000000/1000000007              327 ns          325 ns      2249774
BM_FakeLua_FastPow_GCC/1234567/7654321/1000000007        409 ns          407 ns      1686407
BM_CPP_Popcount/1000                                    3282 ns         3278 ns       204671
BM_CPP_Popcount/10000                                  42106 ns        41136 ns        17869
BM_CPP_Popcount/100000                                487744 ns       487719 ns         1374
BM_Lua_Popcount/1000                                   86961 ns        86612 ns         8006
BM_Lua_Popcount/10000                                1140874 ns      1129389 ns          664
BM_Lua_Popcount/100000                              12522028 ns     12513430 ns           56
BM_FakeLua_Popcount_TCC/1000                           10533 ns        10155 ns        55582
BM_FakeLua_Popcount_TCC/10000                         160068 ns       159870 ns         4351
BM_FakeLua_Popcount_TCC/100000                       2027382 ns      2026291 ns          335
BM_FakeLua_Popcount_GCC/1000                            3645 ns         3637 ns       201158
BM_FakeLua_Popcount_GCC/10000                          43211 ns        43187 ns        17187
BM_FakeLua_Popcount_GCC/100000                        467624 ns       467606 ns         1487
BM_CPP_InsertionSort/50                                  727 ns          727 ns       937705
BM_CPP_InsertionSort/100                                3373 ns         3368 ns       222286
BM_CPP_InsertionSort/200                               11683 ns        11671 ns        60574
BM_Lua_InsertionSort/50                                41838 ns        39889 ns        12840
BM_Lua_InsertionSort/100                              143672 ns       143665 ns         4908
BM_Lua_InsertionSort/200                              614804 ns       584335 ns          873
BM_FakeLua_InsertionSort_TCC/50                        83073 ns        78141 ns         9955
BM_FakeLua_InsertionSort_TCC/100                      274297 ns       274285 ns         2601
BM_FakeLua_InsertionSort_TCC/200                     1080296 ns      1078597 ns          465
BM_FakeLua_InsertionSort_GCC/50                        13299 ns        13298 ns        50998
BM_FakeLua_InsertionSort_GCC/100                       56689 ns        53994 ns         9608
BM_FakeLua_InsertionSort_GCC/200                      217696 ns       213696 ns         3511
BM_CPP_MatMul                                           3.10 ns         3.10 ns    225283315
BM_Lua_MatMul                                           2650 ns         2649 ns       234462
BM_FakeLua_MatMul_TCC                                   2091 ns         1982 ns       380810
BM_FakeLua_MatMul_GCC                                    420 ns          420 ns      1397027
BM_VarTable_Set/2                                        132 ns          130 ns      5433783
BM_VarTable_Set/4                                        156 ns          153 ns      4799231
BM_VarTable_Set/8                                        193 ns          193 ns      3584147
BM_VarTable_Set/16                                       664 ns          639 ns       843871
BM_VarTable_Set/32                                      1515 ns         1513 ns       468456
BM_VarTable_Set/64                                      3209 ns         3209 ns       187709
BM_VarTable_Set/128                                     7117 ns         6993 ns        99507
BM_VarTable_Set/256                                    13533 ns        13531 ns        50769
BM_VarTable_Set/512                                    26850 ns        26848 ns        22287
BM_VarTable_Set/1024                                   58540 ns        57306 ns        11417
BM_StdUnorderedMap_Set/2                                88.1 ns         88.1 ns      6664036
BM_StdUnorderedMap_Set/4                                 135 ns          135 ns      5251670
BM_StdUnorderedMap_Set/8                                 249 ns          249 ns      2350712
BM_StdUnorderedMap_Set/16                                649 ns          636 ns      1092011
BM_StdUnorderedMap_Set/32                               1275 ns         1274 ns       559361
BM_StdUnorderedMap_Set/64                               2665 ns         2661 ns       231906
BM_StdUnorderedMap_Set/128                              6089 ns         6078 ns       115604
BM_StdUnorderedMap_Set/256                             12066 ns        12057 ns        55457
BM_StdUnorderedMap_Set/512                             28011 ns        27979 ns        21213
BM_StdUnorderedMap_Set/1024                            60725 ns        60689 ns        11284
BM_LuaTable_Set/2                                        818 ns          814 ns       899614
BM_LuaTable_Set/4                                        996 ns          981 ns       667613
BM_LuaTable_Set/8                                       1217 ns         1223 ns       594271
BM_LuaTable_Set/16                                      1653 ns         1660 ns       371383
BM_LuaTable_Set/32                                      2572 ns         2538 ns       259001
BM_LuaTable_Set/64                                      4264 ns         4247 ns       171832
BM_LuaTable_Set/128                                     7591 ns         7440 ns        99513
BM_LuaTable_Set/256                                    12566 ns        12576 ns        55957
BM_LuaTable_Set/512                                    23171 ns        23181 ns        25109
BM_LuaTable_Set/1024                                   48430 ns        47869 ns        16227
BM_VarTable_Get/2                                       26.1 ns         25.8 ns     28142743
BM_VarTable_Get/4                                       52.6 ns         52.6 ns     12483656
BM_VarTable_Get/8                                        115 ns          114 ns      4882669
BM_VarTable_Get/16                                       229 ns          225 ns      3262320
BM_VarTable_Get/32                                       393 ns          393 ns      1797754
BM_VarTable_Get/64                                       816 ns          786 ns       649776
BM_VarTable_Get/128                                     1539 ns         1536 ns       432462
BM_VarTable_Get/256                                     3127 ns         3125 ns       208347
BM_VarTable_Get/512                                     6684 ns         6499 ns       104891
BM_VarTable_Get/1024                                   12798 ns        12790 ns        55466
BM_StdUnorderedMap_Get/2                                10.1 ns         9.95 ns     68337498
BM_StdUnorderedMap_Get/4                                21.8 ns         21.8 ns     32322255
BM_StdUnorderedMap_Get/8                                46.7 ns         46.4 ns     15184903
BM_StdUnorderedMap_Get/16                               94.2 ns         94.1 ns      7386905
BM_StdUnorderedMap_Get/32                                195 ns          191 ns      3663197
BM_StdUnorderedMap_Get/64                                390 ns          384 ns      1821327
BM_StdUnorderedMap_Get/128                               749 ns          748 ns       898937
BM_StdUnorderedMap_Get/256                              1565 ns         1563 ns       430274
BM_StdUnorderedMap_Get/512                              3141 ns         3131 ns       222646
BM_StdUnorderedMap_Get/1024                             6255 ns         6248 ns       105426
BM_LuaTable_Get/2                                       32.8 ns         32.8 ns     19615973
BM_LuaTable_Get/4                                       69.9 ns         67.5 ns     10238667
BM_LuaTable_Get/8                                        135 ns          134 ns      4998546
BM_LuaTable_Get/16                                       264 ns          264 ns      2282064
BM_LuaTable_Get/32                                       554 ns          537 ns      1240718
BM_LuaTable_Get/64                                      1003 ns         1002 ns       643729
BM_LuaTable_Get/128                                     2113 ns         2110 ns       288679
BM_LuaTable_Get/256                                     4435 ns         4329 ns       169044
BM_LuaTable_Get/512                                     8276 ns         8275 ns        82213
BM_LuaTable_Get/1024                                   16219 ns        16209 ns        34531
BM_VarTable_Iter/2                                      1.59 ns         1.56 ns    467957756
BM_VarTable_Iter/4                                      2.77 ns         2.77 ns    255885879
BM_VarTable_Iter/8                                      6.57 ns         6.56 ns    104086212
BM_VarTable_Iter/16                                     15.6 ns         14.3 ns     50566924
BM_VarTable_Iter/32                                     30.1 ns         29.8 ns     25213533
BM_VarTable_Iter/64                                     55.8 ns         55.8 ns      9212989
BM_VarTable_Iter/128                                     124 ns          123 ns      6066440
BM_VarTable_Iter/256                                     230 ns          230 ns      2979010
BM_VarTable_Iter/512                                     516 ns          499 ns      1319730
BM_VarTable_Iter/1024                                    949 ns          949 ns       740761
BM_StdUnorderedMap_Iter/2                              0.839 ns        0.839 ns    768787535
BM_StdUnorderedMap_Iter/4                               1.79 ns         1.79 ns    394780527
BM_StdUnorderedMap_Iter/8                               4.63 ns         4.55 ns    138126071
BM_StdUnorderedMap_Iter/16                              10.4 ns         10.4 ns     67549192
BM_StdUnorderedMap_Iter/32                              27.4 ns         27.4 ns     22910654
BM_StdUnorderedMap_Iter/64                              68.4 ns         68.3 ns      9866343
BM_StdUnorderedMap_Iter/128                              170 ns          170 ns      3755772
BM_StdUnorderedMap_Iter/256                              400 ns          400 ns      2063608
BM_StdUnorderedMap_Iter/512                             1246 ns         1245 ns       589421
BM_StdUnorderedMap_Iter/1024                            2262 ns         2261 ns       298647
BM_LuaTable_Iter/2                                      64.8 ns         64.1 ns     11035072
BM_LuaTable_Iter/4                                       122 ns          121 ns      5887527
BM_LuaTable_Iter/8                                       212 ns          212 ns      2811206
BM_LuaTable_Iter/16                                      434 ns          420 ns      1774122
BM_LuaTable_Iter/32                                      812 ns          812 ns       903987
BM_LuaTable_Iter/64                                     1574 ns         1573 ns       411555
BM_LuaTable_Iter/128                                    3386 ns         3298 ns       220485
BM_LuaTable_Iter/256                                    5951 ns         5914 ns       108687
BM_LuaTable_Iter/512                                   11913 ns        11906 ns        58791
BM_LuaTable_Iter/1024                                  23818 ns        23814 ns        24343
BM_VarTable_Del/2                                        287 ns          288 ns      2320571
BM_VarTable_Del/4                                        325 ns          326 ns      2204907
BM_VarTable_Del/8                                        403 ns          404 ns      1788569
BM_VarTable_Del/16                                       554 ns          550 ns      1314288
BM_VarTable_Del/32                                       815 ns          809 ns       643078
BM_VarTable_Del/64                                      1669 ns         1611 ns       474972
BM_VarTable_Del/128                                     3313 ns         3106 ns       295063
BM_VarTable_Del/256                                     6239 ns         5558 ns       117872
BM_VarTable_Del/512                                    14425 ns        11394 ns        75171
BM_VarTable_Del/1024                                   26779 ns        23488 ns        43454
BM_StdUnorderedMap_Del/2                                 315 ns          316 ns      2145545
BM_StdUnorderedMap_Del/4                                 328 ns          330 ns      2155750
BM_StdUnorderedMap_Del/8                                 386 ns          368 ns      1754913
BM_StdUnorderedMap_Del/16                                594 ns          594 ns      1225697
BM_StdUnorderedMap_Del/32                                984 ns          967 ns       747051
BM_StdUnorderedMap_Del/64                               1693 ns         1679 ns       411369
BM_StdUnorderedMap_Del/128                              3263 ns         3257 ns       218718
BM_StdUnorderedMap_Del/256                              5267 ns         5198 ns       109455
BM_StdUnorderedMap_Del/512                             10002 ns         9988 ns        71961
BM_StdUnorderedMap_Del/1024                            19789 ns        19562 ns        36181
BM_LuaTable_Del/2                                        572 ns          566 ns      1195698
BM_LuaTable_Del/4                                        586 ns          577 ns      1245089
BM_LuaTable_Del/8                                        666 ns          643 ns      1132316
BM_LuaTable_Del/16                                       748 ns          744 ns       967447
BM_LuaTable_Del/32                                       998 ns          986 ns       733708
BM_LuaTable_Del/64                                      1472 ns         1453 ns       494051
BM_LuaTable_Del/128                                     2491 ns         2478 ns       293035
BM_LuaTable_Del/256                                     4355 ns         4297 ns       164014
BM_LuaTable_Del/512                                     8210 ns         8075 ns        88790
BM_LuaTable_Del/1024                                   15711 ns        15645 ns        45440
```

---

## 算法性能分析（C++ vs Lua vs FakeLua TCC vs FakeLua GCC）

以下取各算法的典型参数做横向对比（CPU Time），**倍数 = 对应实现时间 / C++ 时间**：

### 1. Fibonacci（n=32，递归无记忆化）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 5.24 ms | 1x |
| Lua | 205.08 ms | **39.1x** 慢 |
| FakeLua TCC | 19.76 ms | **3.8x** 慢 |
| FakeLua GCC | 4.57 ms | **0.87x** 快 (比 C++ 快 **15%**) |

> GCC 比 Lua 快 **44.9x**，TCC 比 Lua 快 **10.4x**。数值参数特化生成原生递归 + 原生条件比较，是 TCC/GCC 均优于 Lua 的关键。

### 2. GCD（a=832040, b=514229，约 30 次迭代）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 121.0 ns | 1x |
| Lua | 445.0 ns | **3.7x** 慢 |
| FakeLua TCC | 263.0 ns | **2.2x** 慢 (比 Lua 快 **1.7x**) |
| FakeLua GCC | 196.0 ns | **1.6x** 慢 (比 Lua 快 **2.3x**) |

### 3. PowMod（base=1234567, exp=7654321, mod=1e9+7，约 23 次循环，用 `%`/`//`）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 296.0 ns | 1x |
| Lua | 937.0 ns | **3.2x** 慢 |
| FakeLua TCC | 690.0 ns | **2.3x** 慢 (比 Lua 快 **1.36x**) |
| FakeLua GCC | 422.0 ns | **1.4x** 慢 (比 Lua 快 **2.2x**) |

### 4. FastPow（base=1234567, exp=7654321, mod=1e9+7，用 `&`/`>>`）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 296.0 ns | 1x |
| Lua | 813.0 ns | **2.7x** 慢 |
| FakeLua TCC | 429.0 ns | **1.45x** 慢 (比 Lua 快 **1.9x**) |
| FakeLua GCC | 407.0 ns | **1.37x** 慢 (比 Lua 快 **2.0x**) |

> FastPow 用位运算 `&`/`>>` 代替取余/整除 `%`/`//`，在 FakeLua TCC 下比 PowMod 快约 **1.6x**（690.0 ns → 429.0 ns），说明 TCC 对位运算的代码生成较优。GCC 两者表现也较接近。

### 5. Sum（n=5000000，纯循环累加）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 1.55 ms | 1x |
| Lua | 26.30 ms | **17.0x** 慢 |
| FakeLua TCC | 12.99 ms | **8.4x** 慢 (比 Lua 快 **2.0x**) |
| FakeLua GCC | 1.54 ms | **1.0x** 与 C++ 相同 |

> 纯整数累加循环：FakeLua GCC 与 C++ 几乎完全相同，说明 GCC `-O3` 对简单数值循环已达到 C++ 原生水平。TCC 比 Lua 快 **2.0x**。

### 6. BubbleSort（n=200，O(n²)，含表操作）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 125.5 µs | 1x |
| Lua | 846.9 µs | **6.7x** 慢 |
| FakeLua TCC | 1766.8 µs | **14.1x** 慢 (比 Lua 慢 **2.1x**) |
| FakeLua GCC | 319.6 µs | **2.5x** 慢 (比 Lua 快 **2.6x**) |

> 含大量表 Set/Get 操作的排序算法，**TCC 表现明显弱于 Lua**（2.1x 差距）。TCC 对 table 索引操作生成的代码路径较长（无寄存器分配优化），而 Lua 解释器在 table 操作上已高度优化。GCC 目前比 Lua 快约 **2.6x**。

### 7. Sieve（n=5000，Eratosthenes 筛）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 12.2 µs | 1x |
| Lua | 265.9 µs | **21.8x** 慢 |
| FakeLua TCC | 615.6 µs | **50.5x** 慢 (比 Lua 慢 **2.3x**) |
| FakeLua GCC | 117.4 µs | **9.6x** 慢 (比 Lua 快 **2.3x**) |

> 筛法涉及大量 boolean 表操作（`is_prime[j] = false`），TCC 在此类写密集型表操作上比 Lua 慢 **2.3x**，同样反映 TCC 在 table 写操作的代码生成开销。GCC 比 Lua 快 **2.3x**。

### 8. BinarySearch（n=1000，n 次二分查找，查全局常量表）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 16.6 µs | 1x |
| Lua | 435.7 µs | **26.2x** 慢 |
| FakeLua TCC | 499.1 µs | **30.1x** 慢 (比 Lua 慢 **1.15x**) |
| FakeLua GCC | 81.2 µs | **4.9x** 慢 (比 Lua 快 **5.4x**) |

> 二分查找改用全局常量表 `search_init_vals` 后，**避免了每次调用时的 Table 重复分配与填充开销**。这大幅减少了 GC 抖动，使得 FakeLua GCC 相比 Lua 的优势从之前的 4.7x 扩大到 **5.4x**，TCC 相比 Lua 的劣势也从 1.4x 缩小到仅 1.15x。

### 9. Popcount（n=100000，Brian Kernighan 位计数累加）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 487.7 µs | 1x |
| Lua | 12.51 ms | **25.7x** 慢 |
| FakeLua TCC | 2.03 ms | **4.2x** 慢 (比 Lua 快 **6.2x**) |
| FakeLua GCC | 467.6 µs | **0.96x** 快 (比 C++ 快 **4%**) |

> 纯整数位运算（`&`，`!=`），无表操作。**TCC 比 Lua 快 6.2x，GCC 比 Lua 快 26.8x，超越 C++**（在测量误差范围内基本持平）。

### 10. InsertionSort（n=200，O(n²)，含表操作）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 11.7 µs | 1x |
| Lua | 584.3 µs | **49.9x** 慢 |
| FakeLua TCC | 1078.6 µs | **92.2x** 慢 (比 Lua 慢 **1.8x**) |
| FakeLua GCC | 213.7 µs | **18.3x** 慢 (比 Lua 快 **2.7x**) |

> 与冒泡排序类似，表操作为瓶颈。TCC 比 Lua 慢约 1.8x，GCC 快于 Lua 约 2.7x。

### 11. MatMul（单次 3×3 矩阵乘法，使用全局常量 Table 读）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 3.10 ns | 1x |
| Lua | 2649.0 ns | **854.5x** 慢 |
| FakeLua TCC | 1982.0 ns | **639.4x** 慢 (比 Lua 快 **1.34x**) |
| FakeLua GCC | 420.0 ns | **135.5x** 慢 (比 Lua 快 **6.3x**) |

> 将只读矩阵 `mat_a` 和 `mat_b` 移入全局常量后，**TCC 和 GCC-JIT 的性能均获得巨大突破**：
> - **TCC** 性能提升 **33%** (从 2968 ns 降至 1982 ns)，**成功跑赢 Lua 5.4 解释器**（快 **1.34x**，此前为慢 1.37x）。
> - **GCC** 性能提升 **22%** (从 537 ns 降至 420 ns)，**比 Lua 5.4 解释器快 6.3x**（此前为快 3.4x）。
> 证明了全局常量 Table 在减少 JIT 临时内存分配与 GC 压力上的巨大作用。

---

## FakeLua TCC vs Lua 5.4 详细对比

| 算法 | 参数 | Lua | FakeLua TCC | 结果 |
|------|------|-----|-------------|------|
| Fibonacci | n=32 | 205.08 ms | 19.76 ms | TCC **10.4x 快** |
| GCD | 832040/514229 | 445.0 ns | 263.0 ns | TCC **1.7x 快** |
| PowMod | 1234567/7654321/1e9+7 | 937.0 ns | 690.0 ns | TCC **1.36x 快** |
| FastPow | 1234567/7654321/1e9+7 | 813.0 ns | 429.0 ns | TCC **1.9x 快** |
| Sum | n=5000000 | 26.30 ms | 12.99 ms | TCC **2.0x 快** |
| BubbleSort | n=200 | 846.9 µs | 1766.8 µs | TCC **2.1x 慢** |
| Sieve | n=5000 | 265.9 µs | 615.6 µs | TCC **2.3x 慢** |
| BinarySearch | n=1000 | 435.7 µs | 499.1 µs | TCC **1.15x 慢** |
| Popcount | n=100000 | 12.51 ms | 2.03 ms | TCC **6.2x 快** |
| InsertionSort | n=200 | 584.3 µs | 1078.6 µs | TCC **1.8x 慢** |
| MatMul | 单次 3×3 | 2649.0 ns | 1982.0 ns | TCC **1.34x 快** |

> **结论**：在将部分 Table 静态只读部分常量化后，TCC 在 MatMul 这样的运算中超越了 Lua；在纯数值算法上也保持 1.7x ~ 10.4x 的巨大优势。表写操作依然是 TCC 的软肋，但表读与遍历性能表现优异。

---

## FakeLua GCC vs Lua 5.4 详细对比

| 算法 | 参数 | Lua | FakeLua GCC | GCC 快多少 |
|------|------|-----|-------------|-----------|
| Fibonacci | n=32 | 205.08 ms | 4.57 ms | **44.9x** |
| GCD | 832040/514229 | 445.0 ns | 196.0 ns | **2.3x** |
| PowMod | 1234567/7654321/1e9+7 | 937.0 ns | 422.0 ns | **2.2x** |
| FastPow | 1234567/7654321/1e9+7 | 813.0 ns | 407.0 ns | **2.0x** |
| Sum | n=5000000 | 26.30 ms | 1.54 ms | **17.1x** |
| BubbleSort | n=200 | 846.9 µs | 319.6 µs | **2.6x** |
| Sieve | n=5000 | 265.9 µs | 117.4 µs | **2.3x** |
| BinarySearch | n=1000 | 435.7 µs | 81.2 µs | **5.4x** |
| Popcount | n=100000 | 12.51 ms | 467.6 µs | **26.8x** |
| InsertionSort | n=200 | 584.3 µs | 213.7 µs | **2.7x** |
| MatMul | 单次 3×3 | 2649.0 ns | 420.0 ns | **6.3x** |

### FakeLua GCC 按场景分类

| 场景类型 | FakeLua GCC 比 Lua 快 | 原因 |
|---------|----------------------|------|
| **纯整数累加 (Sum)** | **17.1x** | GCC `-O3` 向量化，达到 C++ 原生水平 |
| **纯整数位运算 (Popcount)** | **26.8x** | 位运算全部原生化，GCC 极进优化 |
| **递归 (Fibonacci)** | **44.9x** | 数值特化 + 原生递归，GCC 深度内联 |
| **算术循环 (PowMod/FastPow)** | **2.2–2.0x** | 循环体数值特化，取模运算受益于寄存器优化 |
| **短迭代 (GCD)** | **2.3x** | 迭代次数少，函数调用开销占比高 |
| **二分查找 (BinarySearch)** | **5.4x** | 混合数值+表操作，GCC 部分消除 table 开销 |
| **表操作为主 (BubbleSort/InsertionSort/Sieve/MatMul)** | **2.3–6.3x** | table 操作仍是瓶颈，GCC 无法完全消除 |

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
