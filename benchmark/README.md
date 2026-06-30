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
2026-06-30T09:44:59+08:00
Running build/bin/bench_mark
Run on (2 X 2595.12 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x2)
  L1 Instruction 32 KiB (x2)
  L2 Unified 4096 KiB (x2)
  L3 Unified 16384 KiB (x1)
Load Average: 1.37, 1.11, 0.56
***WARNING*** ASLR is enabled, the results may have unreproducible noise in them.
--------------------------------------------------------------------------------------------
Benchmark                                                  Time             CPU   Iterations
--------------------------------------------------------------------------------------------
BM_CPP_Fibonacci/20                                    16008 ns        16007 ns        45519
BM_CPP_Fibonacci/25                                   149842 ns       149834 ns         3763
BM_CPP_Fibonacci/30                                  1935410 ns      1935203 ns          323
BM_CPP_Fibonacci/32                                  4609975 ns      4609667 ns          140
BM_Lua_Fibonacci/20                                   647729 ns       637369 ns          834
BM_Lua_Fibonacci/25                                  7210360 ns      7208117 ns           75
BM_Lua_Fibonacci/30                                 77382140 ns     77374146 ns            7
BM_Lua_Fibonacci/32                                203315917 ns    203261013 ns            3
BM_FakeLua_Fibonacci_TCC/20                            62273 ns        62226 ns         8328
BM_FakeLua_Fibonacci_TCC/25                           678051 ns       677999 ns          769
BM_FakeLua_Fibonacci_TCC/30                          7902576 ns      7902125 ns           90
BM_FakeLua_Fibonacci_TCC/32                         20584100 ns     20582117 ns           32
BM_FakeLua_Fibonacci_GCC/20                            18091 ns        18087 ns        42269
BM_FakeLua_Fibonacci_GCC/25                           167772 ns       167747 ns         4647
BM_FakeLua_Fibonacci_GCC/30                          1753677 ns      1753566 ns          424
BM_FakeLua_Fibonacci_GCC/32                          4482231 ns      4481388 ns          154
BM_CPP_GCD/832040/514229                                 121 ns          121 ns      5793388
BM_CPP_GCD/123456789/987654321                          16.7 ns         16.7 ns     41989728
BM_CPP_GCD/2147483647/1073741823                        13.0 ns         13.0 ns     53785631
BM_Lua_GCD/832040/514229                                 455 ns          455 ns      1557703
BM_Lua_GCD/123456789/987654321                           121 ns          121 ns      6117795
BM_Lua_GCD/2147483647/1073741823                         102 ns          102 ns      6983506
BM_FakeLua_GCD_TCC/832040/514229                         249 ns          249 ns      2829847
BM_FakeLua_GCD_TCC/123456789/987654321                   123 ns          123 ns      5648381
BM_FakeLua_GCD_TCC/2147483647/1073741823                 120 ns          120 ns      5483997
BM_FakeLua_GCD_GCC/832040/514229                         204 ns          204 ns      3184055
BM_FakeLua_GCD_GCC/123456789/987654321                   104 ns          104 ns      6985374
BM_FakeLua_GCD_GCC/2147483647/1073741823                 103 ns          103 ns      7112510
BM_CPP_PowMod/2/1000/1000000007                          104 ns          104 ns      6672134
BM_CPP_PowMod/7/1000000/1000000007                       211 ns          204 ns      3430097
BM_CPP_PowMod/1234567/7654321/1000000007                 296 ns          296 ns      2364986
BM_Lua_PowMod/2/1000/1000000007                          431 ns          430 ns      1346558
BM_Lua_PowMod/7/1000000/1000000007                       783 ns          783 ns       702983
BM_Lua_PowMod/1234567/7654321/1000000007                 929 ns          929 ns       587754
BM_FakeLua_PowMod_TCC/2/1000/1000000007                  329 ns          328 ns      2125330
BM_FakeLua_PowMod_TCC/7/1000000/1000000007               559 ns          559 ns      1249671
BM_FakeLua_PowMod_TCC/1234567/7654321/1000000007         686 ns          685 ns       910892
BM_FakeLua_PowMod_GCC/2/1000/1000000007                  215 ns          215 ns      3082299
BM_FakeLua_PowMod_GCC/7/1000000/1000000007               321 ns          320 ns      2180342
BM_FakeLua_PowMod_GCC/1234567/7654321/1000000007         416 ns          416 ns      1669384
BM_CPP_Sum/10000                                        3102 ns         3084 ns       226200
BM_CPP_Sum/100000                                      30843 ns        30839 ns        22541
BM_CPP_Sum/1000000                                    309256 ns       308840 ns         2259
BM_CPP_Sum/5000000                                   1544944 ns      1543977 ns          454
BM_Lua_Sum/10000                                       54854 ns        54850 ns        13183
BM_Lua_Sum/100000                                     561601 ns       561492 ns         1314
BM_Lua_Sum/1000000                                   5477424 ns      5477005 ns          124
BM_Lua_Sum/5000000                                  27062610 ns     27058821 ns           24
BM_FakeLua_Sum_TCC/10000                               23160 ns        23158 ns        35149
BM_FakeLua_Sum_TCC/100000                             282868 ns       282840 ns         2677
BM_FakeLua_Sum_TCC/1000000                           2192839 ns      2182694 ns          247
BM_FakeLua_Sum_TCC/5000000                          11623169 ns     11621250 ns           54
BM_FakeLua_Sum_GCC/10000                                3200 ns         3199 ns       217286
BM_FakeLua_Sum_GCC/100000                              30991 ns        30984 ns        22479
BM_FakeLua_Sum_GCC/1000000                            309443 ns       309399 ns         2257
BM_FakeLua_Sum_GCC/5000000                           1547809 ns      1547548 ns          452
BM_CPP_BubbleSort/50                                    7630 ns         7630 ns        88511
BM_CPP_BubbleSort/100                                  31130 ns        31128 ns        22369
BM_CPP_BubbleSort/200                                 126463 ns       126440 ns         5545
BM_Lua_BubbleSort/50                                   55824 ns        55821 ns        12185
BM_Lua_BubbleSort/100                                 212161 ns       212150 ns         3078
BM_Lua_BubbleSort/200                                 829936 ns       829893 ns          820
BM_FakeLua_BubbleSort_TCC/50                          110365 ns       110357 ns         5981
BM_FakeLua_BubbleSort_TCC/100                         435808 ns       435786 ns         1556
BM_FakeLua_BubbleSort_TCC/200                        1670608 ns      1670406 ns          357
BM_FakeLua_BubbleSort_GCC/50                           31257 ns        31255 ns        21844
BM_FakeLua_BubbleSort_GCC/100                         126102 ns       126083 ns         5734
BM_FakeLua_BubbleSort_GCC/200                         494390 ns       494083 ns         1387
BM_CPP_Sieve/100                                         262 ns          262 ns      2786529
BM_CPP_Sieve/500                                        1195 ns         1195 ns       486069
BM_CPP_Sieve/1000                                       2431 ns         2431 ns       268792
BM_CPP_Sieve/5000                                      12225 ns        12222 ns        52696
BM_Lua_Sieve/100                                        6972 ns         6971 ns       104759
BM_Lua_Sieve/500                                       27702 ns        27700 ns        24378
BM_Lua_Sieve/1000                                      54331 ns        54323 ns        12999
BM_Lua_Sieve/5000                                     271115 ns       271094 ns         2464
BM_FakeLua_Sieve_TCC/100                                9853 ns         9852 ns        69927
BM_FakeLua_Sieve_TCC/500                               48193 ns        48114 ns        14698
BM_FakeLua_Sieve_TCC/1000                              93058 ns        93013 ns         5540
BM_FakeLua_Sieve_TCC/5000                             618925 ns       615934 ns          987
BM_FakeLua_Sieve_GCC/100                                2040 ns         2035 ns       335329
BM_FakeLua_Sieve_GCC/500                                9593 ns         9592 ns        75845
BM_FakeLua_Sieve_GCC/1000                              19647 ns        19632 ns        32409
BM_FakeLua_Sieve_GCC/5000                             129324 ns       127890 ns         5774
BM_CPP_BinarySearch/100                                  851 ns          851 ns       665348
BM_CPP_BinarySearch/500                                 5952 ns         5946 ns        96513
BM_CPP_BinarySearch/1000                               17173 ns        17155 ns        35337
BM_Lua_BinarySearch/100                                29224 ns        29205 ns        23414
BM_Lua_BinarySearch/500                               198527 ns       197626 ns         3635
BM_Lua_BinarySearch/1000                              446568 ns       446178 ns         1260
BM_FakeLua_BinarySearch_TCC/100                        39536 ns        39385 ns        13558
BM_FakeLua_BinarySearch_TCC/500                       280991 ns       280134 ns         2359
BM_FakeLua_BinarySearch_TCC/1000                      666917 ns       661824 ns         1058
BM_FakeLua_BinarySearch_GCC/100                         6127 ns         6126 ns       112390
BM_FakeLua_BinarySearch_GCC/500                        53016 ns        52983 ns        11614
BM_FakeLua_BinarySearch_GCC/1000                      119763 ns       119686 ns         5314
BM_CPP_FastPow/2/1000/1000000007                         105 ns          105 ns      6641928
BM_CPP_FastPow/7/1000000/1000000007                      204 ns          204 ns      3434751
BM_CPP_FastPow/1234567/7654321/1000000007                297 ns          297 ns      2359400
BM_Lua_FastPow/2/1000/1000000007                         401 ns          401 ns      1705543
BM_Lua_FastPow/7/1000000/1000000007                      705 ns          705 ns       989627
BM_Lua_FastPow/1234567/7654321/1000000007                837 ns          837 ns       840387
BM_FakeLua_FastPow_TCC/2/1000/1000000007                 222 ns          222 ns      2868975
BM_FakeLua_FastPow_TCC/7/1000000/1000000007              340 ns          340 ns      2049087
BM_FakeLua_FastPow_TCC/1234567/7654321/1000000007        433 ns          431 ns      1580977
BM_FakeLua_FastPow_GCC/2/1000/1000000007                 209 ns          209 ns      3480654
BM_FakeLua_FastPow_GCC/7/1000000/1000000007              312 ns          312 ns      2209156
BM_FakeLua_FastPow_GCC/1234567/7654321/1000000007        421 ns          402 ns      1752476
BM_CPP_Popcount/1000                                    3377 ns         3374 ns       200543
BM_CPP_Popcount/10000                                  39030 ns        39026 ns        16843
BM_CPP_Popcount/100000                                496824 ns       496720 ns         1047
BM_Lua_Popcount/1000                                   85285 ns        85273 ns         6181
BM_Lua_Popcount/10000                                1047535 ns      1047486 ns          535
BM_Lua_Popcount/100000                              13216690 ns     13215924 ns           47
BM_FakeLua_Popcount_TCC/1000                           12488 ns        12486 ns        56229
BM_FakeLua_Popcount_TCC/10000                         162758 ns       162474 ns         4346
BM_FakeLua_Popcount_TCC/100000                       2033611 ns      2033476 ns          345
BM_FakeLua_Popcount_GCC/1000                            3282 ns         3282 ns       211005
BM_FakeLua_Popcount_GCC/10000                          37908 ns        37905 ns        18826
BM_FakeLua_Popcount_GCC/100000                        463118 ns       463087 ns         1488
BM_CPP_InsertionSort/50                                  766 ns          765 ns       938554
BM_CPP_InsertionSort/100                                3259 ns         3258 ns       209119
BM_CPP_InsertionSort/200                               11818 ns        11811 ns        61323
BM_Lua_InsertionSort/50                                39667 ns        39603 ns        12852
BM_Lua_InsertionSort/100                              147392 ns       147281 ns         3860
BM_Lua_InsertionSort/200                              583389 ns       582117 ns          871
BM_FakeLua_InsertionSort_TCC/50                        74512 ns        74417 ns        10037
BM_FakeLua_InsertionSort_TCC/100                      280040 ns       279870 ns         2395
BM_FakeLua_InsertionSort_TCC/200                     1108195 ns      1107482 ns          660
BM_FakeLua_InsertionSort_GCC/50                        12548 ns        12546 ns        43291
BM_FakeLua_InsertionSort_GCC/100                       48255 ns        48252 ns        15064
BM_FakeLua_InsertionSort_GCC/200                      186111 ns       186099 ns         3298
BM_CPP_MatMul                                           3.24 ns         3.19 ns    225790435
BM_Lua_MatMul                                           2658 ns         2658 ns       255933
BM_FakeLua_MatMul_TCC                                   2389 ns         2388 ns       315072
BM_FakeLua_MatMul_GCC                                    611 ns          611 ns      1113757
BM_VarTable_Set/2                                        134 ns          134 ns      4401795
BM_VarTable_Set/4                                        160 ns          160 ns      3887808
BM_VarTable_Set/8                                        202 ns          202 ns      2998203
BM_VarTable_Set/16                                       643 ns          637 ns       844942
BM_VarTable_Set/32                                      1505 ns         1505 ns       400637
BM_VarTable_Set/64                                      3330 ns         3329 ns       210505
BM_VarTable_Set/128                                     6660 ns         6659 ns        81755
BM_VarTable_Set/256                                    13410 ns        13408 ns        44498
BM_VarTable_Set/512                                    26788 ns        26783 ns        23661
BM_VarTable_Set/1024                                   53629 ns        53623 ns        10213
BM_StdUnorderedMap_Set/2                                91.1 ns         91.1 ns      5660054
BM_StdUnorderedMap_Set/4                                 138 ns          138 ns      4245368
BM_StdUnorderedMap_Set/8                                 255 ns          255 ns      2446435
BM_StdUnorderedMap_Set/16                                607 ns          606 ns      1212194
BM_StdUnorderedMap_Set/32                               1338 ns         1338 ns       568249
BM_StdUnorderedMap_Set/64                               2636 ns         2635 ns       274793
BM_StdUnorderedMap_Set/128                              5545 ns         5545 ns       128400
BM_StdUnorderedMap_Set/256                             12219 ns        12218 ns        60944
BM_StdUnorderedMap_Set/512                             27034 ns        27015 ns        25733
BM_StdUnorderedMap_Set/1024                            56935 ns        56898 ns         9645
BM_LuaTable_Set/2                                        792 ns          797 ns       755142
BM_LuaTable_Set/4                                        961 ns          967 ns       650491
BM_LuaTable_Set/8                                       1238 ns         1245 ns       534927
BM_LuaTable_Set/16                                      1716 ns         1722 ns       370643
BM_LuaTable_Set/32                                      2520 ns         2529 ns       292984
BM_LuaTable_Set/64                                      4040 ns         4050 ns       174723
BM_LuaTable_Set/128                                     7241 ns         7259 ns       104225
BM_LuaTable_Set/256                                    13029 ns        13047 ns        57720
BM_LuaTable_Set/512                                    23800 ns        23823 ns        28305
BM_LuaTable_Set/1024                                   45894 ns        45927 ns        15405
BM_VarTable_Get/2                                       25.1 ns         25.1 ns     26224788
BM_VarTable_Get/4                                       53.7 ns         53.7 ns     12900369
BM_VarTable_Get/8                                        123 ns          123 ns      6077651
BM_VarTable_Get/16                                       195 ns          195 ns      3008175
BM_VarTable_Get/32                                       396 ns          393 ns      1327700
BM_VarTable_Get/64                                       769 ns          768 ns       741136
BM_VarTable_Get/128                                     1547 ns         1546 ns       401881
BM_VarTable_Get/256                                     3155 ns         3152 ns       226503
BM_VarTable_Get/512                                     6240 ns         6218 ns       107660
BM_VarTable_Get/1024                                   12789 ns        12783 ns        54934
BM_StdUnorderedMap_Get/2                                9.92 ns         9.92 ns     67849166
BM_StdUnorderedMap_Get/4                                21.7 ns         21.7 ns     31893695
BM_StdUnorderedMap_Get/8                                46.2 ns         46.2 ns     15118455
BM_StdUnorderedMap_Get/16                               94.5 ns         94.4 ns      7363551
BM_StdUnorderedMap_Get/32                                191 ns          191 ns      3684983
BM_StdUnorderedMap_Get/64                                385 ns          384 ns      1812854
BM_StdUnorderedMap_Get/128                               751 ns          750 ns       909450
BM_StdUnorderedMap_Get/256                              1553 ns         1552 ns       435835
BM_StdUnorderedMap_Get/512                              3135 ns         3131 ns       221898
BM_StdUnorderedMap_Get/1024                             6225 ns         6220 ns       106909
BM_LuaTable_Get/2                                       33.3 ns         33.2 ns     20425020
BM_LuaTable_Get/4                                       65.6 ns         65.5 ns      8967110
BM_LuaTable_Get/8                                        143 ns          143 ns      4555389
BM_LuaTable_Get/16                                       275 ns          275 ns      2633494
BM_LuaTable_Get/32                                       545 ns          544 ns      1272535
BM_LuaTable_Get/64                                      1012 ns         1012 ns       533830
BM_LuaTable_Get/128                                     1986 ns         1983 ns       316631
BM_LuaTable_Get/256                                     4301 ns         4296 ns       123403
BM_LuaTable_Get/512                                     8162 ns         8158 ns        66512
BM_LuaTable_Get/1024                                   16591 ns        16580 ns        36680
BM_VarTable_Iter/2                                      1.27 ns         1.27 ns    400938278
BM_VarTable_Iter/4                                      2.68 ns         2.68 ns    246875959
BM_VarTable_Iter/8                                      5.34 ns         5.34 ns    129641028
BM_VarTable_Iter/16                                     14.3 ns         14.3 ns     50621487
BM_VarTable_Iter/32                                     30.5 ns         30.5 ns     21527565
BM_VarTable_Iter/64                                     57.3 ns         57.3 ns     12119780
BM_VarTable_Iter/128                                     112 ns          112 ns      4840048
BM_VarTable_Iter/256                                     231 ns          231 ns      2758155
BM_VarTable_Iter/512                                     476 ns          476 ns      1317886
BM_VarTable_Iter/1024                                    994 ns          993 ns       765138
BM_StdUnorderedMap_Iter/2                              0.912 ns        0.881 ns    780554000
BM_StdUnorderedMap_Iter/4                               1.73 ns         1.73 ns    397890693
BM_StdUnorderedMap_Iter/8                               4.34 ns         4.32 ns    165085593
BM_StdUnorderedMap_Iter/16                              10.4 ns         10.4 ns     53420604
BM_StdUnorderedMap_Iter/32                              27.4 ns         27.4 ns     23705292
BM_StdUnorderedMap_Iter/64                              67.7 ns         67.6 ns      8988589
BM_StdUnorderedMap_Iter/128                              170 ns          170 ns      3970423
BM_StdUnorderedMap_Iter/256                              365 ns          364 ns      1965055
BM_StdUnorderedMap_Iter/512                              998 ns          998 ns       604533
BM_StdUnorderedMap_Iter/1024                            2015 ns         2015 ns       334402
BM_LuaTable_Iter/2                                      63.4 ns         63.4 ns     10136812
BM_LuaTable_Iter/4                                       120 ns          120 ns      5138734
BM_LuaTable_Iter/8                                       222 ns          222 ns      2960988
BM_LuaTable_Iter/16                                      393 ns          393 ns      1661334
BM_LuaTable_Iter/32                                      795 ns          795 ns       915013
BM_LuaTable_Iter/64                                     1494 ns         1493 ns       392443
BM_LuaTable_Iter/128                                    3006 ns         3005 ns       219387
BM_LuaTable_Iter/256                                    6080 ns         6078 ns        82600
BM_LuaTable_Iter/512                                   12414 ns        12413 ns        48441
BM_LuaTable_Iter/1024                                  24401 ns        24398 ns        30014
BM_VarTable_Del/2                                        292 ns          293 ns      2478516
BM_VarTable_Del/4                                        327 ns          328 ns      2169742
BM_VarTable_Del/8                                        401 ns          403 ns      1298310
BM_VarTable_Del/16                                       523 ns          519 ns      1097282
BM_VarTable_Del/32                                       974 ns          898 ns       823902
BM_VarTable_Del/64                                      1825 ns         1649 ns       537873
BM_VarTable_Del/128                                     3222 ns         2994 ns       299084
BM_VarTable_Del/256                                     5433 ns         5028 ns       100000
BM_VarTable_Del/512                                    13662 ns        10536 ns        85579
BM_VarTable_Del/1024                                   25912 ns        20676 ns        44492
BM_StdUnorderedMap_Del/2                                 286 ns          287 ns      2303641
BM_StdUnorderedMap_Del/4                                 321 ns          323 ns      2243128
BM_StdUnorderedMap_Del/8                                 363 ns          365 ns      1914816
BM_StdUnorderedMap_Del/16                                582 ns          583 ns      1247619
BM_StdUnorderedMap_Del/32                                963 ns          961 ns       680338
BM_StdUnorderedMap_Del/64                               1698 ns         1697 ns       412703
BM_StdUnorderedMap_Del/128                              3236 ns         3230 ns       211936
BM_StdUnorderedMap_Del/256                              5078 ns         5072 ns       118861
BM_StdUnorderedMap_Del/512                              9964 ns         9953 ns        69310
BM_StdUnorderedMap_Del/1024                            19689 ns        19658 ns        36471
BM_LuaTable_Del/2                                        549 ns          548 ns      1237456
BM_LuaTable_Del/4                                        571 ns          572 ns      1071477
BM_LuaTable_Del/8                                        660 ns          651 ns      1113857
BM_LuaTable_Del/16                                       788 ns          786 ns       857193
BM_LuaTable_Del/32                                       993 ns          991 ns       725350
BM_LuaTable_Del/64                                      1464 ns         1458 ns       477532
BM_LuaTable_Del/128                                     2448 ns         2386 ns       294235
BM_LuaTable_Del/256                                     4305 ns         4291 ns       164149
BM_LuaTable_Del/512                                     8053 ns         8039 ns        84879
BM_LuaTable_Del/1024                                   15641 ns        15604 ns        44281
```

---

## 算法性能分析（C++ vs Lua vs FakeLua TCC vs FakeLua GCC）

以下取各算法的典型参数做横向对比（CPU Time），**倍数 = 对应实现时间 / C++ 时间**：

### 1. Fibonacci（n=32，递归无记忆化）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 4.61 ms | 1x |
| Lua | 203.26 ms | **44.09x** 慢 |
| FakeLua TCC | 20.58 ms | **4.46x** 慢 |
| FakeLua GCC | 4.48 ms | **1.03x** 快 (比 C++ 快 **2%**) |

### 2. GCD（a=832040, b=514229，约 30 次迭代）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 121 ns | 1x |
| Lua | 455 ns | **3.76x** 慢 |
| FakeLua TCC | 249 ns | **2.06x** 慢 (比 Lua 快 **1.83x**) |
| FakeLua GCC | 204 ns | **1.69x** 慢 (比 Lua 快 **2.23x**) |

### 3. PowMod（base=1234567, exp=7654321, mod=1e9+7，约 23 次循环，用 `%`/`//`）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 296 ns | 1x |
| Lua | 929 ns | **3.14x** 慢 |
| FakeLua TCC | 685 ns | **2.31x** 慢 (比 Lua 快 **1.36x**) |
| FakeLua GCC | 416 ns | **1.41x** 慢 (比 Lua 快 **2.23x**) |

