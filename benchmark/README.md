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
2026-06-26T14:35:13+08:00
Running build/bin/bench_mark
Run on (2 X 2595.12 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x2)
  L1 Instruction 32 KiB (x2)
  L2 Unified 4096 KiB (x2)
  L3 Unified 16384 KiB (x1)
Load Average: 1.52, 1.30, 1.35
***WARNING*** ASLR is enabled, the results may have unreproducible noise in them.
--------------------------------------------------------------------------------------------
Benchmark                                                  Time             CPU   Iterations
--------------------------------------------------------------------------------------------
BM_CPP_Fibonacci/20                                    17909 ns        17060 ns        44688
BM_CPP_Fibonacci/25                                   152987 ns       152861 ns         4657
BM_CPP_Fibonacci/30                                  1890723 ns      1889158 ns          313
BM_CPP_Fibonacci/32                                  5060545 ns      5052482 ns          100
BM_Lua_Fibonacci/20                                   639096 ns       638485 ns          875
BM_Lua_Fibonacci/25                                  8532480 ns      7923035 ns           92
BM_Lua_Fibonacci/30                                 78075417 ns     78069982 ns            8
BM_Lua_Fibonacci/32                                435726130 ns    275639480 ns            2
BM_FakeLua_Fibonacci_TCC/20                            65120 ns        65101 ns         9676
BM_FakeLua_Fibonacci_TCC/25                           674882 ns       667263 ns          880
BM_FakeLua_Fibonacci_TCC/30                          7583334 ns      7582937 ns           91
BM_FakeLua_Fibonacci_TCC/32                         19421473 ns     19406171 ns           33
BM_FakeLua_Fibonacci_GCC/20                            19440 ns        19011 ns        40712
BM_FakeLua_Fibonacci_GCC/25                           156522 ns       156435 ns         4542
BM_FakeLua_Fibonacci_GCC/30                          1749477 ns      1748232 ns          323
BM_FakeLua_Fibonacci_GCC/32                          4925909 ns      4856626 ns          157
BM_CPP_GCD/832040/514229                                 122 ns          121 ns      5789012
BM_CPP_GCD/123456789/987654321                          16.7 ns         16.7 ns     42048782
BM_CPP_GCD/2147483647/1073741823                        13.2 ns         13.0 ns     54005152
BM_Lua_GCD/832040/514229                                 477 ns          472 ns      1576515
BM_Lua_GCD/123456789/987654321                           117 ns          116 ns      5897202
BM_Lua_GCD/2147483647/1073741823                         103 ns          103 ns      5114747
BM_FakeLua_GCD_TCC/832040/514229                         227 ns          227 ns      3165066
BM_FakeLua_GCD_TCC/123456789/987654321                   123 ns          123 ns      5784468
BM_FakeLua_GCD_TCC/2147483647/1073741823                 119 ns          119 ns      5917274
BM_FakeLua_GCD_GCC/832040/514229                         214 ns          212 ns      3535705
BM_FakeLua_GCD_GCC/123456789/987654321                   113 ns          111 ns      6696259
BM_FakeLua_GCD_GCC/2147483647/1073741823                99.3 ns         99.3 ns      6960256
BM_CPP_PowMod/2/1000/1000000007                          106 ns          104 ns      6667690
BM_CPP_PowMod/7/1000000/1000000007                       204 ns          204 ns      3422633
BM_CPP_PowMod/1234567/7654321/1000000007                 302 ns          296 ns      2358815
BM_Lua_PowMod/2/1000/1000000007                          458 ns          453 ns      1617927
BM_Lua_PowMod/7/1000000/1000000007                       774 ns          773 ns       827540
BM_Lua_PowMod/1234567/7654321/1000000007                 980 ns          971 ns       731343
BM_FakeLua_PowMod_TCC/2/1000/1000000007                  332 ns          331 ns      2158086
BM_FakeLua_PowMod_TCC/7/1000000/1000000007               555 ns          551 ns      1109578
BM_FakeLua_PowMod_TCC/1234567/7654321/1000000007         709 ns          702 ns       998090
BM_FakeLua_PowMod_GCC/2/1000/1000000007                  211 ns          211 ns      3343135
BM_FakeLua_PowMod_GCC/7/1000000/1000000007               317 ns          317 ns      2053110
BM_FakeLua_PowMod_GCC/1234567/7654321/1000000007         431 ns          429 ns      1699367
BM_CPP_Sum/10000                                        3156 ns         3092 ns       227306
BM_CPP_Sum/100000                                      30875 ns        30873 ns        22707
BM_CPP_Sum/1000000                                    308767 ns       308383 ns         2261
BM_CPP_Sum/5000000                                   1578915 ns      1546738 ns          454
BM_Lua_Sum/10000                                       52626 ns        52567 ns        12359
BM_Lua_Sum/100000                                     525488 ns       525161 ns         1215
BM_Lua_Sum/1000000                                   5827724 ns      5764602 ns          123
BM_Lua_Sum/5000000                                  28798308 ns     28345122 ns           27
BM_FakeLua_Sum_TCC/10000                               24864 ns        24836 ns        35892
BM_FakeLua_Sum_TCC/100000                             266611 ns       266418 ns         2797
BM_FakeLua_Sum_TCC/1000000                           2538739 ns      2452636 ns          257
BM_FakeLua_Sum_TCC/5000000                          14391572 ns     14390320 ns           51
BM_FakeLua_Sum_GCC/10000                                3194 ns         3194 ns       216553
BM_FakeLua_Sum_GCC/100000                              31423 ns        31111 ns        22566
BM_FakeLua_Sum_GCC/1000000                            317199 ns       309506 ns         2268
BM_FakeLua_Sum_GCC/5000000                           1549194 ns      1546296 ns          454
BM_CPP_BubbleSort/50                                    7905 ns         7431 ns        90553
BM_CPP_BubbleSort/100                                  32006 ns        31176 ns        22615
BM_CPP_BubbleSort/200                                 126710 ns       126628 ns         5525
BM_Lua_BubbleSort/50                                   52067 ns        52032 ns        12727
BM_Lua_BubbleSort/100                                 222281 ns       217175 ns         3497
BM_Lua_BubbleSort/200                                 832954 ns       832432 ns          742
BM_FakeLua_BubbleSort_TCC/50                          104444 ns       104433 ns         5837
BM_FakeLua_BubbleSort_TCC/100                         465442 ns       451837 ns         1661
BM_FakeLua_BubbleSort_TCC/200                        1846641 ns      1779921 ns          421
BM_FakeLua_BubbleSort_GCC/50                           19579 ns        19578 ns        35967
BM_FakeLua_BubbleSort_GCC/100                          87086 ns        84549 ns         8593
BM_FakeLua_BubbleSort_GCC/200                         334285 ns       330231 ns         2241
BM_CPP_Sieve/100                                         253 ns          253 ns      2754330
BM_CPP_Sieve/500                                        1189 ns         1189 ns       427045
BM_CPP_Sieve/1000                                       2403 ns         2403 ns       291808
BM_CPP_Sieve/5000                                      12416 ns        12392 ns        56873
BM_Lua_Sieve/100                                        7394 ns         7046 ns        71528
BM_Lua_Sieve/500                                       29667 ns        29186 ns        25844
BM_Lua_Sieve/1000                                      52104 ns        52067 ns        11903
BM_Lua_Sieve/5000                                     272348 ns       267948 ns         2491
BM_FakeLua_Sieve_TCC/100                               10272 ns        10123 ns        72663
BM_FakeLua_Sieve_TCC/500                               45145 ns        45143 ns        13443
BM_FakeLua_Sieve_TCC/1000                             102127 ns       101075 ns         7236
BM_FakeLua_Sieve_TCC/5000                             568261 ns       568161 ns         1168
BM_FakeLua_Sieve_GCC/100                                1908 ns         1905 ns       290116
BM_FakeLua_Sieve_GCC/500                                9763 ns         9480 ns        78381
BM_FakeLua_Sieve_GCC/1000                              18123 ns        18110 ns        38840
BM_FakeLua_Sieve_GCC/5000                             121292 ns       120978 ns         5964
BM_CPP_BinarySearch/100                                  872 ns          857 ns       663449
BM_CPP_BinarySearch/500                                 5869 ns         5869 ns       115719
BM_CPP_BinarySearch/1000                               17670 ns        17644 ns        35802
BM_Lua_BinarySearch/100                                29666 ns        29612 ns        24707
BM_Lua_BinarySearch/500                               179124 ns       179020 ns         3814
BM_Lua_BinarySearch/1000                              411185 ns       403307 ns         1324
BM_FakeLua_BinarySearch_TCC/100                        41437 ns        40725 ns        18093
BM_FakeLua_BinarySearch_TCC/500                       264616 ns       259392 ns         2807
BM_FakeLua_BinarySearch_TCC/1000                      558212 ns       557492 ns         1119
BM_FakeLua_BinarySearch_GCC/100                         5269 ns         5018 ns       138737
BM_FakeLua_BinarySearch_GCC/500                        43580 ns        43572 ns        17400
BM_FakeLua_BinarySearch_GCC/1000                       91376 ns        91300 ns         7693
BM_CPP_FastPow/2/1000/1000000007                         107 ns          104 ns      6656928
BM_CPP_FastPow/7/1000000/1000000007                      204 ns          204 ns      3437529
BM_CPP_FastPow/1234567/7654321/1000000007                296 ns          296 ns      2365368
BM_Lua_FastPow/2/1000/1000000007                         427 ns          416 ns      1324348
BM_Lua_FastPow/7/1000000/1000000007                      680 ns          680 ns       995853
BM_Lua_FastPow/1234567/7654321/1000000007                889 ns          846 ns       669326
BM_FakeLua_FastPow_TCC/2/1000/1000000007                 239 ns          235 ns      3124921
BM_FakeLua_FastPow_TCC/7/1000000/1000000007              338 ns          337 ns      2117889
BM_FakeLua_FastPow_TCC/1234567/7654321/1000000007        443 ns          430 ns      1368188
BM_FakeLua_FastPow_GCC/2/1000/1000000007                 205 ns          205 ns      3450087
BM_FakeLua_FastPow_GCC/7/1000000/1000000007              305 ns          305 ns      2120430
BM_FakeLua_FastPow_GCC/1234567/7654321/1000000007        401 ns          401 ns      1646608
BM_CPP_Popcount/1000                                    3619 ns         3585 ns       205564
BM_CPP_Popcount/10000                                  41887 ns        41475 ns        17699
BM_CPP_Popcount/100000                                488892 ns       488855 ns         1351
BM_Lua_Popcount/1000                                   91408 ns        87648 ns         5893
BM_Lua_Popcount/10000                                1044250 ns      1043922 ns          674
BM_Lua_Popcount/100000                              12393274 ns     12392509 ns           54
BM_FakeLua_Popcount_TCC/1000                           11746 ns        11595 ns        56316
BM_FakeLua_Popcount_TCC/10000                         161116 ns       161109 ns         4345
BM_FakeLua_Popcount_TCC/100000                       1755394 ns      1754928 ns          363
BM_FakeLua_Popcount_GCC/1000                            3425 ns         3343 ns       218675
BM_FakeLua_Popcount_GCC/10000                          41250 ns        41248 ns        16412
BM_FakeLua_Popcount_GCC/100000                        531536 ns       518373 ns         1099
BM_CPP_InsertionSort/50                                  724 ns          724 ns       933581
BM_CPP_InsertionSort/100                                3141 ns         3141 ns       200466
BM_CPP_InsertionSort/200                               12308 ns        12206 ns        58773
BM_Lua_InsertionSort/50                                40602 ns        40582 ns        18335
BM_Lua_InsertionSort/100                              143575 ns       143564 ns         4278
BM_Lua_InsertionSort/200                              608322 ns       595346 ns         1176
BM_FakeLua_InsertionSort_TCC/50                        69734 ns        69677 ns         9663
BM_FakeLua_InsertionSort_TCC/100                      269627 ns       269433 ns         2310
BM_FakeLua_InsertionSort_TCC/200                     1152476 ns      1137985 ns          659
BM_FakeLua_InsertionSort_GCC/50                        13199 ns        13185 ns        53533
BM_FakeLua_InsertionSort_GCC/100                       54884 ns        54660 ns         9418
BM_FakeLua_InsertionSort_GCC/200                      199820 ns       199805 ns         3523
BM_CPP_MatMul                                           3.10 ns         3.10 ns    214102667
BM_Lua_MatMul                                           2232 ns         2172 ns       340955
BM_FakeLua_MatMul_TCC                                   2968 ns         2968 ns       246347
BM_FakeLua_MatMul_GCC                                    538 ns          537 ns      1252834
BM_VarTable_Set/2                                        140 ns          138 ns      5755225
BM_VarTable_Set/4                                        156 ns          154 ns      4576289
BM_VarTable_Set/8                                        193 ns          193 ns      3532134
BM_VarTable_Set/16                                       667 ns          641 ns       837737
BM_VarTable_Set/32                                      1596 ns         1584 ns       476168
BM_VarTable_Set/64                                      3213 ns         3211 ns       217736
BM_VarTable_Set/128                                     6615 ns         6609 ns        98841
BM_VarTable_Set/256                                    14386 ns        13928 ns        52179
BM_VarTable_Set/512                                    27996 ns        27821 ns        25603
BM_VarTable_Set/1024                                   54220 ns        54154 ns        11618
BM_StdUnorderedMap_Set/2                                96.1 ns         92.6 ns      5717854
BM_StdUnorderedMap_Set/4                                 144 ns          144 ns      5145484
BM_StdUnorderedMap_Set/8                                 256 ns          256 ns      2723912
BM_StdUnorderedMap_Set/16                                615 ns          615 ns      1127721
BM_StdUnorderedMap_Set/32                               1263 ns         1263 ns       488665
BM_StdUnorderedMap_Set/64                               2814 ns         2770 ns       271886
BM_StdUnorderedMap_Set/128                              5381 ns         5379 ns       119890
BM_StdUnorderedMap_Set/256                             12379 ns        12370 ns        43202
BM_StdUnorderedMap_Set/512                             27256 ns        27108 ns        26526
BM_StdUnorderedMap_Set/1024                            51121 ns        51057 ns        12280
BM_LuaTable_Set/2                                        745 ns          748 ns       703668
BM_LuaTable_Set/4                                       1026 ns         1012 ns       748007
BM_LuaTable_Set/8                                       1280 ns         1284 ns       581354
BM_LuaTable_Set/16                                      1771 ns         1759 ns       424946
BM_LuaTable_Set/32                                      2666 ns         2625 ns       286632
BM_LuaTable_Set/64                                      4357 ns         4324 ns       177428
BM_LuaTable_Set/128                                     7468 ns         7436 ns        97580
BM_LuaTable_Set/256                                    13664 ns        13677 ns        51763
| BM_LuaTable_Set/512                                    22957 ns        22978 ns        26986
BM_LuaTable_Set/1024                                   48506 ns        47898 ns        13963
BM_VarTable_Get/2                                       24.4 ns         24.4 ns     28299105
BM_VarTable_Get/4                                       57.8 ns         56.4 ns     12434540
BM_VarTable_Get/8                                        112 ns          112 ns      6245513
BM_VarTable_Get/16                                       197 ns          197 ns      2943449
BM_VarTable_Get/32                                       417 ns          416 ns      1770272
BM_VarTable_Get/64                                       853 ns          852 ns       792935
BM_VarTable_Get/128                                     1633 ns         1631 ns       345683
BM_VarTable_Get/256                                     3226 ns         3192 ns       221693
BM_VarTable_Get/512                                     6235 ns         6231 ns       109594
BM_VarTable_Get/1024                                   13113 ns        13102 ns        43035
BM_StdUnorderedMap_Get/2                                9.91 ns         9.91 ns     70033905
BM_StdUnorderedMap_Get/4                                21.6 ns         21.5 ns     31388158
BM_StdUnorderedMap_Get/8                                47.0 ns         46.5 ns     15152215
BM_StdUnorderedMap_Get/16                               94.1 ns         94.1 ns      7399737
BM_StdUnorderedMap_Get/32                                190 ns          190 ns      3533024
BM_StdUnorderedMap_Get/64                                388 ns          384 ns      1827371
BM_StdUnorderedMap_Get/128                               753 ns          752 ns       894278
BM_StdUnorderedMap_Get/256                              1577 ns         1564 ns       452193
BM_StdUnorderedMap_Get/512                              3175 ns         3132 ns       226392
BM_StdUnorderedMap_Get/1024                             6236 ns         6234 ns       107162
BM_LuaTable_Get/2                                       33.4 ns         33.4 ns     19738438
BM_LuaTable_Get/4                                       71.3 ns         68.4 ns     10453731
BM_LuaTable_Get/8                                        138 ns          138 ns      5123390
BM_LuaTable_Get/16                                       267 ns          267 ns      2474660
BM_LuaTable_Get/32                                       557 ns          550 ns      1280936
BM_LuaTable_Get/64                                      1000 ns          999 ns       667660
BM_LuaTable_Get/128                                     2075 ns         2074 ns       301684
BM_LuaTable_Get/256                                     6999 ns         6804 ns        99525
BM_LuaTable_Get/512                                     8310 ns         8309 ns        82584
BM_LuaTable_Get/1024                                   16738 ns        16736 ns        35517
BM_VarTable_Iter/2                                      1.65 ns         1.56 ns    479688515
BM_VarTable_Iter/4                                      2.73 ns         2.73 ns    251187900
BM_VarTable_Iter/8                                      6.34 ns         6.34 ns    107846218
BM_VarTable_Iter/16                                     15.2 ns         15.0 ns     50295934
BM_VarTable_Iter/32                                     30.1 ns         29.6 ns     25328116
BM_VarTable_Iter/64                                     55.2 ns         55.2 ns      9711586
BM_VarTable_Iter/128                                     116 ns          116 ns      5522213
BM_VarTable_Iter/256                                     231 ns          230 ns      2519796
BM_VarTable_Iter/512                                     515 ns          489 ns      1280083
BM_VarTable_Iter/1024                                    925 ns          925 ns       741599
BM_StdUnorderedMap_Iter/2                              0.848 ns        0.848 ns    803148925
BM_StdUnorderedMap_Iter/4                               1.88 ns         1.87 ns    320050541
BM_StdUnorderedMap_Iter/8                               4.51 ns         4.48 ns    169904261
BM_StdUnorderedMap_Iter/16                              11.2 ns         11.0 ns     66925487
BM_StdUnorderedMap_Iter/32                              27.7 ns         27.7 ns     25717730
BM_StdUnorderedMap_Iter/64                              71.5 ns         69.3 ns     10063773
BM_StdUnorderedMap_Iter/128                              169 ns          169 ns      4096055
BM_StdUnorderedMap_Iter/256                              417 ns          412 ns      1312993
BM_StdUnorderedMap_Iter/512                              803 ns          803 ns       836849
BM_StdUnorderedMap_Iter/1024                            1668 ns         1667 ns       400541
BM_LuaTable_Iter/2                                      64.3 ns         64.3 ns     11155801
BM_LuaTable_Iter/4                                       118 ns          118 ns      5635063
BM_LuaTable_Iter/8                                       212 ns          212 ns      2705394
BM_LuaTable_Iter/16                                      433 ns          432 ns      1760654
BM_LuaTable_Iter/32                                      790 ns          790 ns       886884
BM_LuaTable_Iter/64                                     1518 ns         1517 ns       412217
BM_LuaTable_Iter/128                                    3282 ns         3201 ns       234436
BM_LuaTable_Iter/256                                    5987 ns         5983 ns       112548
BM_LuaTable_Iter/512                                   11863 ns        11862 ns        42585
BM_LuaTable_Iter/1024                                  26150 ns        25922 ns        29268
BM_VarTable_Del/2                                        287 ns          287 ns      2520422
BM_VarTable_Del/4                                        321 ns          321 ns      2110579
BM_VarTable_Del/8                                        397 ns          398 ns      1780046
BM_VarTable_Del/16                                       543 ns          537 ns      1331910
BM_VarTable_Del/32                                       998 ns          950 ns       893373
BM_VarTable_Del/64                                      1747 ns         1667 ns       478376
BM_VarTable_Del/128                                     3127 ns         2982 ns       275834
BM_VarTable_Del/256                                     5882 ns         5484 ns       134127
BM_VarTable_Del/512                                    12665 ns        10974 ns        74116
BM_VarTable_Del/1024                                   24685 ns        22462 ns        44613
BM_StdUnorderedMap_Del/2                                 306 ns          307 ns      2278882
BM_StdUnorderedMap_Del/4                                 308 ns          309 ns      1972854
BM_StdUnorderedMap_Del/8                                 369 ns          367 ns      1951609
BM_StdUnorderedMap_Del/16                                596 ns          596 ns      1215863
BM_StdUnorderedMap_Del/32                                962 ns          959 ns       750678
BM_StdUnorderedMap_Del/64                               1689 ns         1688 ns       422065
BM_StdUnorderedMap_Del/128                              3304 ns         3267 ns       218622
BM_StdUnorderedMap_Del/256                              5041 ns         5035 ns       100000
BM_StdUnorderedMap_Del/512                              9672 ns         9666 ns        72898
BM_StdUnorderedMap_Del/1024                            19151 ns        19121 ns        36315
BM_LuaTable_Del/2                                        563 ns          560 ns      1229284
BM_LuaTable_Del/4                                        586 ns          575 ns      1285863
BM_LuaTable_Del/8                                        634 ns          634 ns      1153904
BM_LuaTable_Del/16                                       735 ns          735 ns       916097
BM_LuaTable_Del/32                                       983 ns          980 ns       659553
BM_LuaTable_Del/64                                      1450 ns         1444 ns       466417
BM_LuaTable_Del/128                                     2435 ns         2403 ns       297972
BM_LuaTable_Del/256                                     5028 ns         4904 ns       143032
BM_LuaTable_Del/512                                     8150 ns         8055 ns        89177
BM_LuaTable_Del/1024                                   15935 ns        15598 ns        45368
```

## 算法性能分析（C++ vs Lua vs FakeLua TCC vs FakeLua GCC）

以下取各算法的典型参数做横向对比（CPU Time），**倍数 = 对应实现时间 / C++ 时间**：

### 1. Fibonacci（n=32，递归无记忆化）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 5.05 ms | 1x |
| Lua | 275.64 ms | **54.6x** 慢 |
| FakeLua TCC | 19.41 ms | **3.8x** 慢 |
| FakeLua GCC | 4.86 ms | **0.96x** 快 (约持平) |

> GCC 比 Lua 快 **56.7x**，TCC 比 Lua 快 **14.2x**。数值参数特化生成原生递归 + 原生条件比较，是 TCC/GCC 均优于 Lua 的关键。

### 2. GCD（a=832040, b=514229，约 30 次迭代）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 121.0 ns | 1x |
| Lua | 472.0 ns | **3.9x** 慢 |
| FakeLua TCC | 227.0 ns | **1.9x** 慢 (比 Lua 快 **2.1x**) |
| FakeLua GCC | 212.0 ns | **1.75x** 慢 (比 Lua 快 **2.2x**) |

### 3. PowMod（base=1234567, exp=7654321, mod=1e9+7，约 23 次循环，用 `%`/`//`）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 296.0 ns | 1x |
| Lua | 971.0 ns | **3.3x** 慢 |
| FakeLua TCC | 702.0 ns | **2.4x** 慢 (比 Lua 快 **1.4x**) |
| FakeLua GCC | 429.0 ns | **1.45x** 慢 (比 Lua 快 **2.3x**) |

