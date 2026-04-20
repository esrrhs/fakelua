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

- 日期：2026-04-20
- 机器：4 vCPU @ 3244.02 MHz
- CPU 缓存：L1d 32 KiB (x2)，L1i 32 KiB (x2)，L2 512 KiB (x2)，L3 32768 KiB (x1)
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
2026-04-20T11:31:25+00:00
Running ./bench_mark
Run on (4 X 3244.02 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x2)
  L1 Instruction 32 KiB (x2)
  L2 Unified 512 KiB (x2)
  L3 Unified 32768 KiB (x1)
Load Average: 1.60, 1.38, 0.60
***WARNING*** ASLR is enabled, the results may have unreproducible noise in them.
-------------------------------------------------------------------------------------------
Benchmark                                                 Time             CPU   Iterations
-------------------------------------------------------------------------------------------
BM_CPP_Fibonacci/20                                   14707 ns        14704 ns        47601
BM_CPP_Fibonacci/25                                  145959 ns       145939 ns         4796
BM_CPP_Fibonacci/30                                 1782594 ns      1782400 ns          398
BM_CPP_Fibonacci/32                                 4491205 ns      4490949 ns          152
BM_Lua_Fibonacci/20                                  439019 ns       438947 ns         1590
BM_Lua_Fibonacci/25                                 4856619 ns      4856321 ns          143
BM_Lua_Fibonacci/30                                53859800 ns     53854017 ns           13
BM_Lua_Fibonacci/32                               140950346 ns    140940354 ns            5
BM_FakeLua_Fibonacci_TCC/20                         1011799 ns      1011733 ns          692
BM_FakeLua_Fibonacci_TCC/25                        11217400 ns     11217300 ns           62
BM_FakeLua_Fibonacci_TCC/30                       124640518 ns    124631853 ns            6
BM_FakeLua_Fibonacci_TCC/32                       325647852 ns    325641592 ns            2
BM_FakeLua_Fibonacci_GCC/20                           61603 ns        61600 ns        11321
BM_FakeLua_Fibonacci_GCC/25                          682649 ns       682581 ns         1027
BM_FakeLua_Fibonacci_GCC/30                         7566023 ns      7565831 ns           88
BM_FakeLua_Fibonacci_GCC/32                        19841575 ns     19837003 ns           35
BM_CPP_GCD/832040/514229                               61.0 ns         61.0 ns     11462112
BM_CPP_GCD/123456789/987654321                         6.54 ns         6.54 ns    107053907
BM_CPP_GCD/2147483647/1073741823                       4.38 ns         4.38 ns    160592052
BM_Lua_GCD/832040/514229                                358 ns          357 ns      1956381
BM_Lua_GCD/123456789/987654321                         85.7 ns         85.7 ns      7980638
BM_Lua_GCD/2147483647/1073741823                       76.5 ns         76.5 ns      9150943
BM_FakeLua_GCD_TCC/832040/514229                        993 ns          993 ns       708942
BM_FakeLua_GCD_TCC/123456789/987654321                  161 ns          161 ns      4352740
BM_FakeLua_GCD_TCC/2147483647/1073741823                129 ns          129 ns      5298434
BM_FakeLua_GCD_GCC/832040/514229                        150 ns          150 ns      4686388
BM_FakeLua_GCD_GCC/123456789/987654321                 59.8 ns         59.8 ns     11685764
BM_FakeLua_GCD_GCC/2147483647/1073741823               58.3 ns         58.3 ns     12005621
BM_CPP_PowMod/2/1000/1000000007                        37.7 ns         37.7 ns     18574978
BM_CPP_PowMod/7/1000000/1000000007                     67.6 ns         67.6 ns     10372970
BM_CPP_PowMod/1234567/7654321/1000000007               91.2 ns         91.2 ns      7666225
BM_Lua_PowMod/2/1000/1000000007                         276 ns          276 ns      2621796
BM_Lua_PowMod/7/1000000/1000000007                      466 ns          466 ns      1493893
BM_Lua_PowMod/1234567/7654321/1000000007                554 ns          554 ns      1261924
BM_FakeLua_PowMod_TCC/2/1000/1000000007                1161 ns         1161 ns       603473
BM_FakeLua_PowMod_TCC/7/1000000/1000000007             2186 ns         2185 ns       320095
BM_FakeLua_PowMod_TCC/1234567/7654321/1000000007       2641 ns         2641 ns       265039
BM_FakeLua_PowMod_GCC/2/1000/1000000007                 103 ns          103 ns      6762184
BM_FakeLua_PowMod_GCC/7/1000000/1000000007              164 ns          164 ns      4255644
BM_FakeLua_PowMod_GCC/1234567/7654321/1000000007        199 ns          199 ns      3516902
BM_CPP_Sum/10000                                       3110 ns         3110 ns       224916
BM_CPP_Sum/100000                                     31114 ns        31111 ns        22481
BM_CPP_Sum/1000000                                   311144 ns       311113 ns         2249
BM_CPP_Sum/5000000                                  1556013 ns      1555891 ns          450
BM_Lua_Sum/10000                                      46792 ns        46787 ns        14948
BM_Lua_Sum/100000                                    501695 ns       501653 ns         1498
BM_Lua_Sum/1000000                                  5951463 ns      5950908 ns          118
BM_Lua_Sum/5000000                                 29578910 ns     29575819 ns           24
BM_FakeLua_Sum_TCC/10000                             299144 ns       299124 ns         2348
BM_FakeLua_Sum_TCC/100000                           2970139 ns      2969705 ns          236
BM_FakeLua_Sum_TCC/1000000                         29726898 ns     29725333 ns           24
BM_FakeLua_Sum_TCC/5000000                        147991132 ns    147976266 ns            5
BM_FakeLua_Sum_GCC/10000                               3156 ns         3156 ns       221751
BM_FakeLua_Sum_GCC/100000                             31180 ns        31177 ns        22462
BM_FakeLua_Sum_GCC/1000000                           311351 ns       311334 ns         2250
BM_FakeLua_Sum_GCC/5000000                          1556656 ns      1556528 ns          450
BM_VarTable_Set/2                                       173 ns          173 ns      3988606
BM_VarTable_Set/4                                       186 ns          186 ns      3728614
BM_VarTable_Set/8                                       224 ns          224 ns      3168403
BM_VarTable_Set/16                                      851 ns          851 ns       860558
BM_VarTable_Set/32                                     2106 ns         2105 ns       326589
BM_VarTable_Set/64                                     4610 ns         4610 ns       150017
BM_VarTable_Set/128                                    9547 ns         9546 ns        78225
BM_VarTable_Set/256                                   19320 ns        19317 ns        36049
BM_VarTable_Set/512                                   38733 ns        38731 ns        17868
BM_VarTable_Set/1024                                  78059 ns        78045 ns         9547
BM_StdUnorderedMap_Set/2                               74.5 ns         74.5 ns      9434136
BM_StdUnorderedMap_Set/4                                112 ns          112 ns      6249336
BM_StdUnorderedMap_Set/8                                211 ns          211 ns      3326993
BM_StdUnorderedMap_Set/16                               445 ns          445 ns      1572371
BM_StdUnorderedMap_Set/32                               967 ns          967 ns       720493
BM_StdUnorderedMap_Set/64                              2003 ns         2003 ns       358002
BM_StdUnorderedMap_Set/128                             3900 ns         3899 ns       178599
BM_StdUnorderedMap_Set/256                             8992 ns         8990 ns        77915
BM_StdUnorderedMap_Set/512                            20240 ns        20238 ns        34581
BM_StdUnorderedMap_Set/1024                           42662 ns        42656 ns        16510
BM_LuaTable_Set/2                                      1538 ns         1546 ns       451901
BM_LuaTable_Set/4                                      1662 ns         1673 ns       425286
BM_LuaTable_Set/8                                      1897 ns         1908 ns       373992
BM_LuaTable_Set/16                                     2334 ns         2345 ns       303900
BM_LuaTable_Set/32                                     3095 ns         3109 ns       232830
BM_LuaTable_Set/64                                     4524 ns         4539 ns       159924
BM_LuaTable_Set/128                                    7841 ns         7856 ns        97616
BM_LuaTable_Set/256                                   14047 ns        14074 ns        54139
BM_LuaTable_Set/512                                   25803 ns        25837 ns        29517
BM_LuaTable_Set/1024                                  48063 ns        48107 ns        14919
BM_VarTable_Get/2                                      19.3 ns         19.3 ns     36190878
BM_VarTable_Get/4                                      39.1 ns         39.1 ns     17917159
BM_VarTable_Get/8                                      82.6 ns         82.6 ns      8475075
BM_VarTable_Get/16                                      161 ns          161 ns      4359514
BM_VarTable_Get/32                                      325 ns          325 ns      2152103
BM_VarTable_Get/64                                      646 ns          646 ns      1086905
BM_VarTable_Get/128                                    1282 ns         1282 ns       545824
BM_VarTable_Get/256                                    2568 ns         2568 ns       273620
BM_VarTable_Get/512                                    5110 ns         5109 ns       137182
BM_VarTable_Get/1024                                  10208 ns        10207 ns        68391
BM_StdUnorderedMap_Get/2                               4.36 ns         4.36 ns    160542164
BM_StdUnorderedMap_Get/4                               9.04 ns         9.04 ns     79558280
BM_StdUnorderedMap_Get/8                               17.6 ns         17.6 ns     39809112
BM_StdUnorderedMap_Get/16                              35.0 ns         35.0 ns     19961242
BM_StdUnorderedMap_Get/32                              69.9 ns         69.9 ns     10019885
BM_StdUnorderedMap_Get/64                               141 ns          140 ns      4998870
BM_StdUnorderedMap_Get/128                              282 ns          282 ns      2495413
BM_StdUnorderedMap_Get/256                              561 ns          561 ns      1250530
BM_StdUnorderedMap_Get/512                             1134 ns         1134 ns       618952
BM_StdUnorderedMap_Get/1024                            2274 ns         2274 ns       300575
BM_LuaTable_Get/2                                      36.8 ns         36.8 ns     19046080
BM_LuaTable_Get/4                                      73.9 ns         73.9 ns      9486907
BM_LuaTable_Get/8                                       146 ns          146 ns      4783066
BM_LuaTable_Get/16                                      303 ns          303 ns      2310624
BM_LuaTable_Get/32                                      592 ns          592 ns      1182088
BM_LuaTable_Get/64                                     1170 ns         1170 ns       599392
BM_LuaTable_Get/128                                    2324 ns         2324 ns       301071
BM_LuaTable_Get/256                                    4640 ns         4640 ns       151020
BM_LuaTable_Get/512                                    9258 ns         9257 ns        75586
BM_LuaTable_Get/1024                                  18567 ns        18567 ns        37813
BM_VarTable_Iter/2                                    0.934 ns        0.934 ns    749595470
BM_VarTable_Iter/4                                     1.94 ns         1.94 ns    364327780
BM_VarTable_Iter/8                                     2.82 ns         2.82 ns    249793352
BM_VarTable_Iter/16                                    9.96 ns         9.96 ns     70249892
BM_VarTable_Iter/32                                    22.0 ns         22.0 ns     31374963
BM_VarTable_Iter/64                                    40.8 ns         40.8 ns     17148721
BM_VarTable_Iter/128                                   85.3 ns         85.3 ns      8170918
BM_VarTable_Iter/256                                    165 ns          165 ns      4236520
BM_VarTable_Iter/512                                    325 ns          325 ns      2158189
BM_VarTable_Iter/1024                                   644 ns          644 ns      1088690
BM_StdUnorderedMap_Iter/2                             0.624 ns        0.624 ns   1124659744
BM_StdUnorderedMap_Iter/4                              1.52 ns         1.52 ns    462223586
BM_StdUnorderedMap_Iter/8                              2.79 ns         2.79 ns    250315204
BM_StdUnorderedMap_Iter/16                             6.44 ns         6.44 ns    108412524
BM_StdUnorderedMap_Iter/32                             16.9 ns         16.9 ns     41339755
BM_StdUnorderedMap_Iter/64                             45.1 ns         45.1 ns     15476974
BM_StdUnorderedMap_Iter/128                             166 ns          166 ns      4221241
BM_StdUnorderedMap_Iter/256                             325 ns          325 ns      2151743
BM_StdUnorderedMap_Iter/512                             644 ns          644 ns      1084440
BM_StdUnorderedMap_Iter/1024                           1571 ns         1571 ns       442087
BM_LuaTable_Iter/2                                     65.4 ns         65.4 ns     10683925
BM_LuaTable_Iter/4                                      121 ns          121 ns      5807814
BM_LuaTable_Iter/8                                      229 ns          229 ns      3055086
BM_LuaTable_Iter/16                                     456 ns          456 ns      1512711
BM_LuaTable_Iter/32                                     890 ns          890 ns       786436
BM_LuaTable_Iter/64                                    1805 ns         1803 ns       398695
BM_LuaTable_Iter/128                                   3492 ns         3492 ns       200386
BM_LuaTable_Iter/256                                   6960 ns         6959 ns       100640
BM_LuaTable_Iter/512                                  13904 ns        13902 ns        50132
BM_LuaTable_Iter/1024                                 27797 ns        27793 ns        25195
BM_VarTable_Del/2                                       666 ns          662 ns      1060797
BM_VarTable_Del/4                                       685 ns          681 ns      1027638
BM_VarTable_Del/8                                       728 ns          726 ns       963498
BM_VarTable_Del/16                                      858 ns          847 ns       844134
BM_VarTable_Del/32                                     1080 ns         1055 ns       685653
BM_VarTable_Del/64                                     1517 ns         1491 ns       493280
BM_VarTable_Del/128                                    2297 ns         2261 ns       308558
BM_VarTable_Del/256                                    3654 ns         3621 ns       194133
BM_VarTable_Del/512                                    6453 ns         6426 ns       108930
BM_VarTable_Del/1024                                  11884 ns        11845 ns        59144
BM_StdUnorderedMap_Del/2                                672 ns          672 ns      1040916
BM_StdUnorderedMap_Del/4                                692 ns          692 ns      1010409
BM_StdUnorderedMap_Del/8                                740 ns          740 ns       946118
BM_StdUnorderedMap_Del/16                               919 ns          918 ns       760982
BM_StdUnorderedMap_Del/32                              1243 ns         1244 ns       562699
BM_StdUnorderedMap_Del/64                              1860 ns         1860 ns       376886
BM_StdUnorderedMap_Del/128                             3252 ns         3248 ns       215725
BM_StdUnorderedMap_Del/256                             4725 ns         4716 ns       148156
BM_StdUnorderedMap_Del/512                             8782 ns         8763 ns        80164
BM_StdUnorderedMap_Del/1024                           17046 ns        17033 ns        41079
BM_LuaTable_Del/2                                      1322 ns         1316 ns       531875
BM_LuaTable_Del/4                                      1354 ns         1348 ns       515630
BM_LuaTable_Del/8                                      1411 ns         1404 ns       499202
BM_LuaTable_Del/16                                     1533 ns         1525 ns       459276
BM_LuaTable_Del/32                                     1758 ns         1748 ns       400963
BM_LuaTable_Del/64                                     2213 ns         2201 ns       319841
BM_LuaTable_Del/128                                    3117 ns         3096 ns       226621
BM_LuaTable_Del/256                                    4882 ns         4860 ns       144259
BM_LuaTable_Del/512                                    8463 ns         8429 ns        82941
BM_LuaTable_Del/1024                                  15554 ns        15517 ns        45173
```

---

## 算法性能分析（C++ vs Lua vs FakeLua TCC vs FakeLua GCC）

以下取各算法的典型参数做横向对比（CPU Time），**倍数 = 对应实现时间 / C++ 时间**：

### 1. Fibonacci（n=32，递归无记忆化）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 4.49 ms | 1x |
| Lua | 140.94 ms | **31.4x** 慢 |
| FakeLua TCC | 325.64 ms | **72.5x** 慢 |
| FakeLua GCC | 19.84 ms | **4.4x** 慢 |

> GCC `-O3` 对递归 Fibonacci 提升约 **16.4x**（相比 TCC），且明显快于 Lua（Lua 的 7.1x）。瓶颈仍在于跨边界调用桥接，但 GCC 已大幅压缩函数调用本身的指令开销。TCC 比 Lua 慢约 2.3 倍，说明 TCC 的低优化级别在深度递归上开销更大。

### 2. GCD（a=832040, b=514229，约 30 次迭代）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 61.0 ns | 1x |
| Lua | 357 ns | **5.9x** 慢 |
| FakeLua TCC | 993 ns | **16.3x** 慢 |
| FakeLua GCC | 150 ns | **2.5x** 慢 |

> GCC 相比 TCC 提升约 **6.6x**，已明显快于 Lua（Lua 的 2.4x）。GCC 后端仅为 C++ 的 2.5 倍，接近原生水平。TCC 在短循环场景开销较大，落后 Lua 约 2.8x。

### 3. PowMod（base=1234567, exp=7654321, mod=1e9+7，约 23 次循环）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 91.2 ns | 1x |
| Lua | 554 ns | **6.1x** 慢 |
| FakeLua TCC | 2641 ns | **29.0x** 慢 |
| FakeLua GCC | 199 ns | **2.2x** 慢 |

> GCC 相比 TCC 提升约 **13.3x**，快于 Lua 2.8 倍，接近 C++ 的 2.2x 级别。对于计算密集型循环，GCC 后端表现优秀。TCC 在本场景开销显著，约为 Lua 的 4.8x。

### 4. Sum（n=5000000，纯循环累加）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 1.556 ms | 1x |
| Lua | 29.576 ms | **19.0x** 慢 |
| FakeLua TCC | 147.976 ms | **95.1x** 慢 |
| FakeLua GCC | 1.557 ms | **≈1.0x**（与 C++ 完全持平） |

> Sum 是最震撼的结果：GCC `-O3` 完全内联并向量化循环体，FakeLua GCC 与原生 C++ **完全持平**，相比 TCC 提升约 **95.1x**，比 Lua 快约 **19.0x**。

---

## 表操作性能分析（VarTable vs unordered_map vs Lua Table）

取 n=1024 比较：

| 操作 | VarTable | unordered_map | Lua Table | VarTable vs map | VarTable vs Lua |
|------|----------|---------------|-----------|-----------------|-----------------|
| Set  | 78.0 µs | 42.7 µs | 48.1 µs | 1.83x 慢 | 1.62x 慢 |
| Get  | 10.2 µs | 2.3 µs | 18.6 µs | 4.43x 慢 | **1.82x 快** |
| Iter | 0.644 µs | 1.571 µs | 27.793 µs | **2.44x 快** | **43.2x 快** |
| Del  | 11.8 µs | 17.0 µs | 15.5 µs | **1.43x 快** | **1.31x 快** |

> VarTable 的核心优势在 **Iterate** 上（紧密的 active_list 遍历完全 cache-friendly），在 1024 元素时比 unordered_map 快 2.44 倍，比 Lua Table 快 **43 倍**。**Delete** 和 **Get** 方面 VarTable 也优于 Lua Table。**Set** 因分配 heap + active_list 维护略慢，但差距在可接受范围内（小规模 ≤8 时 VarTable 的 quick_data 路径更优）。**Get** 方面 VarTable 弱于 `unordered_map`（后者哈希查找常数更小）。

---

## FakeLua GCC vs Lua 5.4 详细对比

以下直接对比 FakeLua GCC 后端与 Lua 5.4 解释器的性能（**倍数 = Lua CPU Time / FakeLua GCC CPU Time**）：

### Fibonacci（递归密集型）

| 参数 | Lua | FakeLua GCC | GCC 快多少 |
|------|-----|-------------|-----------|
| n=20 | 438.9 µs | 61.6 µs | **7.1x** |
| n=25 | 4856.3 µs | 682.6 µs | **7.1x** |
| n=30 | 53.85 ms | 7.57 ms | **7.1x** |
| n=32 | 140.94 ms | 19.84 ms | **7.1x** |

### GCD（短循环迭代）

| 参数 | Lua | FakeLua GCC | GCC 快多少 |
|------|-----|-------------|-----------|
| 832040/514229 | 357 ns | 150 ns | **2.4x** |
| 123456789/987654321 | 85.7 ns | 59.8 ns | **1.4x** |
| 2147483647/1073741823 | 76.5 ns | 58.3 ns | **1.3x** |

### PowMod（中等循环+取模）

| 参数 | Lua | FakeLua GCC | GCC 快多少 |
|------|-----|-------------|-----------|
| 2/1000/1e9+7 | 276 ns | 103 ns | **2.7x** |
| 7/1e6/1e9+7 | 466 ns | 164 ns | **2.8x** |
| 1234567/7654321/1e9+7 | 554 ns | 199 ns | **2.8x** |

### Sum（纯循环累加）

| 参数 | Lua | FakeLua GCC | GCC 快多少 |
|------|-----|-------------|-----------|
| n=10000 | 46.8 µs | 3.16 µs | **14.8x** |
| n=100000 | 501.7 µs | 31.2 µs | **16.1x** |
| n=1000000 | 5.951 ms | 0.311 ms | **19.1x** |
| n=5000000 | 29.576 ms | 1.557 ms | **19.0x** |

### FakeLua GCC vs Lua 总结

| 场景类型 | FakeLua GCC 比 Lua 快 | 原因 |
|---------|----------------------|------|
| **纯循环累加 (Sum)** | **14.8–19.1x** | GCC `-O3` 内联 + 向量化，达到 C++ 原生水平 |
| **递归 (Fibonacci)** | **≈7.1x** | GCC 优化函数调用开销，指令级提升显著 |
| **算术循环 (PowMod)** | **2.7–2.8x** | 循环体内取模运算受益于寄存器分配优化 |
| **短迭代 (GCD)** | **1.3–2.4x** | 迭代次数少，函数调用开销占比较高，优势相对小 |

> **FakeLua GCC 后端比 Lua 5.4 快 1.3x ~ 19.1x**，计算越密集（循环越长）优势越大。核心原因是 GCC `-O3` 对生成的 C 代码做了积极的内联、循环展开和向量化，而 Lua 作为解释器每条指令都需要 dispatch 开销。

---

## 总体结论

1. **C++ 最快**：在全部算法上领先，受益于 `-O3` 内联/展开/向量化。
2. **FakeLua GCC 全面超越 Lua，多场景接近 C++ 原生性能**：
   - Fib(32): GCC 19.84ms，仅为 C++ 的 **4.4x**，比 Lua 快 **7.1x**
   - GCD(832040,514229): GCC 150ns，仅为 C++ 的 **2.5x**，比 Lua 快 **2.4x**
   - PowMod(1234567,7654321,1e9+7): GCC 199ns，仅为 C++ 的 **2.2x**，比 Lua 快 **2.8x**
   - Sum(5e6): GCC 1.557ms，与 C++ **完全持平**（≈1.0x），比 Lua 快 **19.0x**
3. **FakeLua TCC 在本机上慢于 Lua**：TCC 优化级别低（`-O2` 但无向量化/内联），加之动态链接调用开销，在所有算法上均落后于 Lua 5.4，在 Sum 上甚至比 Lua 慢 5x。TCC 的优势主要在于编译速度快，适合开发调试阶段。
4. **VarTable 遍历性能极优**：Iter 操作比 Lua Table 快 43 倍，比 `unordered_map` 快 2.4 倍，核心在于 active_list 的紧凑布局。Del 也优于两者，Get 快于 Lua Table，Set 略慢于两者但差距可接受。

> 注：ASLR 开启，结果有一定随机噪声；建议在 `--cpu-scaling-enabled=false` 环境下多重复（`--benchmark_repetitions=5`）后取均值。