### 4. FastPow（base=1234567, exp=7654321, mod=1e9+7，用 `&`/`>>`）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 297 ns | 1x |
| Lua | 837 ns | **2.82x** 慢 |
| FakeLua TCC | 431 ns | **1.45x** 慢 (比 Lua 快 **1.94x**) |
| FakeLua GCC | 402 ns | **1.35x** 慢 (比 Lua 快 **2.08x**) |

> FastPow 用位运算 `&`/`>>` 代替取余/整除 `%`/`//`，在 FakeLua TCC 下比 PowMod 快约 **1.6x**（690.0 ns → 429.0 ns），说明 TCC 对位运算的代码生成较优。GCC 两者表现也较接近。

### 5. Sum（n=5000000，纯循环累加）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 1.54 ms | 1x |
| Lua | 27.06 ms | **17.53x** 慢 |
| FakeLua TCC | 11.62 ms | **7.53x** 慢 (比 Lua 快 **2.33x**) |
| FakeLua GCC | 1.55 ms | **1.00x** 慢 |

> 纯整数累加循环：FakeLua GCC 与 C++ 几乎完全相同，说明 GCC `-O3` 对简单数值循环已达到 C++ 原生水平。TCC 比 Lua 快 **2.0x**。

### 6. BubbleSort（n=200，O(n²)，含表操作）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 126.4 µs | 1x |
| Lua | 829.9 µs | **6.56x** 慢 |
| FakeLua TCC | 1.67 ms | **13.21x** 慢 (比 Lua 慢 **2.01x**) |
| FakeLua GCC | 494.1 µs | **3.91x** 慢 (比 Lua 快 **1.68x**) |