### 4. FastPow（base=1234567, exp=7654321, mod=1e9+7，用 `&`/`>>`）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 296.0 ns | 1x |
| Lua | 846.0 ns | **2.9x** 慢 |
| FakeLua TCC | 430.0 ns | **1.5x** 慢 (比 Lua 快 **2.0x**) |
| FakeLua GCC | 401.0 ns | **1.4x** 慢 (比 Lua 快 **2.1x**) |

> FastPow 用位运算 `&`/`>>` 代替取余/整除 `%`/`//`，在 FakeLua TCC 下比 PowMod 快约 **1.6x**（702.0 ns → 430.0 ns），说明 TCC 对位运算的代码生成较优。GCC 两者表现也较接近。

### 5. Sum（n=5000000，纯循环累加）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 1.55 ms | 1x |
| Lua | 28.35 ms | **18.3x** 慢 |
| FakeLua TCC | 14.39 ms | **9.3x** 慢 (比 Lua 快 **2.0x**) |
| FakeLua GCC | 1.55 ms | **1.0x** 与 C++ 相同 |

> 纯整数累加循环：FakeLua GCC 与 C++ 几乎完全相同，说明 GCC `-O3` 对简单数值循环已达到 C++ 原生水平。TCC 比 Lua 快 **2.0x**。

