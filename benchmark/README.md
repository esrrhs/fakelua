# Benchmark Results

本文件记录在本地以 **Release 模式**（`-O3 -DNDEBUG`）编译运行 `bench_mark` 的完整结果，以及对各算法的性能分析。

## 基准说明

### 算法对比（benchmark_algo.cpp）

将 C++、Lua 5.4、FakeLua（JIT_TCC / JIT_GCC）在同一文件中进行横向性能对比，覆盖四类算法：

| 算法 | 说明 | 参数规模 |
|------|------|---------|
| Fibonacci | 递归斐波那契（无记忆化） | n=20/25/30/32 |
| GCD | 欧几里得最大公约数 | 多组大整数对 |
| PowMod | 快速幂取模（二进制分解） | 多组底数/指数/模数 |
| Sum | 1..n 线性累加 | n=10000/100000/1000000/5000000 |

### 表操作对比（benchmark_table.cpp）

VarTable vs `std::unordered_map` vs Lua Table 的 Set/Get/Iter/Del 对比，参数从 2 到 1024。

---

## 运行环境

- 日期：2026-04-17
- 机器：16 vCPU @ 2000 MHz
- CPU 缓存：L1d 64 KiB (x16)，L1i 64 KiB (x16)，L2 512 KiB (x16)，L3 65536 KiB (x2)
- 构建模式：**Release**（`cmake .. -DCMAKE_BUILD_TYPE=Release`，最终编译标志 `-O3 -DNDEBUG`）
- FakeLua TCC JIT：**Release 模式**（`debug_mode=false`，TCC 启用 `-O2` 优化）
- FakeLua GCC JIT：**Release 模式**（`debug_mode=false`，GCC 启用 `-O3` 优化）
- 二进制：`build_release/bin/bench_mark`

## 运行命令

```bash
mkdir build_release && cd build_release
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
cd bin
./bench_mark --benchmark_repetitions=1 --benchmark_report_aggregates_only=true
```

---

## 完整原始输出