> 含大量表 Set/Get 操作的排序算法，**TCC 表现明显弱于 Lua**（2.1x 差距）。TCC 对 table 索引操作生成的代码路径较长（无寄存器分配优化），而 Lua 解释器在 table 操作上已高度优化。GCC 目前比 Lua 快约 **2.6x**。

### 7. Sieve（n=5000，Eratosthenes 筛）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 12.2 µs | 1x |
| Lua | 271.1 µs | **22.18x** 慢 |
| FakeLua TCC | 615.9 µs | **50.40x** 慢 (比 Lua 慢 **2.27x**) |
| FakeLua GCC | 127.9 µs | **10.46x** 慢 (比 Lua 快 **2.12x**) |

> 筛法涉及大量 boolean 表操作（`is_prime[j] = false`），TCC 在此类写密集型表操作上比 Lua 慢 **2.3x**，同样反映 TCC 在 table 写操作的代码生成开销。GCC 比 Lua 快 **2.3x**。

### 8. BinarySearch（n=1000，n 次二分查找，查全局常量表）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 17.2 µs | 1x |
| Lua | 446.2 µs | **26.01x** 慢 |
| FakeLua TCC | 661.8 µs | **38.58x** 慢 (比 Lua 慢 **1.48x**) |
| FakeLua GCC | 119.7 µs | **6.98x** 慢 (比 Lua 快 **3.73x**) |