### 6. BubbleSort（n=200，O(n²)，含表操作）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 126.6 µs | 1x |
| Lua | 832.4 µs | **6.6x** 慢 |
| FakeLua TCC | 1779.9 µs | **14.1x** 慢 (比 Lua 慢 **2.1x**) |
| FakeLua GCC | 330.2 µs | **2.6x** 慢 (比 Lua 快 **2.5x**) |

> 含大量表 Set/Get 操作 of the 排序算法，**TCC 表现明显弱于 Lua**（2.1x 差距）。TCC 对 table 索引操作生成的代码路径较长（无寄存器分配优化），而 Lua 解释器在 table 操作上已高度优化。GCC 目前比 Lua 快约 **2.5x**。

### 7. Sieve（n=5000，Eratosthenes 筛）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 12.4 µs | 1x |
| Lua | 267.9 µs | **21.6x** 慢 |
| FakeLua TCC | 568.2 µs | **45.8x** 慢 (比 Lua 慢 **2.1x**) |
| FakeLua GCC | 121.0 µs | **9.8x** 慢 (比 Lua 快 **2.2x**) |

> 筛法涉及大量 boolean 表操作（`is_prime[j] = false`），TCC 在此类写密集型表操作上比 Lua 慢 **2.1x**，同样反映 TCC 在 table 写操作的代码生成开销。GCC 比 Lua 快 **2.2x**。

