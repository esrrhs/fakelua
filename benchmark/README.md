# Benchmark Results

本文件记录本次在本地对 `bench_mark` 的完整运行结果，并补充对新 `benchmark_algo.cpp`（C++ / Lua / FakeLua 算法对比）的简要分析。

## 变更说明

- 将 `benchmark_cpp.cpp`、`benchmark_lua.cpp`、`benchmark_fakelua.cpp` 合并为 `benchmark_algo.cpp`
- 在同一文件中统一对比三类实现：
  - C++
  - Lua（Lua 5.4）
  - FakeLua（JIT_TCC）
- 新增多算法与多参数：
  - 递归斐波那契（Fibonacci）
  - 欧几里得最大公约数（GCD）
  - 快速幂取模（PowMod）
  - 线性求和（Sum）

## 运行环境

- 时间：2026-04-16
- 机器：4 vCPU @ 2800 MHz
- 二进制：`/home/runner/work/fakelua/fakelua/build/bin/bench_mark`

## 运行命令

```bash
cd /home/runner/work/fakelua/fakelua/build/bin
./bench_mark --benchmark_repetitions=1 --benchmark_report_aggregates_only=true
```

## 完整输出

```text
Starting benchmarks...
---------------------------------------------------------------------------------------
Benchmark                                             Time             CPU   Iterations
---------------------------------------------------------------------------------------
BM_CPP_Fibonacci/20                               11291 ns        11290 ns        61996
BM_CPP_Fibonacci/25                              148055 ns       148049 ns         4733
BM_CPP_Fibonacci/30                             1415977 ns      1415871 ns          466
BM_CPP_Fibonacci/32                             3752691 ns      3752405 ns          186
BM_Lua_Fibonacci/20                              472437 ns       472397 ns         1689
BM_Lua_Fibonacci/25                             4920128 ns      4919989 ns          133
BM_Lua_Fibonacci/30                            51471760 ns     51470379 ns           14
BM_Lua_Fibonacci/32                           136234974 ns    136230425 ns            5
BM_FakeLua_Fibonacci/20                          791334 ns       791318 ns          885
BM_FakeLua_Fibonacci/25                         8772088 ns      8771412 ns           80
BM_FakeLua_Fibonacci/30                        97499697 ns     97495280 ns            7
BM_FakeLua_Fibonacci/32                       254802669 ns    254781661 ns            3
BM_CPP_GCD/832040/514229                           92.2 ns         92.2 ns      7594225
BM_CPP_GCD/123456789/987654321                     8.64 ns         8.64 ns     81041431
BM_CPP_GCD/2147483647/1073741823                   5.80 ns         5.80 ns    121596159
BM_Lua_GCD/832040/514229                            312 ns          312 ns      2244335
BM_Lua_GCD/123456789/987654321                     73.3 ns         73.2 ns      9540195
BM_Lua_GCD/2147483647/1073741823                   64.4 ns         64.4 ns     10851449
BM_FakeLua_GCD/832040/514229                        959 ns          959 ns       729661
BM_FakeLua_GCD/123456789/987654321                  141 ns          141 ns      4955709
BM_FakeLua_GCD/2147483647/1073741823                110 ns          110 ns      6345882
BM_CPP_PowMod/2/1000/1000000007                    51.5 ns         51.5 ns     13595714
BM_CPP_PowMod/7/1000000/1000000007                  118 ns          118 ns      5954855
BM_CPP_PowMod/1234567/7654321/1000000007            149 ns          149 ns      4697310
BM_Lua_PowMod/2/1000/1000000007                     251 ns          251 ns      2790744
BM_Lua_PowMod/7/1000000/1000000007                  450 ns          450 ns      1545703
BM_Lua_PowMod/1234567/7654321/1000000007            501 ns          501 ns      1394895
BM_FakeLua_PowMod/2/1000/1000000007                 948 ns          948 ns       738959
BM_FakeLua_PowMod/7/1000000/1000000007             1813 ns         1813 ns       386019
BM_FakeLua_PowMod/1234567/7654321/1000000007       2192 ns         2192 ns       319189
BM_CPP_Sum/10000                                   2288 ns         2288 ns       306010
BM_CPP_Sum/100000                                 22766 ns        22765 ns        30756
BM_CPP_Sum/1000000                               227554 ns       227544 ns         3076
BM_CPP_Sum/5000000                              1139727 ns      1139705 ns          615
BM_Lua_Sum/10000                                  43831 ns        43829 ns        16008
BM_Lua_Sum/100000                                437534 ns       437517 ns         1598
BM_Lua_Sum/1000000                              4411966 ns      4411832 ns          159
BM_Lua_Sum/5000000                             22056466 ns     22055822 ns           32
BM_FakeLua_Sum/10000                             278926 ns       278921 ns         2519
BM_FakeLua_Sum/100000                           2798391 ns      2798321 ns          250
BM_FakeLua_Sum/1000000                         28042179 ns     28041453 ns           25
BM_FakeLua_Sum/5000000                        140303441 ns    140298121 ns            5
BM_VarTable_Set/2                                   151 ns          151 ns      4700972
BM_VarTable_Set/4                                   165 ns          165 ns      4370915
BM_VarTable_Set/8                                   194 ns          194 ns      3680083
BM_VarTable_Set/16                                  703 ns          703 ns      1049798
BM_VarTable_Set/32                                 1729 ns         1728 ns       409474
BM_VarTable_Set/64                                 3756 ns         3756 ns       186512
BM_VarTable_Set/128                                7821 ns         7820 ns        96249
BM_VarTable_Set/256                               15806 ns        15805 ns        44936
BM_VarTable_Set/512                               31881 ns        31879 ns        22114
BM_VarTable_Set/1024                              63953 ns        63949 ns        11658
BM_StdUnorderedMap_Set/2                           71.8 ns         71.8 ns      9715833
BM_StdUnorderedMap_Set/4                            106 ns          106 ns      6587721
BM_StdUnorderedMap_Set/8                            185 ns          185 ns      3780481
BM_StdUnorderedMap_Set/16                           419 ns          419 ns      1675025
BM_StdUnorderedMap_Set/32                           913 ns          913 ns       768763
BM_StdUnorderedMap_Set/64                          1846 ns         1845 ns       379292
BM_StdUnorderedMap_Set/128                         3694 ns         3694 ns       189357
BM_StdUnorderedMap_Set/256                         8477 ns         8476 ns        83210
BM_StdUnorderedMap_Set/512                        17837 ns        17837 ns        39209
BM_StdUnorderedMap_Set/1024                       36427 ns        36426 ns        19275
BM_LuaTable_Set/2                                   778 ns          785 ns       939617
BM_LuaTable_Set/4                                   914 ns          919 ns       769733
BM_LuaTable_Set/8                                  1141 ns         1146 ns       619387
BM_LuaTable_Set/16                                 1587 ns         1594 ns       448133
BM_LuaTable_Set/32                                 2277 ns         2284 ns       314930
BM_LuaTable_Set/64                                 3651 ns         3660 ns       194556
BM_LuaTable_Set/128                                6309 ns         6319 ns       114397
BM_LuaTable_Set/256                               11439 ns        11450 ns        63882
BM_LuaTable_Set/512                               21380 ns        21394 ns        34136
BM_LuaTable_Set/1024                              41016 ns        41040 ns        18184
BM_VarTable_Get/2                                  17.0 ns         17.0 ns     41319267
BM_VarTable_Get/4                                  37.4 ns         37.4 ns     18692978
BM_VarTable_Get/8                                  77.2 ns         77.2 ns      9066604
BM_VarTable_Get/16                                  133 ns          133 ns      5260842
BM_VarTable_Get/32                                  250 ns          250 ns      2801112
BM_VarTable_Get/64                                  486 ns          486 ns      1440053
BM_VarTable_Get/128                                 965 ns          965 ns       725290
BM_VarTable_Get/256                                1923 ns         1923 ns       363665
BM_VarTable_Get/512                                3839 ns         3839 ns       182166
BM_VarTable_Get/1024                               7686 ns         7685 ns        91169
BM_StdUnorderedMap_Get/2                           14.2 ns         14.2 ns     49851570
BM_StdUnorderedMap_Get/4                           20.9 ns         20.9 ns     33440384
BM_StdUnorderedMap_Get/8                           33.2 ns         33.2 ns     21083192
BM_StdUnorderedMap_Get/16                          63.5 ns         63.5 ns     11038818
BM_StdUnorderedMap_Get/32                           110 ns          110 ns      6309658
BM_StdUnorderedMap_Get/64                           210 ns          210 ns      3324592
BM_StdUnorderedMap_Get/128                          431 ns          431 ns      1624743
BM_StdUnorderedMap_Get/256                          836 ns          836 ns       840356
BM_StdUnorderedMap_Get/512                         1676 ns         1676 ns       417886
BM_StdUnorderedMap_Get/1024                        3403 ns         3403 ns       204444
BM_LuaTable_Get/2                                  28.6 ns         28.6 ns     24334725
BM_LuaTable_Get/4                                  54.9 ns         54.9 ns     12721957
BM_LuaTable_Get/8                                   109 ns          109 ns      6440963
BM_LuaTable_Get/16                                  245 ns          245 ns      2857630
BM_LuaTable_Get/32                                  458 ns          458 ns      1528573
BM_LuaTable_Get/64                                  883 ns          883 ns       794301
BM_LuaTable_Get/128                                1722 ns         1722 ns       406731
BM_LuaTable_Get/256                                3422 ns         3422 ns       204692
BM_LuaTable_Get/512                                6866 ns         6865 ns       102790
BM_LuaTable_Get/1024                              13600 ns        13599 ns        51538
BM_VarTable_Iter/2                                 1.12 ns         1.12 ns    625569520
BM_VarTable_Iter/4                                 2.33 ns         2.33 ns    302380696
BM_VarTable_Iter/8                                 2.93 ns         2.93 ns    240044863
BM_VarTable_Iter/16                                7.48 ns         7.48 ns     92993545
BM_VarTable_Iter/32                                16.8 ns         16.8 ns     41669477
BM_VarTable_Iter/64                                35.6 ns         35.6 ns     19704505
BM_VarTable_Iter/128                               79.7 ns         79.7 ns      8769733
BM_VarTable_Iter/256                                155 ns          155 ns      4539566
BM_VarTable_Iter/512                                303 ns          303 ns      2310733
BM_VarTable_Iter/1024                               600 ns          600 ns      1167364
BM_StdUnorderedMap_Iter/2                         0.583 ns        0.583 ns   1156505876
BM_StdUnorderedMap_Iter/4                          2.92 ns         2.92 ns    241271550
BM_StdUnorderedMap_Iter/8                          2.88 ns         2.88 ns    242568370
BM_StdUnorderedMap_Iter/16                         5.43 ns         5.43 ns    129866268
BM_StdUnorderedMap_Iter/32                         14.0 ns         14.0 ns     49933017
BM_StdUnorderedMap_Iter/64                         42.8 ns         42.8 ns     16603747
BM_StdUnorderedMap_Iter/128                         122 ns          122 ns      5733245
BM_StdUnorderedMap_Iter/256                         375 ns          375 ns      1863093
BM_StdUnorderedMap_Iter/512                         744 ns          744 ns       940744
BM_StdUnorderedMap_Iter/1024                       1495 ns         1495 ns       465571
BM_LuaTable_Iter/2                                 55.7 ns         55.7 ns     12521740
BM_LuaTable_Iter/4                                 98.2 ns         98.2 ns      7128686
BM_LuaTable_Iter/8                                  210 ns          210 ns      3337888
BM_LuaTable_Iter/16                                 381 ns          381 ns      1835995
BM_LuaTable_Iter/32                                 731 ns          731 ns       958487
BM_LuaTable_Iter/64                                1426 ns         1426 ns       492233
BM_LuaTable_Iter/128                               2811 ns         2810 ns       248966
BM_LuaTable_Iter/256                               5594 ns         5592 ns       124773
BM_LuaTable_Iter/512                              11166 ns        11165 ns        62977
BM_LuaTable_Iter/1024                             22283 ns        22282 ns        31458
BM_VarTable_Del/2                                   289 ns          287 ns      2440953
BM_VarTable_Del/4                                   308 ns          306 ns      2219979
BM_VarTable_Del/8                                   349 ns          351 ns      1990990
BM_VarTable_Del/16                                  426 ns          424 ns      1654074
BM_VarTable_Del/32                                  586 ns          580 ns      1233730
BM_VarTable_Del/64                                  871 ns          864 ns       817783
BM_VarTable_Del/128                                1467 ns         1458 ns       481942
BM_VarTable_Del/256                                2645 ns         2629 ns       267268
BM_VarTable_Del/512                                4982 ns         4967 ns       141219
BM_VarTable_Del/1024                               9500 ns         9489 ns        73833
BM_StdUnorderedMap_Del/2                            299 ns          301 ns      2327318
BM_StdUnorderedMap_Del/4                            322 ns          323 ns      2164565
BM_StdUnorderedMap_Del/8                            370 ns          371 ns      1881123
BM_StdUnorderedMap_Del/16                           474 ns          475 ns      1473034
BM_StdUnorderedMap_Del/32                           726 ns          726 ns       967394
BM_StdUnorderedMap_Del/64                          1144 ns         1145 ns       610265
BM_StdUnorderedMap_Del/128                         2040 ns         2037 ns       343942
BM_StdUnorderedMap_Del/256                         3719 ns         3717 ns       189088
BM_StdUnorderedMap_Del/512                         6509 ns         6504 ns       107649
BM_StdUnorderedMap_Del/1024                       12548 ns        12545 ns        55866
BM_LuaTable_Del/2                                   559 ns          562 ns      1245540
BM_LuaTable_Del/4                                   586 ns          590 ns      1189334
BM_LuaTable_Del/8                                   631 ns          634 ns      1104372
BM_LuaTable_Del/16                                  739 ns          740 ns       944733
BM_LuaTable_Del/32                                  913 ns          913 ns       767944
BM_LuaTable_Del/64                                 1256 ns         1255 ns       559197
BM_LuaTable_Del/128                                1933 ns         1930 ns       363409
BM_LuaTable_Del/256                                3278 ns         3272 ns       214859
BM_LuaTable_Del/512                                5929 ns         5918 ns       118207
BM_LuaTable_Del/1024                              11230 ns        11214 ns        62386
```