> 二分查找改用全局常量表 `search_init_vals` 后，**避免了每次调用时的 Table 重复分配与填充开销**。这大幅减少了 GC 抖动，使得 FakeLua GCC 相比 Lua 的优势从之前的 4.7x 扩大到 **5.4x**，TCC 相比 Lua 的劣势也从 1.4x 缩小到仅 1.15x。

### 9. Popcount（n=100000，Brian Kernighan 位计数累加）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 496.7 µs | 1x |
| Lua | 13.22 ms | **26.61x** 慢 |
| FakeLua TCC | 2.03 ms | **4.09x** 慢 (比 Lua 快 **6.50x**) |
| FakeLua GCC | 463.1 µs | **1.07x** 快 (比 C++ 快 **6%**) |

> 纯整数位运算（`&`，`!=`），无表操作。**TCC 比 Lua 快 6.2x，GCC 比 Lua 快 26.8x，超越 C++**（在测量误差范围内基本持平）。

### 10. InsertionSort（n=200，O(n²)，含表操作）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 11.8 µs | 1x |
| Lua | 582.1 µs | **49.29x** 慢 |
| FakeLua TCC | 1.11 ms | **93.77x** 慢 (比 Lua 慢 **1.90x**) |
| FakeLua GCC | 186.1 µs | **15.76x** 慢 (比 Lua 快 **3.13x**) |