### 8. BinarySearch（n=1000，n 次二分查找）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 17.6 µs | 1x |
| Lua | 403.3 µs | **22.9x** 慢 |
| FakeLua TCC | 557.5 µs | **31.7x** 慢 (比 Lua 慢 **1.4x**) |
| FakeLua GCC | 91.3 µs | **5.2x** 慢 (比 Lua 快 **4.4x**) |

> 二分查找含 `break` 语句的 while 循环和表随机访问。TCC 比 Lua 慢 1.4x，GCC 比 Lua 快 4.4x。

### 9. Popcount（n=100000，Brian Kernighan 位计数累加）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 488.9 µs | 1x |
| Lua | 12.39 ms | **25.3x** 慢 |
| FakeLua TCC | 1.75 ms | **3.6x** 慢 (比 Lua 快 **7.1x**) |
| FakeLua GCC | 518.4 µs | **1.06x** 慢 (比 Lua 快 **23.9x**) |

> 纯整数位运算（`&`，`!=`），无表操作。**TCC 比 Lua 快 7.1x，GCC 比 Lua 快 23.9x，非常接近 C++**。这是纯整数算法场景，FakeLua 优势最大。

### 10. InsertionSort（n=200，O(n²)，含表操作）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 12.2 µs | 1x |
| Lua | 595.3 µs | **48.8x** 慢 |
| FakeLua TCC | 1138.0 µs | **93.3x** 慢 (比 Lua 慢 **1.9x**) |
| FakeLua GCC | 199.8 µs | **16.4x** 慢 (比 Lua 快 **3.0x**) |

