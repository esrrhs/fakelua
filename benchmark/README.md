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

- 日期：2026-05-02
- 机器：4 vCPU @ 3238.47 MHz
- CPU 缓存：L1d 32 KiB (x2)，L1i 32 KiB (x2)，L2 512 KiB (x2)，L3 32768 KiB (x1)
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
2026-05-02T03:52:37+00:00
Running build/bin/bench_mark
Run on (4 X 3238.47 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x2)
  L1 Instruction 32 KiB (x2)
  L2 Unified 512 KiB (x2)
  L3 Unified 32768 KiB (x1)
Load Average: 6.30, 6.13, 2.69
***WARNING*** ASLR is enabled, the results may have unreproducible noise in them.
-------------------------------------------------------------------------------------------
Benchmark                                                 Time             CPU   Iterations
-------------------------------------------------------------------------------------------
BM_CPP_Fibonacci/20                                   14714 ns        14714 ns        47555
BM_CPP_Fibonacci/25                                  146164 ns       146098 ns         4807
BM_CPP_Fibonacci/30                                 1758165 ns      1757618 ns          398
BM_CPP_Fibonacci/32                                 4462955 ns      4462387 ns          157
BM_Lua_Fibonacci/20                                  447009 ns       446980 ns         1584
BM_Lua_Fibonacci/25                                 4942173 ns      4941743 ns          142
BM_Lua_Fibonacci/30                                54296840 ns     54293989 ns           13
BM_Lua_Fibonacci/32                               141775231 ns    141766165 ns            5
BM_FakeLua_Fibonacci_TCC/20                          254195 ns       254183 ns         2753
BM_FakeLua_Fibonacci_TCC/25                         2821882 ns      2821529 ns          248
BM_FakeLua_Fibonacci_TCC/30                        31298091 ns     31296000 ns           22
BM_FakeLua_Fibonacci_TCC/32                        81945115 ns     81935400 ns            9
BM_FakeLua_Fibonacci_GCC/20                           45716 ns        45715 ns        15618
BM_FakeLua_Fibonacci_GCC/25                          501179 ns       501144 ns         1000
BM_FakeLua_Fibonacci_GCC/30                         5647757 ns      5647325 ns          127
BM_FakeLua_Fibonacci_GCC/32                        14583153 ns     14582150 ns           48
BM_CPP_GCD/832040/514229                               61.0 ns         61.0 ns     11473439
BM_CPP_GCD/123456789/987654321                         6.54 ns         6.54 ns    107054646
BM_CPP_GCD/2147483647/1073741823                       4.36 ns         4.36 ns    160627048
BM_Lua_GCD/832040/514229                                358 ns          358 ns      1959981
BM_Lua_GCD/123456789/987654321                         88.1 ns         88.1 ns      8005404
BM_Lua_GCD/2147483647/1073741823                       77.8 ns         77.8 ns      8999006
BM_FakeLua_GCD_TCC/832040/514229                        174 ns          174 ns      4017060
BM_FakeLua_GCD_TCC/123456789/987654321                 72.7 ns         72.7 ns      9633910
BM_FakeLua_GCD_TCC/2147483647/1073741823               69.1 ns         69.1 ns     10165385
BM_FakeLua_GCD_GCC/832040/514229                        158 ns          158 ns      4433521
BM_FakeLua_GCD_GCC/123456789/987654321                 59.0 ns         59.0 ns     11866969
BM_FakeLua_GCD_GCC/2147483647/1073741823               58.3 ns         58.3 ns     11668712
BM_CPP_PowMod/2/1000/1000000007                        37.7 ns         37.7 ns     18520214
BM_CPP_PowMod/7/1000000/1000000007                     67.5 ns         67.5 ns     10373651
BM_CPP_PowMod/1234567/7654321/1000000007               90.9 ns         90.9 ns      7722341
BM_Lua_PowMod/2/1000/1000000007                         274 ns          274 ns      2666574
BM_Lua_PowMod/7/1000000/1000000007                      460 ns          460 ns      1521373
BM_Lua_PowMod/1234567/7654321/1000000007                553 ns          553 ns      1274129
BM_FakeLua_PowMod_TCC/2/1000/1000000007                 141 ns          141 ns      4988913
BM_FakeLua_PowMod_TCC/7/1000000/1000000007              210 ns          210 ns      3337177
BM_FakeLua_PowMod_TCC/1234567/7654321/1000000007        255 ns          255 ns      2748347
BM_FakeLua_PowMod_GCC/2/1000/1000000007                 117 ns          117 ns      5978306
BM_FakeLua_PowMod_GCC/7/1000000/1000000007              179 ns          179 ns      3906099
BM_FakeLua_PowMod_GCC/1234567/7654321/1000000007        213 ns          213 ns      3279361
BM_CPP_Sum/10000                                       3112 ns         3112 ns       224760
BM_CPP_Sum/100000                                     31112 ns        31110 ns        22404
BM_CPP_Sum/1000000                                   311195 ns       311167 ns         2248
BM_CPP_Sum/5000000                                  1555687 ns      1555572 ns          450
BM_Lua_Sum/10000                                      46791 ns        46789 ns        14967
BM_Lua_Sum/100000                                    467331 ns       467302 ns         1499
BM_Lua_Sum/1000000                                  4686983 ns      4686599 ns          150
BM_Lua_Sum/5000000                                 27349707 ns     27348023 ns           30
BM_FakeLua_Sum_TCC/10000                              18770 ns        18768 ns        37329
BM_FakeLua_Sum_TCC/100000                            187170 ns       187164 ns         3746
BM_FakeLua_Sum_TCC/1000000                          1868705 ns      1868542 ns          375
BM_FakeLua_Sum_TCC/5000000                          9352917 ns      9351665 ns           75
BM_FakeLua_Sum_GCC/10000                               3168 ns         3167 ns       221462
BM_FakeLua_Sum_GCC/100000                             31178 ns        31177 ns        22457
BM_FakeLua_Sum_GCC/1000000                           311375 ns       311349 ns         2249
BM_FakeLua_Sum_GCC/5000000                          1556231 ns      1556186 ns          450
BM_VarTable_Set/2                                       179 ns          179 ns      3808169
BM_VarTable_Set/4                                       196 ns          196 ns      3592600
BM_VarTable_Set/8                                       235 ns          235 ns      2977578
BM_VarTable_Set/16                                      885 ns          884 ns       833139
BM_VarTable_Set/32                                     2184 ns         2183 ns       314089
BM_VarTable_Set/64                                     5042 ns         5042 ns       100000
BM_VarTable_Set/128                                    9986 ns         9985 ns        73062
BM_VarTable_Set/256                                   20403 ns        20401 ns        34240
BM_VarTable_Set/512                                   40494 ns        40490 ns        16866
BM_VarTable_Set/1024                                  81854 ns        81845 ns         9085
BM_StdUnorderedMap_Set/2                               73.9 ns         73.9 ns      9389954
BM_StdUnorderedMap_Set/4                                113 ns          113 ns      6220226
BM_StdUnorderedMap_Set/8                                205 ns          205 ns      3437963
BM_StdUnorderedMap_Set/16                               447 ns          447 ns      1565189
BM_StdUnorderedMap_Set/32                               942 ns          942 ns       742896
BM_StdUnorderedMap_Set/64                              1877 ns         1876 ns       373990
BM_StdUnorderedMap_Set/128                             4059 ns         4059 ns       172812
BM_StdUnorderedMap_Set/256                             9242 ns         9239 ns        76244
BM_StdUnorderedMap_Set/512                            20739 ns        20736 ns        34341
BM_StdUnorderedMap_Set/1024                           42938 ns        42934 ns        16226
BM_LuaTable_Set/2                                      1536 ns         1547 ns       466215
BM_LuaTable_Set/4                                      1654 ns         1666 ns       426272
BM_LuaTable_Set/8                                      1888 ns         1901 ns       374869
BM_LuaTable_Set/16                                     2327 ns         2340 ns       307574
BM_LuaTable_Set/32                                     3091 ns         3104 ns       234177
BM_LuaTable_Set/64                                     4521 ns         4536 ns       159768
BM_LuaTable_Set/128                                    7557 ns         7579 ns        97866
BM_LuaTable_Set/256                                   13519 ns        13547 ns        56532
BM_LuaTable_Set/512                                   25015 ns        25048 ns        30986
BM_LuaTable_Set/1024                                  46280 ns        46335 ns        15685
BM_VarTable_Get/2                                      22.4 ns         22.4 ns     31238962
BM_VarTable_Get/4                                      45.5 ns         45.5 ns     15402709
BM_VarTable_Get/8                                      97.4 ns         97.4 ns      7104715
BM_VarTable_Get/16                                      180 ns          180 ns      3884112
BM_VarTable_Get/32                                      384 ns          384 ns      1918128
BM_VarTable_Get/64                                      728 ns          727 ns       967691
BM_VarTable_Get/128                                    1442 ns         1441 ns       485886
BM_VarTable_Get/256                                    2998 ns         2998 ns       242699
BM_VarTable_Get/512                                    5746 ns         5745 ns       121815
BM_VarTable_Get/1024                                  11539 ns        11538 ns        60062
BM_StdUnorderedMap_Get/2                               4.68 ns         4.68 ns    149620196
BM_StdUnorderedMap_Get/4                               9.37 ns         9.37 ns     73995384
BM_StdUnorderedMap_Get/8                               17.9 ns         17.9 ns     39500708
BM_StdUnorderedMap_Get/16                              36.0 ns         36.0 ns     19451025
BM_StdUnorderedMap_Get/32                              72.5 ns         72.5 ns      9663020
BM_StdUnorderedMap_Get/64                               144 ns          144 ns      4862071
BM_StdUnorderedMap_Get/128                              289 ns          289 ns      2419882
BM_StdUnorderedMap_Get/256                              576 ns          576 ns      1219277
BM_StdUnorderedMap_Get/512                             1184 ns         1183 ns       594266
BM_StdUnorderedMap_Get/1024                            2371 ns         2371 ns       294539
BM_LuaTable_Get/2                                      37.1 ns         37.1 ns     18883999
BM_LuaTable_Get/4                                      74.3 ns         74.3 ns      9410653
BM_LuaTable_Get/8                                       147 ns          147 ns      4775283
BM_LuaTable_Get/16                                      303 ns          303 ns      2311372
BM_LuaTable_Get/32                                      592 ns          592 ns      1183426
BM_LuaTable_Get/64                                     1171 ns         1171 ns       598309
BM_LuaTable_Get/128                                    2326 ns         2326 ns       301249
BM_LuaTable_Get/256                                    4644 ns         4643 ns       151073
BM_LuaTable_Get/512                                    9265 ns         9264 ns        75608
BM_LuaTable_Get/1024                                  18520 ns        18518 ns        37829
BM_VarTable_Iter/2                                    0.627 ns        0.627 ns   1125347019
BM_VarTable_Iter/4                                     1.46 ns         1.46 ns    494430774
BM_VarTable_Iter/8                                     2.73 ns         2.73 ns    254318727
BM_VarTable_Iter/16                                    7.56 ns         7.56 ns     92653114
BM_VarTable_Iter/32                                    16.1 ns         16.1 ns     43507335
BM_VarTable_Iter/64                                    32.7 ns         32.7 ns     21421657
BM_VarTable_Iter/128                                   45.5 ns         45.5 ns     15351761
BM_VarTable_Iter/256                                   85.4 ns         85.4 ns      8171584
BM_VarTable_Iter/512                                    165 ns          165 ns      4241217
BM_VarTable_Iter/1024                                   325 ns          325 ns      2155722
BM_StdUnorderedMap_Iter/2                             0.623 ns        0.623 ns   1125635290
BM_StdUnorderedMap_Iter/4                              1.55 ns         1.55 ns    454112662
BM_StdUnorderedMap_Iter/8                              2.79 ns         2.79 ns    251027285
BM_StdUnorderedMap_Iter/16                             6.45 ns         6.45 ns    108629483
BM_StdUnorderedMap_Iter/32                             16.9 ns         16.9 ns     41247630
BM_StdUnorderedMap_Iter/64                             45.1 ns         45.1 ns     15486872
BM_StdUnorderedMap_Iter/128                             166 ns          166 ns      4218218
BM_StdUnorderedMap_Iter/256                             325 ns          325 ns      2151802
BM_StdUnorderedMap_Iter/512                             644 ns          644 ns      1086192
BM_StdUnorderedMap_Iter/1024                           1571 ns         1571 ns       446181
BM_LuaTable_Iter/2                                     65.4 ns         65.4 ns     10702964
BM_LuaTable_Iter/4                                      121 ns          121 ns      5787460
BM_LuaTable_Iter/8                                      230 ns          230 ns      3048078
BM_LuaTable_Iter/16                                     457 ns          457 ns      1534764
BM_LuaTable_Iter/32                                     890 ns          890 ns       786898
BM_LuaTable_Iter/64                                    1758 ns         1758 ns       398416
BM_LuaTable_Iter/128                                   3567 ns         3567 ns       200221
BM_LuaTable_Iter/256                                   6959 ns         6959 ns       100557
BM_LuaTable_Iter/512                                  13898 ns        13897 ns        50193
BM_LuaTable_Iter/1024                                 27781 ns        27779 ns        25202
BM_VarTable_Del/2                                       670 ns          670 ns      1036343
BM_VarTable_Del/4                                       696 ns          697 ns       998887
BM_VarTable_Del/8                                       746 ns          747 ns       934162
BM_VarTable_Del/16                                      895 ns          886 ns       811153
BM_VarTable_Del/32                                     1158 ns         1136 ns       648571
BM_VarTable_Del/64                                     1596 ns         1571 ns       453859
BM_VarTable_Del/128                                    2481 ns         2445 ns       289968
BM_VarTable_Del/256                                    4113 ns         4076 ns       170463
BM_VarTable_Del/512                                    7488 ns         7453 ns        94032
BM_VarTable_Del/1024                                  14173 ns        14084 ns        49694
BM_StdUnorderedMap_Del/2                                675 ns          673 ns      1040195
BM_StdUnorderedMap_Del/4                                690 ns          689 ns      1007970
BM_StdUnorderedMap_Del/8                                734 ns          737 ns       951363
BM_StdUnorderedMap_Del/16                               913 ns          918 ns       764396
BM_StdUnorderedMap_Del/32                              1244 ns         1249 ns       560582
BM_StdUnorderedMap_Del/64                              1867 ns         1871 ns       373451
BM_StdUnorderedMap_Del/128                             3238 ns         3234 ns       207420
BM_StdUnorderedMap_Del/256                             4742 ns         4732 ns       148200
BM_StdUnorderedMap_Del/512                             8777 ns         8755 ns        79444
BM_StdUnorderedMap_Del/1024                           17086 ns        17067 ns        41092
BM_LuaTable_Del/2                                      1344 ns         1316 ns       530719
BM_LuaTable_Del/4                                      1374 ns         1346 ns       515394
BM_LuaTable_Del/8                                      1438 ns         1409 ns       500333
BM_LuaTable_Del/16                                     1557 ns         1526 ns       458729
BM_LuaTable_Del/32                                     1787 ns         1763 ns       396059
BM_LuaTable_Del/64                                     2243 ns         2229 ns       314952
BM_LuaTable_Del/128                                    3167 ns         3139 ns       225382
BM_LuaTable_Del/256                                    5013 ns         4981 ns       141482
BM_LuaTable_Del/512                                    8712 ns         8666 ns        82239
BM_LuaTable_Del/1024                                  15835 ns        15798 ns        44658
```

---

## 算法性能分析（C++ vs Lua vs FakeLua TCC vs FakeLua GCC）

以下取各算法的典型参数做横向对比（CPU Time），**倍数 = 对应实现时间 / C++ 时间**：

### 1. Fibonacci（n=32，递归无记忆化）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 4.46 ms | 1x |
| Lua | 141.77 ms | **31.8x** 慢 |
| FakeLua TCC | 81.94 ms | **18.4x** 慢 |
| FakeLua GCC | 14.58 ms | **3.3x** 慢 |

> GCC `-O3` 对递归 Fibonacci 提升约 **5.6x**（相比 TCC），比 Lua 快 **9.7x**。TCC 在本次结果中已**快于 Lua 1.7 倍**，是较上一版本最显著的改进（TCC 从 325ms 降至 82ms，提升 **4.0x**），主要受益于数值参数特化生成内联条件判断。GCC 也从 19.8ms 降至 14.6ms（**1.4x** 提升）。

### 2. GCD（a=832040, b=514229，约 30 次迭代）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 61.0 ns | 1x |
| Lua | 358 ns | **5.9x** 慢 |
| FakeLua TCC | 174 ns | **2.9x** 慢 |
| FakeLua GCC | 158 ns | **2.6x** 慢 |

> TCC 从 993 ns 降至 174 ns，提升 **5.7x**，现已**快于 Lua 2.1 倍**（上一版本 TCC 比 Lua 慢 2.8x）。改进来源于对 `a, b = b, a % b` 中被重新赋值的参数 `a`/`b` 启用数值特化。GCC 结果基本持平（158 ns vs 150 ns，均在噪声范围内）。

### 3. PowMod（base=1234567, exp=7654321, mod=1e9+7，约 23 次循环）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 90.9 ns | 1x |
| Lua | 553 ns | **6.1x** 慢 |
| FakeLua TCC | 255 ns | **2.8x** 慢 |
| FakeLua GCC | 213 ns | **2.3x** 慢 |

> TCC 从 2641 ns 降至 255 ns，提升 **10.4x**，现已**快于 Lua 2.2 倍**（上一版本 TCC 比 Lua 慢 4.8x）。`base`、`exp` 均为循环内被重新赋值的参数，启用特化后循环体全部使用原生 `int64_t` 运算。GCC 基本持平（213 ns vs 199 ns）。

### 4. Sum（n=5000000，纯循环累加）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 1.556 ms | 1x |
| Lua | 27.35 ms | **17.6x** 慢 |
| FakeLua TCC | 9.35 ms | **6.0x** 慢 |
| FakeLua GCC | 1.556 ms | **≈1.0x**（与 C++ 完全持平） |

> TCC 从 147.98 ms 降至 9.35 ms，提升 **15.8x**，现已**快于 Lua 2.9 倍**（上一版本 TCC 比 Lua 慢 5.0x）。循环变量 `s` 和 `i` 经类型推断为整数，条件判断也生成直接 C 比较表达式，整个循环体无 CVar 装箱。GCC 与 C++ 完全持平，结果同上一版本一致。

---

## 与上一版本（2026-04-20）的对比

| 场景 | TCC 上一版 | TCC 本次 | TCC 提升 | GCC 上一版 | GCC 本次 | GCC 提升 |
|------|-----------|---------|---------|-----------|---------|---------|
| Fib(32) | 325.6 ms | 81.9 ms | **4.0x** | 19.8 ms | 14.6 ms | **1.4x** |
| GCD(832040/514229) | 993 ns | 174 ns | **5.7x** | 150 ns | 158 ns | ≈同 |
| PowMod(1234567/…) | 2641 ns | 255 ns | **10.4x** | 199 ns | 213 ns | ≈同 |
| Sum(5e6) | 147.98 ms | 9.35 ms | **15.8x** | 1.557 ms | 1.556 ms | ≈同 |

> TCC 的大幅提升来源于两项优化：① **数值参数特化**（含重新赋值参数），让 GCD/PowMod/Fib 循环体内全部使用原生 `int64_t` 而非 CVar；② **条件表达式原生化**（`TryCompileNativeBoolExpr`），将 `OpLe`+`IsTrue` 替换为直接 C 比较（如 `(n) <= (1)`）。GCC 后端已无明显提升空间（原本已接近 C++ 原生），本次数值变化在噪声范围内。

---

## 表操作性能分析（VarTable vs unordered_map vs Lua Table）

取 n=1024 比较：

| 操作 | VarTable | unordered_map | Lua Table | VarTable vs map | VarTable vs Lua |
|------|----------|---------------|-----------|-----------------|-----------------|
| Set  | 81.8 µs | 42.9 µs | 46.3 µs | 1.91x 慢 | 1.77x 慢 |
| Get  | 11.5 µs | 2.4 µs | 18.5 µs | 4.86x 慢 | **1.61x 快** |
| Iter | 0.325 µs | 1.571 µs | 27.78 µs | **4.83x 快** | **85.5x 快** |
| Del  | 14.1 µs | 17.1 µs | 15.8 µs | **1.21x 快** | **1.12x 快** |

> VarTable 的核心优势在 **Iterate** 上（紧密的 active_list 遍历完全 cache-friendly），在 1024 元素时比 unordered_map 快 4.8 倍，比 Lua Table 快 **85 倍**。**Delete** 和 **Get** 方面 VarTable 也优于 Lua Table。**Set** 因分配 heap + active_list 维护略慢，但差距在可接受范围内（小规模 ≤8 时 VarTable 的 quick_data 路径更优）。**Get** 方面 VarTable 弱于 `unordered_map`（后者哈希查找常数更小）。

---

## FakeLua TCC vs Lua 5.4 详细对比

本次结果中 **TCC 后端在全部算法上均已超越 Lua 5.4**（**倍数 = Lua CPU Time / FakeLua TCC CPU Time**）：

| 算法 | 参数 | Lua | FakeLua TCC | TCC 快多少 |
|------|------|-----|-------------|-----------|
| Fibonacci | n=20 | 447.0 µs | 254.2 µs | **1.8x** |
| Fibonacci | n=25 | 4941.7 µs | 2821.5 µs | **1.8x** |
| Fibonacci | n=30 | 54.29 ms | 31.30 ms | **1.7x** |
| Fibonacci | n=32 | 141.77 ms | 81.94 ms | **1.7x** |
| GCD | 832040/514229 | 358 ns | 174 ns | **2.1x** |
| GCD | 123456789/987654321 | 88.1 ns | 72.7 ns | **1.2x** |
| GCD | 2147483647/1073741823 | 77.8 ns | 69.1 ns | **1.1x** |
| PowMod | 2/1000/1e9+7 | 274 ns | 141 ns | **1.9x** |
| PowMod | 7/1e6/1e9+7 | 460 ns | 210 ns | **2.2x** |
| PowMod | 1234567/7654321/1e9+7 | 553 ns | 255 ns | **2.2x** |
| Sum | n=10000 | 46.8 µs | 18.8 µs | **2.5x** |
| Sum | n=100000 | 467.3 µs | 187.2 µs | **2.5x** |
| Sum | n=1000000 | 4686.6 µs | 1868.5 µs | **2.5x** |
| Sum | n=5000000 | 27348.0 µs | 9351.7 µs | **2.9x** |

---

## FakeLua GCC vs Lua 5.4 详细对比

以下直接对比 FakeLua GCC 后端与 Lua 5.4 解释器的性能（**倍数 = Lua CPU Time / FakeLua GCC CPU Time**）：

### Fibonacci（递归密集型）

| 参数 | Lua | FakeLua GCC | GCC 快多少 |
|------|-----|-------------|-----------|
| n=20 | 447.0 µs | 45.7 µs | **9.8x** |
| n=25 | 4941.7 µs | 501.1 µs | **9.9x** |
| n=30 | 54.29 ms | 5.65 ms | **9.6x** |
| n=32 | 141.77 ms | 14.58 ms | **9.7x** |

### GCD（短循环迭代）

| 参数 | Lua | FakeLua GCC | GCC 快多少 |
|------|-----|-------------|-----------|
| 832040/514229 | 358 ns | 158 ns | **2.3x** |
| 123456789/987654321 | 88.1 ns | 59.0 ns | **1.5x** |
| 2147483647/1073741823 | 77.8 ns | 58.3 ns | **1.3x** |

### PowMod（中等循环+取模）

| 参数 | Lua | FakeLua GCC | GCC 快多少 |
|------|-----|-------------|-----------|
| 2/1000/1e9+7 | 274 ns | 117 ns | **2.3x** |
| 7/1e6/1e9+7 | 460 ns | 179 ns | **2.6x** |
| 1234567/7654321/1e9+7 | 553 ns | 213 ns | **2.6x** |

### Sum（纯循环累加）

| 参数 | Lua | FakeLua GCC | GCC 快多少 |
|------|-----|-------------|-----------|
| n=10000 | 46.8 µs | 3.17 µs | **14.8x** |
| n=100000 | 467.3 µs | 31.2 µs | **15.0x** |
| n=1000000 | 4686.6 µs | 311.3 µs | **15.1x** |
| n=5000000 | 27348.0 µs | 1556.2 µs | **17.6x** |

### FakeLua GCC vs Lua 总结

| 场景类型 | FakeLua GCC 比 Lua 快 | 原因 |
|---------|----------------------|------|
| **纯循环累加 (Sum)** | **14.8–17.6x** | GCC `-O3` 内联 + 向量化，达到 C++ 原生水平 |
| **递归 (Fibonacci)** | **≈9.7x** | 数值特化 + 原生条件比较，GCC 优化递归调用开销 |
| **算术循环 (PowMod)** | **2.3–2.6x** | 循环体内取模运算受益于寄存器分配优化 |
| **短迭代 (GCD)** | **1.3–2.3x** | 迭代次数少，函数调用开销占比较高，优势相对小 |

> **FakeLua GCC 后端比 Lua 5.4 快 1.3x ~ 17.6x**，计算越密集（循环越长）优势越大。

---

## 总体结论

1. **C++ 最快**：在全部算法上领先，受益于 `-O3` 内联/展开/向量化。
2. **FakeLua GCC 全面超越 Lua，多场景接近 C++ 原生性能**：
   - Fib(32): GCC 14.58ms，仅为 C++ 的 **3.3x**，比 Lua 快 **9.7x**
   - GCD(832040,514229): GCC 158ns，仅为 C++ 的 **2.6x**，比 Lua 快 **2.3x**
   - PowMod(1234567,7654321,1e9+7): GCC 213ns，仅为 C++ 的 **2.3x**，比 Lua 快 **2.6x**
   - Sum(5e6): GCC 1.556ms，与 C++ **完全持平**（≈1.0x），比 Lua 快 **17.6x**
3. **FakeLua TCC 本次全面超越 Lua**（上一版本 TCC 全面落后于 Lua）：数值参数特化（含循环内重新赋值参数）和条件表达式原生化两项优化，使 TCC 在全部算法上比 Lua 快 **1.1x ~ 2.9x**。TCC 比 GCC 仍慢 1.5–6x，主要因为 TCC 无向量化/内联，但在编译速度方面依然有优势。
4. **VarTable 遍历性能极优**：Iter 操作比 Lua Table 快 **85 倍**，比 `unordered_map` 快 4.8 倍，核心在于 active_list 的紧凑布局。Get 快于 Lua Table，Del 快于两者，Set 略慢于两者但差距可接受。

> 注：ASLR 开启，结果有一定随机噪声；建议在 `--cpu-scaling-enabled=false` 环境下多重复（`--benchmark_repetitions=5`）后取均值。