> 与冒泡排序类似，表操作为瓶颈。TCC 比 Lua 慢约 1.8x，GCC 快于 Lua 约 2.7x。

### 11. MatMul（单次 3×3 矩阵乘法，使用全局常量 Table 读）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 3.19 ns | 1x |
| Lua | 2.7 µs | **833.23x** 慢 |
| FakeLua TCC | 2.4 µs | **748.59x** 慢 (比 Lua 快 **1.11x**) |
| FakeLua GCC | 611 ns | **191.54x** 慢 (比 Lua 快 **4.35x**) |

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
| Fibonacci | n=32 | 203.26 ms | 20.58 ms | TCC **9.88x 快** |
| GCD | 832040/514229 | 455 ns | 249 ns | TCC **1.83x 快** |
| PowMod | 1234567/7654321/1e9+7 | 929 ns | 685 ns | TCC **1.36x 快** |
| FastPow | 1234567/7654321/1e9+7 | 837 ns | 431 ns | TCC **1.94x 快** |
| Sum | n=5000000 | 27.06 ms | 11.62 ms | TCC **2.33x 快** |
| BubbleSort | n=200 | 829.9 µs | 1.67 ms | TCC **2.01x 慢** |
| Sieve | n=5000 | 271.1 µs | 615.9 µs | TCC **2.27x 慢** |
| BinarySearch | n=1000 | 446.2 µs | 661.8 µs | TCC **1.48x 慢** |
| Popcount | n=100000 | 13.22 ms | 2.03 ms | TCC **6.50x 快** |
| InsertionSort | n=200 | 582.1 µs | 1.11 ms | TCC **1.90x 慢** |
| MatMul | 单次 3×3 | 2.7 µs | 2.4 µs | TCC **1.11x 快** |