> 与冒泡排序类似，表操作为瓶颈。TCC 比 Lua 慢约 1.9x，GCC 快于 Lua 约 3.0x。

### 11. MatMul（单次 3×3 矩阵乘法）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 3.10 ns | 1x |
| Lua | 2172.0 ns | **700.6x** 慢 |
| FakeLua TCC | 2968.0 ns | **957.4x** 慢 (比 Lua 慢 **1.37x**) |
| FakeLua GCC | 537.0 ns | **173.2x** 慢 (比 Lua 快 **4.0x**) |

> 矩阵乘法每次调用需创建 3 个 Lua table（各 9 个元素），**table 创建与 GC 开销远大于实际计算**。C++ 的数 ns 得益于 `-O3` 向量化。TCC 在 table 分配+初始化开销下比 Lua 慢 1.37x；GCC 通过更优的寄存器分配比 Lua 快 4.0x。

---

## FakeLua TCC vs Lua 5.4 详细对比

| 算法 | 参数 | Lua | FakeLua TCC | 结果 |
|------|------|-----|-------------|------|
| Fibonacci | n=32 | 275.64 ms | 19.41 ms | TCC **14.2x 快** |
| GCD | 832040/514229 | 472.0 ns | 227.0 ns | TCC **2.1x 快** |
| PowMod | 1234567/7654321/1e9+7 | 971.0 ns | 702.0 ns | TCC **1.4x 快** |
| FastPow | 1234567/7654321/1e9+7 | 846.0 ns | 430.0 ns | TCC **2.0x 快** |
| Sum | n=5000000 | 28.35 ms | 14.39 ms | TCC **2.0x 快** |
| BubbleSort | n=200 | 832.4 µs | 1779.9 µs | TCC **2.1x 慢** |
| Sieve | n=5000 | 267.9 µs | 568.2 µs | TCC **2.1x 慢** |
| BinarySearch | n=1000 | 403.3 µs | 557.5 µs | TCC **1.4x 慢** |
| Popcount | n=100000 | 12.39 ms | 1.75 ms | TCC **7.1x 快** |
| InsertionSort | n=200 | 595.3 µs | 1138.0 µs | TCC **1.9x 慢** |
| MatMul | 单次 3×3 | 2172.0 ns | 2968.0 ns | TCC **1.37x 慢** |