```text
2026-04-17T19:36:16+08:00
Running ./bench_mark
Run on (16 X 2000 MHz CPU s)
CPU Caches:
  L1 Data 64 KiB (x16)
  L1 Instruction 64 KiB (x16)
  L2 Unified 512 KiB (x16)
  L3 Unified 65536 KiB (x2)
Load Average: 3.32, 2.71, 2.38
***WARNING*** ASLR is enabled, the results may have unreproducible noise in them.
-------------------------------------------------------------------------------------------
Benchmark                                                 Time             CPU   Iterations
-------------------------------------------------------------------------------------------
BM_CPP_Fibonacci/20                                   24659 ns        24654 ns        27071
BM_CPP_Fibonacci/25                                  241002 ns       240998 ns         2792
BM_CPP_Fibonacci/30                                 3124163 ns      3124038 ns          237
BM_CPP_Fibonacci/32                                 8010534 ns      8010056 ns           88
BM_Lua_Fibonacci/20                                 1291018 ns      1291021 ns          529
BM_Lua_Fibonacci/25                                14083989 ns     14083917 ns           49
BM_Lua_Fibonacci/30                               153777903 ns    153776325 ns            5
BM_Lua_Fibonacci/32                               414549984 ns    414533192 ns            2
BM_FakeLua_Fibonacci_TCC/20                         1358633 ns      1358622 ns          516
BM_FakeLua_Fibonacci_TCC/25                        14908719 ns     14908451 ns           48
BM_FakeLua_Fibonacci_TCC/30                       167318147 ns    167303189 ns            4
BM_FakeLua_Fibonacci_TCC/32                       439358465 ns    439347139 ns            2
BM_FakeLua_Fibonacci_GCC/20                           76038 ns        76035 ns         9235
BM_FakeLua_Fibonacci_GCC/25                          845632 ns       845585 ns          833
BM_FakeLua_Fibonacci_GCC/30                         9716779 ns      9716450 ns           74
BM_FakeLua_Fibonacci_GCC/32                        26875814 ns     26874335 ns           26
BM_CPP_GCD/832040/514229                                173 ns          173 ns      4021164
BM_CPP_GCD/123456789/987654321                         21.7 ns         21.7 ns     32548611
BM_CPP_GCD/2147483647/1073741823                       17.1 ns         17.1 ns     40589250
BM_Lua_GCD/832040/514229                                852 ns          850 ns       849986
BM_Lua_GCD/123456789/987654321                          192 ns          192 ns      3724206
BM_Lua_GCD/2147483647/1073741823                        159 ns          159 ns      4385113
BM_FakeLua_GCD_TCC/832040/514229                       1273 ns         1273 ns       550938
BM_FakeLua_GCD_TCC/123456789/987654321                  272 ns          272 ns      2582969
BM_FakeLua_GCD_TCC/2147483647/1073741823                190 ns          190 ns      3695174
BM_FakeLua_GCD_GCC/832040/514229                        257 ns          257 ns      2664252
BM_FakeLua_GCD_GCC/123456789/987654321                 95.5 ns         96.4 ns      7446704
BM_FakeLua_GCD_GCC/2147483647/1073741823               94.1 ns         92.3 ns      7607102
BM_CPP_PowMod/2/1000/1000000007                         146 ns          146 ns      4768265
BM_CPP_PowMod/7/1000000/1000000007                      275 ns          275 ns      2545751
BM_CPP_PowMod/1234567/7654321/1000000007                396 ns          396 ns      1774244
BM_Lua_PowMod/2/1000/1000000007                         981 ns          981 ns       713569
BM_Lua_PowMod/7/1000000/1000000007                     1631 ns         1631 ns       431384
BM_Lua_PowMod/1234567/7654321/1000000007               2115 ns         2115 ns       326739
BM_FakeLua_PowMod_TCC/2/1000/1000000007                1543 ns         1543 ns       446361
BM_FakeLua_PowMod_TCC/7/1000000/1000000007             2776 ns         2775 ns       252514
BM_FakeLua_PowMod_TCC/1234567/7654321/1000000007       3408 ns         3408 ns       205163
BM_FakeLua_PowMod_GCC/2/1000/1000000007                 248 ns          248 ns      2819355
BM_FakeLua_PowMod_GCC/7/1000000/1000000007              426 ns          426 ns      1694528
BM_FakeLua_PowMod_GCC/1234567/7654321/1000000007        538 ns          538 ns      1301138
BM_CPP_Sum/10000                                       3965 ns         3965 ns       174931
BM_CPP_Sum/100000                                     39938 ns        39936 ns        17419
BM_CPP_Sum/1000000                                   397471 ns       397442 ns         1761
BM_CPP_Sum/5000000                                  1996890 ns      1996866 ns          352
BM_Lua_Sum/10000                                      94007 ns        93982 ns         7484
BM_Lua_Sum/100000                                    939528 ns       938990 ns          745
BM_Lua_Sum/1000000                                  9314728 ns      9313500 ns           75
BM_Lua_Sum/5000000                                 46640609 ns     46614825 ns           15
BM_FakeLua_Sum_TCC/10000                             325340 ns       325177 ns         2155
BM_FakeLua_Sum_TCC/100000                           3241208 ns      3238907 ns          217
BM_FakeLua_Sum_TCC/1000000                         33160156 ns     33159329 ns           21
BM_FakeLua_Sum_TCC/5000000                        163335048 ns    163238152 ns            4
BM_FakeLua_Sum_GCC/10000                               4069 ns         4068 ns       172292
BM_FakeLua_Sum_GCC/100000                             39921 ns        39899 ns        17550
BM_FakeLua_Sum_GCC/1000000                           401306 ns       401283 ns         1761
BM_FakeLua_Sum_GCC/5000000                          2034549 ns      2033175 ns          345
BM_VarTable_Set/2                                       202 ns          202 ns      3544564
BM_VarTable_Set/4                                       229 ns          229 ns      3138018
BM_VarTable_Set/8                                       310 ns          310 ns      2358290
BM_VarTable_Set/16                                      985 ns          985 ns       729003
BM_VarTable_Set/32                                     2454 ns         2448 ns       306789
BM_VarTable_Set/64                                     5498 ns         5498 ns       100000
BM_VarTable_Set/128                                   10860 ns        10860 ns        73955
BM_VarTable_Set/256                                   22855 ns        22854 ns        30928
BM_VarTable_Set/512                                   43955 ns        43953 ns        16676
BM_VarTable_Set/1024                                  89640 ns        89638 ns         9289
BM_StdUnorderedMap_Set/2                                157 ns          157 ns      4452680
BM_StdUnorderedMap_Set/4                                261 ns          261 ns      2684985
BM_StdUnorderedMap_Set/8                                435 ns          435 ns      1620555
BM_StdUnorderedMap_Set/16                               976 ns          976 ns       706453
BM_StdUnorderedMap_Set/32                              2063 ns         2063 ns       339634
BM_StdUnorderedMap_Set/64                              4224 ns         4223 ns       165370
BM_StdUnorderedMap_Set/128                             9685 ns         9685 ns        72415
BM_StdUnorderedMap_Set/256                            18080 ns        18080 ns        38785
BM_StdUnorderedMap_Set/512                            36680 ns        36679 ns        19169
BM_StdUnorderedMap_Set/1024                           73768 ns        73767 ns         9567
BM_LuaTable_Set/2                                      1251 ns         1255 ns       560620
BM_LuaTable_Set/4                                      1510 ns         1514 ns       459308
BM_LuaTable_Set/8                                      1993 ns         1997 ns       346243
BM_LuaTable_Set/16                                     2652 ns         2657 ns       262676
BM_LuaTable_Set/32                                     4118 ns         4132 ns       162074
BM_LuaTable_Set/64                                     6377 ns         6400 ns       117134
BM_LuaTable_Set/128                                   10690 ns        10726 ns        65124
BM_LuaTable_Set/256                                   18459 ns        18500 ns        38159
BM_LuaTable_Set/512                                   34007 ns        34067 ns        20437
BM_LuaTable_Set/1024                                  64125 ns        64212 ns        10991
BM_VarTable_Get/2                                      28.5 ns         28.5 ns     24561878
BM_VarTable_Get/4                                      61.3 ns         61.3 ns     11712316
BM_VarTable_Get/8                                       127 ns          127 ns      5324633
BM_VarTable_Get/16                                      228 ns          228 ns      3075945
BM_VarTable_Get/32                                      442 ns          442 ns      1571544
BM_VarTable_Get/64                                      883 ns          883 ns       800586
BM_VarTable_Get/128                                    1771 ns         1771 ns       397782
BM_VarTable_Get/256                                    3552 ns         3549 ns       197796
BM_VarTable_Get/512                                    6988 ns         6988 ns        96341
BM_VarTable_Get/1024                                  14013 ns        14012 ns        50066
BM_StdUnorderedMap_Get/2                               12.8 ns         12.8 ns     50582391
BM_StdUnorderedMap_Get/4                               28.0 ns         28.0 ns     24840686
BM_StdUnorderedMap_Get/8                               59.6 ns         59.5 ns     11706006
BM_StdUnorderedMap_Get/16                               121 ns          121 ns      5800699
BM_StdUnorderedMap_Get/32                               244 ns          244 ns      2867133
BM_StdUnorderedMap_Get/64                               493 ns          493 ns      1428255
BM_StdUnorderedMap_Get/128                              957 ns          956 ns       727902
BM_StdUnorderedMap_Get/256                             1968 ns         1968 ns       356291
BM_StdUnorderedMap_Get/512                             3931 ns         3929 ns       178089
BM_StdUnorderedMap_Get/1024                            7881 ns         7881 ns        87647
BM_LuaTable_Get/2                                      44.1 ns         44.0 ns     15766249
BM_LuaTable_Get/4                                      96.9 ns         96.9 ns      7223734
BM_LuaTable_Get/8                                       186 ns          186 ns      3917553
BM_LuaTable_Get/16                                      351 ns          351 ns      2036531
BM_LuaTable_Get/32                                      689 ns          689 ns      1021100
BM_LuaTable_Get/64                                     1367 ns         1366 ns       518043
BM_LuaTable_Get/128                                    2739 ns         2739 ns       255158
BM_LuaTable_Get/256                                    5482 ns         5479 ns       128007
BM_LuaTable_Get/512                                   10942 ns        10942 ns        63884
BM_LuaTable_Get/1024                                  21695 ns        21694 ns        31974
BM_VarTable_Iter/2                                    0.796 ns        0.796 ns    880914194
BM_VarTable_Iter/4                                     2.78 ns         2.78 ns    251494849
BM_VarTable_Iter/8                                     4.78 ns         4.78 ns    146319383
BM_VarTable_Iter/16                                    11.3 ns         11.3 ns     61698960
BM_VarTable_Iter/32                                    42.8 ns         42.8 ns     16496587
BM_VarTable_Iter/64                                    58.8 ns         58.8 ns     11933966
BM_VarTable_Iter/128                                    104 ns          104 ns      6743082
BM_VarTable_Iter/256                                    205 ns          205 ns      3408167
BM_VarTable_Iter/512                                    422 ns          422 ns      1700224
BM_VarTable_Iter/1024                                   816 ns          816 ns       831574
BM_StdUnorderedMap_Iter/2                              1.97 ns         1.97 ns    356437545
BM_StdUnorderedMap_Iter/4                              3.18 ns         3.18 ns    220352606
BM_StdUnorderedMap_Iter/8                              5.29 ns         5.29 ns    135471266
BM_StdUnorderedMap_Iter/16                             11.9 ns         11.9 ns     59336384
BM_StdUnorderedMap_Iter/32                             62.6 ns         62.6 ns     11050787
BM_StdUnorderedMap_Iter/64                              111 ns          111 ns      6290146
BM_StdUnorderedMap_Iter/128                             215 ns          215 ns      3216423
BM_StdUnorderedMap_Iter/256                             416 ns          416 ns      1680011
BM_StdUnorderedMap_Iter/512                             824 ns          824 ns       852220
BM_StdUnorderedMap_Iter/1024                           1747 ns         1747 ns       399580
BM_LuaTable_Iter/2                                     81.4 ns         81.4 ns      8484127
BM_LuaTable_Iter/4                                      157 ns          157 ns      4376380
BM_LuaTable_Iter/8                                      287 ns          287 ns      2395034
BM_LuaTable_Iter/16                                     548 ns          548 ns      1290851
BM_LuaTable_Iter/32                                    1086 ns         1086 ns       657857
BM_LuaTable_Iter/64                                    2165 ns         2165 ns       314548
BM_LuaTable_Iter/128                                   4170 ns         4166 ns       166517
BM_LuaTable_Iter/256                                   8268 ns         8268 ns        84493
BM_LuaTable_Iter/512                                  16515 ns        16514 ns        42262
BM_LuaTable_Iter/1024                                 33157 ns        33156 ns        21212
BM_VarTable_Del/2                                       495 ns          496 ns      1370043
BM_VarTable_Del/4                                       526 ns          527 ns      1327289
BM_VarTable_Del/8                                       593 ns          594 ns      1172595
BM_VarTable_Del/16                                      763 ns          763 ns       902999
BM_VarTable_Del/32                                     1038 ns         1035 ns       661144
BM_VarTable_Del/64                                     1627 ns         1619 ns       429949
BM_VarTable_Del/128                                    2810 ns         2792 ns       254885
BM_VarTable_Del/256                                    5082 ns         5048 ns       133940
BM_VarTable_Del/512                                    9387 ns         9333 ns        75432
BM_VarTable_Del/1024                                  17007 ns        16935 ns        37694
BM_StdUnorderedMap_Del/2                                532 ns          533 ns      1320072
BM_StdUnorderedMap_Del/4                                573 ns          574 ns      1227566
BM_StdUnorderedMap_Del/8                                668 ns          670 ns      1040362
BM_StdUnorderedMap_Del/16                               912 ns          913 ns       705634
BM_StdUnorderedMap_Del/32                              1297 ns         1297 ns       539219
BM_StdUnorderedMap_Del/64                              2114 ns         2113 ns       342425
BM_StdUnorderedMap_Del/128                             3515 ns         3511 ns       196088
BM_StdUnorderedMap_Del/256                             5574 ns         5569 ns       123150
BM_StdUnorderedMap_Del/512                            10721 ns        10706 ns        65943
BM_StdUnorderedMap_Del/1024                           20915 ns        20895 ns        33313
BM_LuaTable_Del/2                                       979 ns          982 ns       710458
BM_LuaTable_Del/4                                      1040 ns         1042 ns       684759
BM_LuaTable_Del/8                                      1103 ns         1105 ns       618166
BM_LuaTable_Del/16                                     1243 ns         1244 ns       563495
BM_LuaTable_Del/32                                     1557 ns         1548 ns       450458
BM_LuaTable_Del/64                                     2103 ns         2090 ns       335333
BM_LuaTable_Del/128                                    3254 ns         3228 ns       217594
BM_LuaTable_Del/256                                    5442 ns         5410 ns       129967
BM_LuaTable_Del/512                                   10041 ns         9996 ns        71597
BM_LuaTable_Del/1024                                  19170 ns        19080 ns        36883
```