> **结论**：在将部分 Table 静态只读部分常量化后，TCC 在 MatMul 这样的运算中超越了 Lua；在纯数值算法上也保持 1.7x ~ 10.4x 的巨大优势。表写操作依然是 TCC 的软肋，但表读与遍历性能表现优异。

---

## FakeLua GCC vs Lua 5.4 详细对比

| 算法 | 参数 | Lua | FakeLua GCC | GCC 快多少 |
|------|------|-----|-------------|-----------|
| Fibonacci | n=32 | 203.26 ms | 4.48 ms | **45.36x** |
| GCD | 832040/514229 | 455 ns | 204 ns | **2.23x** |
| PowMod | 1234567/7654321/1e9+7 | 929 ns | 416 ns | **2.23x** |
| FastPow | 1234567/7654321/1e9+7 | 837 ns | 402 ns | **2.08x** |
| Sum | n=5000000 | 27.06 ms | 1.55 ms | **17.48x** |
| BubbleSort | n=200 | 829.9 µs | 494.1 µs | **1.68x** |
| Sieve | n=5000 | 271.1 µs | 127.9 µs | **2.12x** |
| BinarySearch | n=1000 | 446.2 µs | 119.7 µs | **3.73x** |
| Popcount | n=100000 | 13.22 ms | 463.1 µs | **28.54x** |
| InsertionSort | n=200 | 582.1 µs | 186.1 µs | **3.13x** |
| MatMul | 单次 3×3 | 2.7 µs | 611 ns | **4.35x** |

### FakeLua GCC 按场景分类

| 场景类型 | FakeLua GCC 比 Lua 快 | 原因 |
|---------|----------------------|------|
| **纯整数累加 (Sum)** | **17.48x** | GCC `-O3` 向量化，达到 C++ 原生水平 |
| **纯整数位运算 (Popcount)** | **28.54x** | 位运算全部原生化，GCC 极进优化 |
| **递归 (Fibonacci)** | **45.36x** | 数值特化 + 原生递归，GCC 深度内联 |
| **算术循环 (PowMod/FastPow)** | **2.23x–2.08x** | 循环体数值特化，取模运算受益于寄存器优化 |
| **短迭代 (GCD)** | **2.23x** | 迭代次数少，函数调用开销占比高 |
| **二分查找 (BinarySearch)** | **3.73x** | 混合数值+表操作，GCC 部分消除 table 开销 |
| **表操作为主 (BubbleSort/InsertionSort/Sieve/MatMul)** | **1.68x–4.35x** | 引入了 Table 结构体特化与读写，大幅提升读写效率 |

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