> **结论**：TCC 在**纯整数运算**（无表操作）算法上快于 Lua（1.4x ~ 14.2x）；在**表操作密集**算法上慢于 Lua（1.37x ~ 2.1x）。TCC 生成的代码对 table 读写路径较长（无循环不变量提升、无内联），而 Lua 解释器对 table 操作做了深度优化。

---

## FakeLua GCC vs Lua 5.4 详细对比

| 算法 | 参数 | Lua | FakeLua GCC | GCC 快多少 |
|------|------|-----|-------------|-----------|
| Fibonacci | n=32 | 275.64 ms | 4.86 ms | **56.7x** |
| GCD | 832040/514229 | 472.0 ns | 212.0 ns | **2.2x** |
| PowMod | 1234567/7654321/1e9+7 | 971.0 ns | 429.0 ns | **2.3x** |
| FastPow | 1234567/7654321/1e9+7 | 846.0 ns | 401.0 ns | **2.1x** |
| Sum | n=5000000 | 28.35 ms | 1.55 ms | **18.3x** |
| BubbleSort | n=200 | 832.4 µs | 330.2 µs | **2.5x** |
| Sieve | n=5000 | 267.9 µs | 121.0 µs | **2.2x** |
| BinarySearch | n=1000 | 403.3 µs | 91.3 µs | **4.4x** |
| Popcount | n=100000 | 12.39 ms | 518.4 µs | **23.9x** |
| InsertionSort | n=200 | 595.3 µs | 199.8 µs | **3.0x** |
| MatMul | 单次 3×3 | 2172.0 ns | 537.0 ns | **4.0x** |