---

## 算法性能分析（C++ vs Lua vs FakeLua TCC vs FakeLua GCC）

以下取各算法的典型参数做横向对比（CPU Time），**倍数 = 对应实现时间 / C++ 时间**：

### 1. Fibonacci（n=32，递归无记忆化）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 8.01 ms | 1x |
| Lua | 414.53 ms | **51.8x** 慢 |
| FakeLua TCC | 439.35 ms | **54.8x** 慢 |
| FakeLua GCC | 26.87 ms | **3.4x** 慢 |

> GCC `-O3` 对递归 Fibonacci 提升约 **16.3x**（相比 TCC），且明显快于 Lua（Lua 的 15.4x）。瓶颈仍在于跨边界调用桥接，但 GCC 已大幅压缩函数调用本身的指令开销。TCC 与 Lua 基本在同一水平（Lua 略快 6%）。

### 2. GCD（a=832040, b=514229，约 30 次迭代）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 173 ns | 1x |
| Lua | 850 ns | **4.9x** 慢 |
| FakeLua TCC | 1273 ns | **7.4x** 慢 |
| FakeLua GCC | 257 ns | **1.5x** 慢 |

> GCC 相比 TCC 提升约 **5.0x**，已大幅快于 Lua（Lua 的 3.3x）。GCC 后端仅为 C++ 的 1.5 倍，接近原生水平。