## 算法对比结论（benchmark_algo）

以下采用本次完整输出中的典型参数（CPU 时间）做横向比较：

- Fibonacci /32
  - C++: `3.75 ms`
  - Lua: `136.23 ms`（约 `36x` 慢于 C++）
  - FakeLua: `254.78 ms`（约 `68x` 慢于 C++，约 `1.87x` 慢于 Lua）

- GCD /832040/514229
  - C++: `92.2 ns`
  - Lua: `312 ns`（约 `3.4x` 慢于 C++）
  - FakeLua: `959 ns`（约 `10.4x` 慢于 C++，约 `3.1x` 慢于 Lua）

- PowMod /1234567/7654321/1000000007
  - C++: `149 ns`
  - Lua: `501 ns`（约 `3.4x` 慢于 C++）
  - FakeLua: `2192 ns`（约 `14.7x` 慢于 C++，约 `4.4x` 慢于 Lua）

- Sum /5000000
  - C++: `1.14 ms`
  - Lua: `22.06 ms`（约 `19.4x` 慢于 C++）
  - FakeLua: `140.30 ms`（约 `123x` 慢于 C++，约 `6.4x` 慢于 Lua）

## 分析

1. 本次数据中，C++ 在全部算法上都明显领先。
2. 纯 Lua 在这组算例下整体快于当前 FakeLua 路径。
3. 算法越偏“高频循环/递归调用”，FakeLua 与 C++ 的差距越明显（如 Fibonacci、Sum）。
4. 结合已有 `benchmark_table.cpp` 结果可见，FakeLua 的优势更多体现在其自定义运行时数据结构（如 VarTable）场景；在当前算法函数调用类 benchmark 中，JIT 端仍有优化空间（函数调用/运行时桥接成本）。

> 注：benchmark 输出提示 ASLR 开启，结果存在一定噪声；建议在固定环境下多次重复取均值进一步评估。