### FakeLua GCC 按场景分类

| 场景类型 | FakeLua GCC 比 Lua 快 | 原因 |
|---------|----------------------|------|
| **纯整数累加 (Sum)** | **18.3x** | GCC `-O3` 向量化，达到 C++ 原生水平 |
| **纯整数位运算 (Popcount)** | **23.9x** | 位运算全部原生化，GCC 激进优化 |
| **递归 (Fibonacci)** | **56.7x** | 数值特化 + 原生递归，GCC 深度内联 |
| **算术循环 (PowMod/FastPow)** | **2.3–2.1x** | 循环体数值特化，取模运算受益于寄存器优化 |
| **短迭代 (GCD)** | **2.2x** | 迭代次数少，函数调用开销占比高 |
| **二分查找 (BinarySearch)** | **4.4x** | 混合数值+表操作，GCC 部分消除 table 开销 |
| **表操作为主 (BubbleSort/InsertionSort/Sieve/MatMul)** | **2.2–4.0x** | table 操作仍是瓶颈，GCC 无法完全消除 |

> **FakeLua GCC 后端在所有算法上均快于 Lua 5.4**（2.1x ~ 56.7x），纯数值算法优势巨大，表操作密集型算法优势也极为明显（约 2.2x ~ 4.0x）。

---