### 3. PowMod（base=1234567, exp=7654321, mod=1e9+7，约 23 次循环）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 396 ns | 1x |
| Lua | 2115 ns | **5.3x** 慢 |
| FakeLua TCC | 3408 ns | **8.6x** 慢 |
| FakeLua GCC | 538 ns | **1.4x** 慢 |

> GCC 相比 TCC 提升约 **6.3x**，快于 Lua 3.9 倍，接近 C++ 的 1.4x 级别。对于计算密集型循环，GCC 后端几乎与原生 C++ 持平。

### 4. Sum（n=5000000，纯循环累加）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 2.00 ms | 1x |
| Lua | 46.61 ms | **23.4x** 慢 |
| FakeLua TCC | 163.24 ms | **81.8x** 慢 |
| FakeLua GCC | 2.03 ms | **≈1.0x**（与 C++ 持平） |

> Sum 是最震撼的结果：GCC `-O3` 完全内联并向量化循环体，FakeLua GCC 与原生 C++ **完全持平**，相比 TCC 提升约 **80.3x**。

---

## 表操作性能分析（VarTable vs unordered_map vs Lua Table）

取 n=1024 比较：

| 操作 | VarTable | unordered_map | Lua Table | VarTable vs map | VarTable vs Lua |
|------|----------|---------------|-----------|-----------------|-----------------|
| Set  | 89.6 µs | 73.8 µs | 64.2 µs | 1.21x 慢 | 1.40x 慢 |
| Get  | 14.0 µs | 7.9 µs | 21.7 µs | 1.78x 慢 | **1.55x 快** |
| Iter | 0.82 µs | 1.75 µs | 33.2 µs | **2.1x 快** | **40.6x 快** |
| Del  | 16.9 µs | 20.9 µs | 19.1 µs | **1.2x 快** | **1.1x 快** |

