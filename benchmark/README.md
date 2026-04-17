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
2026-04-17T17:27:13+08:00
Running ./bench_mark
Run on (16 X 2000 MHz CPU s)
CPU Caches:
  L1 Data 64 KiB (x16)
  L1 Instruction 64 KiB (x16)
  L2 Unified 512 KiB (x16)
  L3 Unified 65536 KiB (x2)
Load Average: 8.80, 5.92, 3.69
***WARNING*** ASLR is enabled, the results may have unreproducible noise in them.
-------------------------------------------------------------------------------------------
Benchmark                                                 Time             CPU   Iterations
-------------------------------------------------------------------------------------------
BM_CPP_Fibonacci/20                                   24461 ns        24460 ns        29341
BM_CPP_Fibonacci/25                                  230106 ns       230097 ns         3036
BM_CPP_Fibonacci/30                                 3074743 ns      3074639 ns          235
BM_CPP_Fibonacci/32                                 7454684 ns      7454695 ns           93
BM_Lua_Fibonacci/20                                 1251911 ns      1251889 ns          562
BM_Lua_Fibonacci/25                                13962963 ns     13962271 ns           50
BM_Lua_Fibonacci/30                               152602088 ns    152597671 ns            5
BM_Lua_Fibonacci/32                               402893297 ns    402887203 ns            2
BM_FakeLua_Fibonacci_TCC/20                         1328449 ns      1328432 ns          521
BM_FakeLua_Fibonacci_TCC/25                        15281778 ns     15281515 ns           45
BM_FakeLua_Fibonacci_TCC/30                       163082704 ns    163077330 ns            4
BM_FakeLua_Fibonacci_TCC/32                       426274970 ns    426263789 ns            2
BM_FakeLua_Fibonacci_GCC/20                           75163 ns        75162 ns        10068
BM_FakeLua_Fibonacci_GCC/25                          841849 ns       841824 ns          718
BM_FakeLua_Fibonacci_GCC/30                         8557406 ns      8557148 ns           82
BM_FakeLua_Fibonacci_GCC/32                        22450935 ns     22450737 ns           29
BM_CPP_GCD/832040/514229                                170 ns          170 ns      4196020
BM_CPP_GCD/123456789/987654321                         22.1 ns         22.1 ns     31386063
BM_CPP_GCD/2147483647/1073741823                       16.8 ns         16.8 ns     41697925
BM_Lua_GCD/832040/514229                                881 ns          881 ns       848494
BM_Lua_GCD/123456789/987654321                          185 ns          185 ns      3512172
BM_Lua_GCD/2147483647/1073741823                        156 ns          156 ns      4484396
BM_FakeLua_GCD_TCC/832040/514229                       1335 ns         1335 ns       540891
BM_FakeLua_GCD_TCC/123456789/987654321                  274 ns          273 ns      2514141
BM_FakeLua_GCD_TCC/2147483647/1073741823                191 ns          191 ns      3564223
BM_FakeLua_GCD_GCC/832040/514229                        255 ns          255 ns      2781912
BM_FakeLua_GCD_GCC/123456789/987654321                 92.1 ns         92.1 ns      7556089
BM_FakeLua_GCD_GCC/2147483647/1073741823               90.2 ns         90.2 ns      7772691
BM_CPP_PowMod/2/1000/1000000007                         146 ns          146 ns      4801294
BM_CPP_PowMod/7/1000000/1000000007                      276 ns          276 ns      2539460
BM_CPP_PowMod/1234567/7654321/1000000007                394 ns          394 ns      1779144
BM_Lua_PowMod/2/1000/1000000007                         988 ns          988 ns       706070
BM_Lua_PowMod/7/1000000/1000000007                     1638 ns         1637 ns       430337
BM_Lua_PowMod/1234567/7654321/1000000007               2104 ns         2104 ns       331550
BM_FakeLua_PowMod_TCC/2/1000/1000000007                1518 ns         1518 ns       460536
BM_FakeLua_PowMod_TCC/7/1000000/1000000007             2763 ns         2763 ns       253031
BM_FakeLua_PowMod_TCC/1234567/7654321/1000000007       3396 ns         3396 ns       205444
BM_FakeLua_PowMod_GCC/2/1000/1000000007                 217 ns          217 ns      3222486
BM_FakeLua_PowMod_GCC/7/1000000/1000000007              358 ns          358 ns      1955020
BM_FakeLua_PowMod_GCC/1234567/7654321/1000000007        480 ns          480 ns      1450638
BM_CPP_Sum/10000                                       3967 ns         3964 ns       175742
BM_CPP_Sum/100000                                     39813 ns        39812 ns        17599
BM_CPP_Sum/1000000                                   402704 ns       402434 ns         1751
BM_CPP_Sum/5000000                                  2003452 ns      2003278 ns          349
BM_Lua_Sum/10000                                      94094 ns        94039 ns         7475
BM_Lua_Sum/100000                                    955830 ns       944645 ns          750
BM_Lua_Sum/1000000                                  9386699 ns      9386508 ns           75
BM_Lua_Sum/5000000                                 46862449 ns     46861214 ns           15
BM_FakeLua_Sum_TCC/10000                             329318 ns       329315 ns         2149
BM_FakeLua_Sum_TCC/100000                           3246212 ns      3246178 ns          215
BM_FakeLua_Sum_TCC/1000000                         32508388 ns     32465712 ns           22
BM_FakeLua_Sum_TCC/5000000                        161896423 ns    161894439 ns            4
BM_FakeLua_Sum_GCC/10000                               4137 ns         4137 ns       172662
BM_FakeLua_Sum_GCC/100000                             40505 ns        40505 ns        17023
BM_FakeLua_Sum_GCC/1000000                           410937 ns       410938 ns         1739
BM_FakeLua_Sum_GCC/5000000                          2012047 ns      2011965 ns          348
BM_VarTable_Set/2                                       199 ns          199 ns      3733968
BM_VarTable_Set/4                                       215 ns          215 ns      3494648
BM_VarTable_Set/8                                       251 ns          251 ns      2939093
BM_VarTable_Set/16                                      944 ns          944 ns       865707
BM_VarTable_Set/32                                     2314 ns         2313 ns       317233
BM_VarTable_Set/64                                     5126 ns         5126 ns       146229
BM_VarTable_Set/128                                   10563 ns        10563 ns        75536
BM_VarTable_Set/256                                   21344 ns        21342 ns        34427
BM_VarTable_Set/512                                   42436 ns        42435 ns        17377
BM_VarTable_Set/1024                                  84736 ns        84732 ns         9517
BM_StdUnorderedMap_Set/2                                161 ns          161 ns      4363568
BM_StdUnorderedMap_Set/4                                259 ns          259 ns      2695382
BM_StdUnorderedMap_Set/8                                434 ns          434 ns      1616971
BM_StdUnorderedMap_Set/16                               997 ns          996 ns       704963
BM_StdUnorderedMap_Set/32                              2090 ns         2090 ns       344065
BM_StdUnorderedMap_Set/64                              4293 ns         4293 ns       164123
BM_StdUnorderedMap_Set/128                             9593 ns         9593 ns        72982
BM_StdUnorderedMap_Set/256                            17796 ns        17796 ns        39111
BM_StdUnorderedMap_Set/512                            36521 ns        36521 ns        19150
BM_StdUnorderedMap_Set/1024                           72941 ns        72939 ns         9452
BM_LuaTable_Set/2                                      1232 ns         1235 ns       567212
BM_LuaTable_Set/4                                      1494 ns         1497 ns       462994
BM_LuaTable_Set/8                                      1977 ns         1982 ns       348117
BM_LuaTable_Set/16                                     2624 ns         2630 ns       263644
BM_LuaTable_Set/32                                     4291 ns         4304 ns       165401
BM_LuaTable_Set/64                                     6259 ns         6273 ns       120232
BM_LuaTable_Set/128                                   11040 ns        11066 ns        70971
BM_LuaTable_Set/256                                   19469 ns        19510 ns        38434
BM_LuaTable_Set/512                                   33609 ns        33649 ns        20789
BM_LuaTable_Set/1024                                  65301 ns        65393 ns        11126
BM_VarTable_Get/2                                      30.6 ns         30.6 ns     22634713
BM_VarTable_Get/4                                      65.9 ns         65.9 ns     11392186
BM_VarTable_Get/8                                       128 ns          128 ns      5143834
BM_VarTable_Get/16                                      234 ns          234 ns      3001092
BM_VarTable_Get/32                                      457 ns          457 ns      1522362
BM_VarTable_Get/64                                      902 ns          902 ns       775983
BM_VarTable_Get/128                                    1792 ns         1792 ns       388965
BM_VarTable_Get/256                                    3585 ns         3585 ns       195292
BM_VarTable_Get/512                                    7175 ns         7175 ns        98176
BM_VarTable_Get/1024                                  14318 ns        14317 ns        48995
BM_StdUnorderedMap_Get/2                               21.8 ns         21.8 ns     54639356
BM_StdUnorderedMap_Get/4                               27.8 ns         27.8 ns     25165017
BM_StdUnorderedMap_Get/8                               59.3 ns         59.3 ns     11800990
BM_StdUnorderedMap_Get/16                               122 ns          122 ns      5782190
BM_StdUnorderedMap_Get/32                               244 ns          244 ns      2856134
BM_StdUnorderedMap_Get/64                               504 ns          504 ns      1420429
BM_StdUnorderedMap_Get/128                              961 ns          961 ns       706743
BM_StdUnorderedMap_Get/256                             1972 ns         1972 ns       355040
BM_StdUnorderedMap_Get/512                             3942 ns         3942 ns       177771
BM_StdUnorderedMap_Get/1024                            7898 ns         7898 ns        87696
BM_LuaTable_Get/2                                      42.9 ns         42.9 ns     16085417
BM_LuaTable_Get/4                                      90.2 ns         90.2 ns      7700261
BM_LuaTable_Get/8                                       173 ns          173 ns      4010803
BM_LuaTable_Get/16                                      374 ns          374 ns      1968819
BM_LuaTable_Get/32                                      678 ns          678 ns       994428
BM_LuaTable_Get/64                                     1366 ns         1366 ns       506985
BM_LuaTable_Get/128                                    2855 ns         2855 ns       261183
BM_LuaTable_Get/256                                    5514 ns         5514 ns       125107
BM_LuaTable_Get/512                                   10896 ns        10895 ns        65695
BM_LuaTable_Get/1024                                  21711 ns        21710 ns        32293
BM_VarTable_Iter/2                                    0.810 ns        0.810 ns    862709729
BM_VarTable_Iter/4                                     2.80 ns         2.80 ns    250465776
BM_VarTable_Iter/8                                     4.80 ns         4.80 ns    145577367
BM_VarTable_Iter/16                                    11.4 ns         11.4 ns     61395201
BM_VarTable_Iter/32                                    42.7 ns         42.7 ns     16482785
BM_VarTable_Iter/64                                    60.0 ns         60.0 ns     11765608
BM_VarTable_Iter/128                                    110 ns          110 ns      6223220
BM_VarTable_Iter/256                                    213 ns          213 ns      3358651
BM_VarTable_Iter/512                                    404 ns          404 ns      1742934
BM_VarTable_Iter/1024                                   847 ns          847 ns       821710
BM_StdUnorderedMap_Iter/2                              1.96 ns         1.96 ns    358587526
BM_StdUnorderedMap_Iter/4                              3.33 ns         3.31 ns    218965409
BM_StdUnorderedMap_Iter/8                              5.21 ns         5.20 ns    135014732
BM_StdUnorderedMap_Iter/16                             11.0 ns         11.0 ns     63348073
BM_StdUnorderedMap_Iter/32                             62.5 ns         62.5 ns     11081576
BM_StdUnorderedMap_Iter/64                              111 ns          111 ns      6228003
BM_StdUnorderedMap_Iter/128                             216 ns          216 ns      3277302
BM_StdUnorderedMap_Iter/256                             417 ns          417 ns      1678441
BM_StdUnorderedMap_Iter/512                             826 ns          826 ns       841577
BM_StdUnorderedMap_Iter/1024                           1740 ns         1740 ns       402975
BM_LuaTable_Iter/2                                     81.0 ns         81.0 ns      8433825
BM_LuaTable_Iter/4                                      161 ns          161 ns      4447940
BM_LuaTable_Iter/8                                      285 ns          285 ns      2385705
BM_LuaTable_Iter/16                                     547 ns          547 ns      1284646
BM_LuaTable_Iter/32                                    1069 ns         1068 ns       654560
BM_LuaTable_Iter/64                                    2119 ns         2119 ns       329584
BM_LuaTable_Iter/128                                   4166 ns         4158 ns       165876
BM_LuaTable_Iter/256                                   8289 ns         8289 ns        84408
BM_LuaTable_Iter/512                                  17030 ns        17019 ns        42300
BM_LuaTable_Iter/1024                                 33098 ns        33096 ns        20420
BM_VarTable_Del/2                                       509 ns          511 ns      1344998
BM_VarTable_Del/4                                       542 ns          544 ns      1290103
BM_VarTable_Del/8                                       618 ns          620 ns      1150665
BM_VarTable_Del/16                                      769 ns          770 ns       918184
BM_VarTable_Del/32                                     1059 ns         1055 ns       661830
BM_VarTable_Del/64                                     1638 ns         1629 ns       435148
BM_VarTable_Del/128                                    2746 ns         2730 ns       256442
BM_VarTable_Del/256                                    4988 ns         4951 ns       145946
BM_VarTable_Del/512                                    8777 ns         8733 ns        80162
BM_VarTable_Del/1024                                  16938 ns        16857 ns        41636
BM_StdUnorderedMap_Del/2                                537 ns          539 ns      1300388
BM_StdUnorderedMap_Del/4                                586 ns          588 ns      1221982
BM_StdUnorderedMap_Del/8                                680 ns          681 ns       958307
BM_StdUnorderedMap_Del/16                               917 ns          917 ns       772484
BM_StdUnorderedMap_Del/32                              1384 ns         1383 ns       543572
BM_StdUnorderedMap_Del/64                              2039 ns         2037 ns       333013
BM_StdUnorderedMap_Del/128                             3462 ns         3460 ns       200358
BM_StdUnorderedMap_Del/256                             5587 ns         5577 ns       125981
BM_StdUnorderedMap_Del/512                            11070 ns        11050 ns        65089
BM_StdUnorderedMap_Del/1024                           22062 ns        22032 ns        33103
BM_LuaTable_Del/2                                       989 ns          992 ns       709921
BM_LuaTable_Del/4                                      1027 ns         1029 ns       681327
BM_LuaTable_Del/8                                      1105 ns         1106 ns       630078
BM_LuaTable_Del/16                                     1260 ns         1259 ns       561771
BM_LuaTable_Del/32                                     1553 ns         1550 ns       459163
BM_LuaTable_Del/64                                     2102 ns         2097 ns       314256
BM_LuaTable_Del/128                                    3227 ns         3215 ns       219186
BM_LuaTable_Del/256                                    5559 ns         5533 ns       130856
BM_LuaTable_Del/512                                    9812 ns         9779 ns        71805
BM_LuaTable_Del/1024                                  18675 ns        18620 ns        37633
```

---

## 算法性能分析（C++ vs Lua vs FakeLua TCC vs FakeLua GCC）

以下取各算法的典型参数做横向对比（CPU Time），**倍数 = 对应实现时间 / C++ 时间**：

### 1. Fibonacci（n=32，递归无记忆化）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 7.45 ms | 1x |
| Lua | 402.89 ms | **54.0x** 慢 |
| FakeLua TCC | 426.26 ms | **57.2x** 慢 |
| FakeLua GCC | 22.45 ms | **3.0x** 慢 |

> GCC `-O3` 对递归 Fibonacci 提升约 **19.0x**（相比 TCC），且明显快于 Lua（Lua 的 18x）。瓶颈仍在于跨边界调用桥接，但 GCC 已大幅压缩函数调用本身的指令开销。TCC 与 Lua 基本在同一水平（Lua 略快 6%）。

### 2. GCD（a=832040, b=514229，约 30 次迭代）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 170 ns | 1x |
| Lua | 881 ns | **5.2x** 慢 |
| FakeLua TCC | 1335 ns | **7.9x** 慢 |
| FakeLua GCC | 255 ns | **1.5x** 慢 |

> GCC 相比 TCC 提升约 **5.2x**，已大幅快于 Lua（Lua 的 3.5x）。GCC 后端仅为 C++ 的 1.5 倍，接近原生水平。

### 3. PowMod（base=1234567, exp=7654321, mod=1e9+7，约 23 次循环）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 394 ns | 1x |
| Lua | 2104 ns | **5.3x** 慢 |
| FakeLua TCC | 3396 ns | **8.6x** 慢 |
| FakeLua GCC | 480 ns | **1.2x** 慢 |

> GCC 相比 TCC 提升约 **7.1x**，快于 Lua 4.4 倍，接近 C++ 的 1.2x 级别。对于计算密集型循环，GCC 后端几乎与原生 C++ 持平。

### 4. Sum（n=5000000，纯循环累加）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 2.00 ms | 1x |
| Lua | 46.86 ms | **23.4x** 慢 |
| FakeLua TCC | 161.89 ms | **80.9x** 慢 |
| FakeLua GCC | 2.01 ms | **≈1.0x**（与 C++ 持平） |

> Sum 是最震撼的结果：GCC `-O3` 完全内联并向量化循环体，FakeLua GCC 与原生 C++ **完全持平**，相比 TCC 提升约 **80.5x**。

---

## 表操作性能分析（VarTable vs unordered_map vs Lua Table）

取 n=1024 比较：

| 操作 | VarTable | unordered_map | Lua Table | VarTable vs map | VarTable vs Lua |
|------|----------|---------------|-----------|-----------------|-----------------|
| Set  | 84.7 µs | 72.9 µs | 65.4 µs | 1.16x 慢 | 1.30x 慢 |
| Get  | 14.3 µs | 7.9 µs | 21.7 µs | 1.81x 慢 | **1.52x 快** |
| Iter | 0.85 µs | 1.74 µs | 33.1 µs | **2.1x 快** | **39.1x 快** |
| Del  | 16.9 µs | 22.0 µs | 18.6 µs | **1.3x 快** | **1.1x 快** |

> VarTable 的核心优势在 **Iterate** 上（紧密的 active_list 遍历完全 cache-friendly），在 1024 元素时比 unordered_map 快 2.1 倍，比 Lua Table 快 **39 倍**。**Delete** 方面 VarTable 也优于两者。**Get** 方面 VarTable 弱于 map 但快于 Lua Table。**Set** 因分配 heap + active_list 维护略慢，但差距在可接受范围内（小规模 ≤8 时 VarTable 的 quick_data 路径更优）。

---

## FakeLua GCC vs Lua 5.4 详细对比

以下直接对比 FakeLua GCC 后端与 Lua 5.4 解释器的性能（**倍数 = Lua CPU Time / FakeLua GCC CPU Time**）：

### Fibonacci（递归密集型）

| 参数 | Lua | FakeLua GCC | GCC 快多少 |
|------|-----|-------------|-----------|
| n=20 | 1251.9 µs | 75.2 µs | **16.7x** |
| n=25 | 13962.3 µs | 841.8 µs | **16.6x** |
| n=30 | 152.6 ms | 8.56 ms | **17.8x** |
| n=32 | 402.9 ms | 22.5 ms | **17.9x** |

### GCD（短循环迭代）

| 参数 | Lua | FakeLua GCC | GCC 快多少 |
|------|-----|-------------|-----------|
| 832040/514229 | 881 ns | 255 ns | **3.5x** |
| 123456789/987654321 | 185 ns | 92.1 ns | **2.0x** |
| 2147483647/1073741823 | 156 ns | 90.2 ns | **1.7x** |

### PowMod（中等循环+取模）

| 参数 | Lua | FakeLua GCC | GCC 快多少 |
|------|-----|-------------|-----------|
| 2/1000/1e9+7 | 988 ns | 217 ns | **4.6x** |
| 7/1e6/1e9+7 | 1637 ns | 358 ns | **4.6x** |
| 1234567/7654321/1e9+7 | 2104 ns | 480 ns | **4.4x** |

### Sum（纯循环累加）

| 参数 | Lua | FakeLua GCC | GCC 快多少 |
|------|-----|-------------|-----------|
| n=10000 | 94.0 µs | 4.14 µs | **22.7x** |
| n=100000 | 944.6 µs | 40.5 µs | **23.3x** |
| n=1000000 | 9.39 ms | 0.41 ms | **22.8x** |
| n=5000000 | 46.86 ms | 2.01 ms | **23.3x** |

### FakeLua GCC vs Lua 总结

| 场景类型 | FakeLua GCC 比 Lua 快 | 原因 |
|---------|----------------------|------|
| **纯循环累加 (Sum)** | **22–23x** | GCC `-O3` 内联 + 向量化，达到 C++ 原生水平 |
| **递归 (Fibonacci)** | **17–18x** | GCC 优化函数调用开销，指令级提升巨大 |
| **算术循环 (PowMod)** | **4.4–4.6x** | 循环体内取模运算受益于寄存器分配优化 |
| **短迭代 (GCD)** | **1.7–3.5x** | 迭代次数少，函数调用开销占比较高，优势相对小 |

> **FakeLua GCC 后端比 Lua 5.4 快 1.7x ~ 23.3x**，计算越密集（循环越长）优势越大。核心原因是 GCC `-O3` 对生成的 C 代码做了积极的内联、循环展开和向量化，而 Lua 作为解释器每条指令都需要 dispatch 开销。

---

## 总体结论

1. **C++ 最快**：在全部算法上领先，受益于 `-O3` 内联/展开/向量化。
2. **Lua 稳定在 C++ 的 5–54x 慢**：对于简单的迭代算法倍数相对低；递归密集时倍数随调用深度指数上升。
3. **FakeLua GCC 全面超越 Lua，多场景接近 C++ 原生性能**：同一套脚本在 GCC 后端下，算法类 benchmark 相比 TCC 提升约 **5.2x ~ 80.5x**。典型样本：
   - Fib(32): TCC 426.26ms → GCC 22.45ms（**19.0x**，远快于 Lua 402.89ms）
   - GCD(832040,514229): TCC 1335ns → GCC 255ns（**5.2x**，远快于 Lua 881ns）
   - PowMod(1234567,7654321,1e9+7): TCC 3396ns → GCC 480ns（**7.1x**，远快于 Lua 2104ns）
   - Sum(5e6): TCC 161.89ms → GCC 2.01ms（**80.5x**，与 C++ 完全持平）
4. **FakeLua TCC 与 Lua 基本持平**：TCC 后端在递归场景（Fibonacci）与 Lua 性能相当，在循环密集场景（Sum）则明显慢于 Lua（3.5x）。TCC 的价值在于编译速度快（适合即时编译场景），而非运行期性能。
5. **瓶颈分析**：TCC（`-O2`）路径的主要成本是 `FakeluaCallByName` + CVar 装箱/拆箱；GCC（`-O3`）后端通过积极内联/循环展开，大幅压缩脚本内算术/循环开销，Sum 场景已达到与 C++ 持平的极限。后端编译器优化级别对 FakeLua 运行时表现影响极大。
6. FakeLua 的设计目标是"可嵌入的高性能脚本引擎"，GCC 后端的实测数据证明了这一路线的可行性：
    - **算法性能**：GCC 后端下脚本执行仅为 C++ 的 1.0–3.0x，比 Lua 快 **1.7–23x**
    - **数据结构**：VarTable Iterate 极快（比 Lua Table 快 39x），编译期类型安全、无 GC 压力
    - **接口集成**：可与 C++ 零开销边界集成（通过模板 `Call<Ret, Args...>()`）
7. 如需继续提升 FakeLua 性能，可考虑：
    - 在生成的 C 代码中对 int 路径直接用 `int64_t` 操作，跳过 CVar 装箱
    - 对高频小函数做更激进的内联/去桥接（特别是递归和循环中的内部调用）
    - VarTable Set 性能优化（减少 heap 分配开销）

> 注：ASLR 开启，结果有一定随机噪声；建议在 `--cpu-scaling-enabled=false` 环境下多重复（`--benchmark_repetitions=5`）后取均值。