## 表操作性能分析（VarTable vs unordered_map vs Lua Table）

取 n=1024 比较：

| 操作 | VarTable | unordered_map | Lua Table | VarTable vs map | VarTable vs Lua |
|------|----------|---------------|-----------|-----------------|-----------------|
| Set  | 54.2 µs | 51.1 µs | 47.9 µs | **1.06x** 慢 | **1.13x** 慢 |
| Get  | 13.1 µs | 6.2 µs | 16.7 µs | **2.1x** 慢 | **1.27x** 快 |
| Iter | 0.93 µs | 1.67 µs | 25.9 µs | **1.8x** 快 | **28.0x** 快 |
| Del  | 22.5 µs | 19.1 µs | 15.6 µs | **1.18x** 慢 | **1.44x** 慢 |

> VarTable 的核心优势在 **Iterate** 上（紧密的 active_list 遍历完全 cache-friendly），在 1024 元素时比 unordered_map 快 1.8 倍，比 Lua Table 快 **28.0 - 22.7 倍**。

---

## 总体结论

1. **C++ 最快**：在全部算法上领先，受益于 `-O3` 内联/展开/向量化。

2. **FakeLua GCC 全面超越 Lua，纯数值场景接近 C++ 原生**：
   - 纯整数运算（Sum、Popcount）：GCC 与 C++ 完全持平或接近，比 Lua 快 **18.3–23.9x**
   - 递归（Fibonacci）：比 Lua 快 **56.7x**，性能非常接近 C++
   - 算术循环（PowMod/FastPow）：比 Lua 快 **2.3–2.1x**
   - 表操作为主（BubbleSort/InsertionSort/Sieve/MatMul）：比 Lua 快 **2.2–4.0x**

3. **FakeLua TCC 优缺点分明**：
   - 纯整数算法（Sum、Popcount、FastPow、Fibonacci）：比 Lua 快 **2.0x ~ 14.2x**
   - 表操作密集型算法（BubbleSort、Sieve、InsertionSort、MatMul）：比 Lua 慢 **1.37x ~ 2.1x**
   - TCC 生成的 C 代码对 table 读写路径较长，而 Lua 解释器对 table 操作深度优化，导致表密集算法下 TCC 明显落后

4. **位运算 vs 取模**（FastPow `&`/`>>` vs PowMod `%`/`//`）：在 TCC 下位运算快（702.0 ns vs 430.0 ns），GCC 下两者也较接近，说明 FakeLua 已能对两种写法生成相近质量的代码。

5. **VarTable 遍历性能极优**：Iter 比 Lua Table 快 **28.0 倍**，比 `unordered_map` 快 1.8 倍，核心在于 active_list 的紧凑布局。

> 注：ASLR 开启，结果有一定随机噪声；建议在 `--cpu-scaling-enabled=false` 环境下多重复后取均值。