> VarTable 的核心优势在 **Iterate** 上（紧密的 active_list 遍历完全 cache-friendly），在 1024 元素时比 unordered_map 快 2.1 倍，比 Lua Table 快 **41 倍**。**Delete** 方面 VarTable 也优于两者。**Get** 方面 VarTable 弱于 map 但快于 Lua Table。**Set** 因分配 heap + active_list 维护略慢，但差距在可接受范围内（小规模 ≤8 时 VarTable 的 quick_data 路径更优）。

---

## FakeLua GCC vs Lua 5.4 详细对比

以下直接对比 FakeLua GCC 后端与 Lua 5.4 解释器的性能（**倍数 = Lua CPU Time / FakeLua GCC CPU Time**）：

### Fibonacci（递归密集型）

| 参数 | Lua | FakeLua GCC | GCC 快多少 |
|------|-----|-------------|-----------|
| n=20 | 1291.0 µs | 76.0 µs | **17.0x** |
| n=25 | 14083.9 µs | 845.6 µs | **16.7x** |
| n=30 | 153.8 ms | 9.72 ms | **15.8x** |
| n=32 | 414.5 ms | 26.9 ms | **15.4x** |

### GCD（短循环迭代）

| 参数 | Lua | FakeLua GCC | GCC 快多少 |
|------|-----|-------------|-----------|
| 832040/514229 | 850 ns | 257 ns | **3.3x** |
| 123456789/987654321 | 192 ns | 96.4 ns | **2.0x** |
| 2147483647/1073741823 | 159 ns | 92.3 ns | **1.7x** |

### PowMod（中等循环+取模）

| 参数 | Lua | FakeLua GCC | GCC 快多少 |
|------|-----|-------------|-----------|
| 2/1000/1e9+7 | 981 ns | 248 ns | **4.0x** |
| 7/1e6/1e9+7 | 1631 ns | 426 ns | **3.8x** |
| 1234567/7654321/1e9+7 | 2115 ns | 538 ns | **3.9x** |

### Sum（纯循环累加）

| 参数 | Lua | FakeLua GCC | GCC 快多少 |
|------|-----|-------------|-----------|
| n=10000 | 94.0 µs | 4.07 µs | **23.1x** |
| n=100000 | 939.0 µs | 39.9 µs | **23.5x** |
| n=1000000 | 9.31 ms | 0.40 ms | **23.3x** |
| n=5000000 | 46.61 ms | 2.03 ms | **23.0x** |

### FakeLua GCC vs Lua 总结

| 场景类型 | FakeLua GCC 比 Lua 快 | 原因 |
|---------|----------------------|------|
| **纯循环累加 (Sum)** | **23–24x** | GCC `-O3` 内联 + 向量化，达到 C++ 原生水平 |
| **递归 (Fibonacci)** | **15–17x** | GCC 优化函数调用开销，指令级提升巨大 |
| **算术循环 (PowMod)** | **3.8–4.0x** | 循环体内取模运算受益于寄存器分配优化 |
| **短迭代 (GCD)** | **1.7–3.3x** | 迭代次数少，函数调用开销占比较高，优势相对小 |

> **FakeLua GCC 后端比 Lua 5.4 快 1.7x ~ 23.5x**，计算越密集（循环越长）优势越大。核心原因是 GCC `-O3` 对生成的 C 代码做了积极的内联、循环展开和向量化，而 Lua 作为解释器每条指令都需要 dispatch 开销。

---

## 总体结论

1. **C++ 最快**：在全部算法上领先，受益于 `-O3` 内联/展开/向量化。
2. **FakeLua GCC 全面超越 Lua，多场景接近 C++ 原生性能**：
   - Fib(32): GCC 26.87ms，仅为 C++ 的 **3.4x**，比 Lua 快 **15.4x**
   - GCD(832040,514229): GCC 257ns，仅为 C++ 的 **1.5x**，比 Lua 快 **3.3x**
   - PowMod(1234567,7654321,1e9+7): GCC 538ns，仅为 C++ 的 **1.4x**，比 Lua 快 **3.9x**
   - Sum(5e6): GCC 2.03ms，与 C++ **完全持平**（≈1.0x），比 Lua 快 **23.0x**

> 注：ASLR 开启，结果有一定随机噪声；建议在 `--cpu-scaling-enabled=false` 环境下多重复（`--benchmark_repetitions=5`）后取均值。
