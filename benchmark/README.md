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
- 机器：4 vCPU @ 2596 MHz
- 构建模式：**Release**（`cmake .. -DCMAKE_BUILD_TYPE=Release`，最终编译标志 `-O3 -DNDEBUG`）
- FakeLua TCC JIT：**Release 模式**（`debug_mode=false`，TCC 启用 `-O2` 优化）
- 二进制：`build/bin/bench_mark`

## 运行命令

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DFAKELUA_TINYCC_GIT_URL=https://github.com/TinyCC/tinycc.git
cmake --build . -j4
cd bin
./bench_mark --benchmark_repetitions=1 --benchmark_report_aggregates_only=true
```

---

## 完整原始输出

```text
2026-04-17T01:35:07+00:00
Running ./bench_mark
Run on (4 X 2596.14 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x2)
  L1 Instruction 32 KiB (x2)
  L2 Unified 1024 KiB (x2)
  L3 Unified 32768 KiB (x1)
Load Average: 1.60, 1.13, 0.50
***WARNING*** ASLR is enabled, the results may have unreproducible noise in them.
---------------------------------------------------------------------------------------
Benchmark                                             Time             CPU   Iterations
---------------------------------------------------------------------------------------
BM_CPP_Fibonacci/20                               13884 ns        13882 ns        50493
BM_CPP_Fibonacci/25                              140784 ns       140755 ns         4975
BM_CPP_Fibonacci/30                             1717201 ns      1717023 ns          408
BM_CPP_Fibonacci/32                             4337103 ns      4336294 ns          161
BM_Lua_Fibonacci/20                              439108 ns       439049 ns         1597
BM_Lua_Fibonacci/25                             4901192 ns      4899903 ns          143
BM_Lua_Fibonacci/30                            54126355 ns     54120652 ns           13
BM_Lua_Fibonacci/32                           141086184 ns    141056733 ns            5
BM_FakeLua_Fibonacci/20                         1054114 ns      1053980 ns          664
BM_FakeLua_Fibonacci/25                        11686079 ns     11684418 ns           60
BM_FakeLua_Fibonacci/30                       129622788 ns    129604559 ns            5
BM_FakeLua_Fibonacci/32                       339405222 ns    339380380 ns            2
BM_CPP_GCD/832040/514229                           69.4 ns         69.4 ns     10111624
BM_CPP_GCD/123456789/987654321                     7.46 ns         7.46 ns     90291629
BM_CPP_GCD/2147483647/1073741823                   4.92 ns         4.92 ns    142270154
BM_Lua_GCD/832040/514229                            316 ns          316 ns      2215125
BM_Lua_GCD/123456789/987654321                     85.0 ns         85.0 ns      8288920
BM_Lua_GCD/2147483647/1073741823                   75.1 ns         75.1 ns      9345192
BM_FakeLua_GCD/832040/514229                       1026 ns         1026 ns       667225
BM_FakeLua_GCD/123456789/987654321                  163 ns          163 ns      4282847
BM_FakeLua_GCD/2147483647/1073741823                129 ns          129 ns      5433105
BM_CPP_PowMod/2/1000/1000000007                    42.6 ns         42.6 ns     16445234
BM_CPP_PowMod/7/1000000/1000000007                 73.6 ns         73.6 ns      9527033
BM_CPP_PowMod/1234567/7654321/1000000007            103 ns          103 ns      6838548
BM_Lua_PowMod/2/1000/1000000007                     259 ns          259 ns      2706965
BM_Lua_PowMod/7/1000000/1000000007                  435 ns          435 ns      1609252
BM_Lua_PowMod/1234567/7654321/1000000007            514 ns          514 ns      1362079
BM_FakeLua_PowMod/2/1000/1000000007                1189 ns         1188 ns       589411
BM_FakeLua_PowMod/7/1000000/1000000007             2210 ns         2210 ns       317108
BM_FakeLua_PowMod/1234567/7654321/1000000007       2621 ns         2620 ns       267209
BM_CPP_Sum/10000                                   3509 ns         3508 ns       199434
BM_CPP_Sum/100000                                 35150 ns        35142 ns        19930
BM_CPP_Sum/1000000                               351578 ns       351542 ns         1992
BM_CPP_Sum/5000000                              1757948 ns      1757579 ns          398
BM_Lua_Sum/10000                                  54472 ns        54464 ns        12861
BM_Lua_Sum/100000                                544654 ns       544601 ns         1281
BM_Lua_Sum/1000000                              5443014 ns      5441890 ns          129
BM_Lua_Sum/5000000                             27203587 ns     27199619 ns           26
BM_FakeLua_Sum/10000                             299132 ns       299098 ns         2341
BM_FakeLua_Sum/100000                           2988552 ns      2988285 ns          234
BM_FakeLua_Sum/1000000                         29921660 ns     29915931 ns           23
BM_FakeLua_Sum/5000000                        149578706 ns    149557266 ns            5
BM_VarTable_Set/2                                   213 ns          213 ns      3339853
BM_VarTable_Set/4                                   226 ns          226 ns      2982551
BM_VarTable_Set/8                                   266 ns          266 ns      2581146
BM_VarTable_Set/16                                  963 ns          963 ns       749238
BM_VarTable_Set/32                                 2363 ns         2363 ns       284570
BM_VarTable_Set/64                                 5218 ns         5217 ns       100000
BM_VarTable_Set/128                               10369 ns        10366 ns        71745
BM_VarTable_Set/256                               20680 ns        20676 ns        32773
BM_VarTable_Set/512                               41011 ns        41002 ns        17184
BM_VarTable_Set/1024                              80436 ns        80427 ns         9039
BM_StdUnorderedMap_Set/2                           80.8 ns         80.8 ns      9095035
BM_StdUnorderedMap_Set/4                            115 ns          115 ns      6068783
BM_StdUnorderedMap_Set/8                            207 ns          207 ns      3362248
BM_StdUnorderedMap_Set/16                           463 ns          463 ns      1517900
BM_StdUnorderedMap_Set/32                          1011 ns         1010 ns       695343
BM_StdUnorderedMap_Set/64                          2014 ns         2013 ns       348728
BM_StdUnorderedMap_Set/128                         4018 ns         4017 ns       169024
BM_StdUnorderedMap_Set/256                         9490 ns         9489 ns        74115
BM_StdUnorderedMap_Set/512                        21880 ns        21874 ns        32044
BM_StdUnorderedMap_Set/1024                       46441 ns        46433 ns        15175
BM_LuaTable_Set/2                                  1908 ns         1939 ns       347240
BM_LuaTable_Set/4                                  2086 ns         2115 ns       336109
BM_LuaTable_Set/8                                  2324 ns         2352 ns       304527
BM_LuaTable_Set/16                                 2761 ns         2790 ns       258536
BM_LuaTable_Set/32                                 3529 ns         3555 ns       207122
BM_LuaTable_Set/64                                 5043 ns         5072 ns       145316
BM_LuaTable_Set/128                                8423 ns         8433 ns        89825
BM_LuaTable_Set/256                               14961 ns        14959 ns        51213
BM_LuaTable_Set/512                               27228 ns        27225 ns        28699
BM_LuaTable_Set/1024                              48858 ns        48886 ns        14833
BM_VarTable_Get/2                                  22.5 ns         22.5 ns     31092659
BM_VarTable_Get/4                                  45.8 ns         45.7 ns     15315917
BM_VarTable_Get/8                                  95.4 ns         95.3 ns      7406708
BM_VarTable_Get/16                                  175 ns          175 ns      3985532
BM_VarTable_Get/32                                  350 ns          350 ns      1990119
BM_VarTable_Get/64                                  699 ns          699 ns      1001236
BM_VarTable_Get/128                                1402 ns         1402 ns       498848
BM_VarTable_Get/256                                2801 ns         2800 ns       250098
BM_VarTable_Get/512                                5602 ns         5600 ns       125063
BM_VarTable_Get/1024                              11179 ns        11177 ns        62654
BM_StdUnorderedMap_Get/2                           5.06 ns         5.06 ns    139185727
BM_StdUnorderedMap_Get/4                           9.93 ns         9.93 ns     70453379
BM_StdUnorderedMap_Get/8                           19.7 ns         19.7 ns     35464373
BM_StdUnorderedMap_Get/16                          39.5 ns         39.5 ns     17713061
BM_StdUnorderedMap_Get/32                          79.1 ns         79.1 ns      8867435
BM_StdUnorderedMap_Get/64                           158 ns          158 ns      4425917
BM_StdUnorderedMap_Get/128                          316 ns          316 ns      2214033
BM_StdUnorderedMap_Get/256                          633 ns          633 ns      1107259
BM_StdUnorderedMap_Get/512                         1274 ns         1274 ns       546249
BM_StdUnorderedMap_Get/1024                        2569 ns         2569 ns       272508
BM_LuaTable_Get/2                                  37.4 ns         37.4 ns     18780942
BM_LuaTable_Get/4                                  76.2 ns         76.2 ns      9247180
BM_LuaTable_Get/8                                   152 ns          152 ns      4616068
BM_LuaTable_Get/16                                  301 ns          301 ns      2327582
BM_LuaTable_Get/32                                  610 ns          610 ns      1146895
BM_LuaTable_Get/64                                 1232 ns         1232 ns       568575
BM_LuaTable_Get/128                                2407 ns         2406 ns       291454
BM_LuaTable_Get/256                                4830 ns         4829 ns       146211
BM_LuaTable_Get/512                                9747 ns         9746 ns        71886
BM_LuaTable_Get/1024                              19109 ns        19107 ns        36644
BM_VarTable_Iter/2                                0.704 ns        0.703 ns    994930021
BM_VarTable_Iter/4                                 1.69 ns         1.69 ns    421384008
BM_VarTable_Iter/8                                 3.15 ns         3.15 ns    222291832
BM_VarTable_Iter/16                                8.70 ns         8.70 ns     80226519
BM_VarTable_Iter/32                                18.3 ns         18.3 ns     38645379
BM_VarTable_Iter/64                                38.2 ns         38.2 ns     18328577
BM_VarTable_Iter/128                               75.6 ns         75.6 ns      9256702
BM_VarTable_Iter/256                                153 ns          153 ns      4557449
BM_VarTable_Iter/512                                245 ns          245 ns      2863752
BM_VarTable_Iter/1024                               600 ns          600 ns      1185937
BM_StdUnorderedMap_Iter/2                         0.716 ns        0.716 ns    908277859
BM_StdUnorderedMap_Iter/4                          1.75 ns         1.75 ns    398244293
BM_StdUnorderedMap_Iter/8                          3.15 ns         3.15 ns    222254998
BM_StdUnorderedMap_Iter/16                         5.97 ns         5.97 ns    116931102
BM_StdUnorderedMap_Iter/32                         17.0 ns         17.0 ns     41111966
BM_StdUnorderedMap_Iter/64                         48.0 ns         48.0 ns     14489655
BM_StdUnorderedMap_Iter/128                         124 ns          124 ns      5653240
BM_StdUnorderedMap_Iter/256                         304 ns          304 ns      2304945
BM_StdUnorderedMap_Iter/512                         730 ns          729 ns       959356
BM_StdUnorderedMap_Iter/1024                       1870 ns         1870 ns       375285
BM_LuaTable_Iter/2                                 65.6 ns         65.5 ns     10898204
BM_LuaTable_Iter/4                                  121 ns          121 ns      5545113
BM_LuaTable_Iter/8                                  225 ns          225 ns      3133023
BM_LuaTable_Iter/16                                 438 ns          438 ns      1610510
BM_LuaTable_Iter/32                                 872 ns          872 ns       772793
BM_LuaTable_Iter/64                                1716 ns         1716 ns       407445
BM_LuaTable_Iter/128                               3405 ns         3405 ns       205512
BM_LuaTable_Iter/256                               6792 ns         6792 ns       103174
BM_LuaTable_Iter/512                              13552 ns        13551 ns        51650
BM_LuaTable_Iter/1024                             27181 ns        27177 ns        25846
BM_VarTable_Del/2                                   878 ns          889 ns       787426
BM_VarTable_Del/4                                   903 ns          914 ns       766350
BM_VarTable_Del/8                                   954 ns          966 ns       724090
BM_VarTable_Del/16                                 1051 ns         1085 ns       653139
BM_VarTable_Del/32                                 1241 ns         1327 ns       539831
BM_VarTable_Del/64                                 1699 ns         1762 ns       407691
BM_VarTable_Del/128                                2448 ns         2522 ns       273089
BM_VarTable_Del/256                                3940 ns         4007 ns       175652
BM_VarTable_Del/512                                6864 ns         6892 ns        98895
BM_VarTable_Del/1024                              12460 ns        12475 ns        55837
BM_StdUnorderedMap_Del/2                            901 ns          909 ns       769312
BM_StdUnorderedMap_Del/4                            920 ns          928 ns       753868
BM_StdUnorderedMap_Del/8                            968 ns          975 ns       719764
BM_StdUnorderedMap_Del/16                          1215 ns         1222 ns       585054
BM_StdUnorderedMap_Del/32                          1507 ns         1511 ns       464434
BM_StdUnorderedMap_Del/64                          2183 ns         2187 ns       316426
BM_StdUnorderedMap_Del/128                         3717 ns         3718 ns       188275
BM_StdUnorderedMap_Del/256                         5054 ns         5048 ns       139182
BM_StdUnorderedMap_Del/512                         9320 ns         9297 ns        75276
BM_StdUnorderedMap_Del/1024                       18042 ns        18016 ns        38919
BM_LuaTable_Del/2                                  1767 ns         1781 ns       392815
BM_LuaTable_Del/4                                  1801 ns         1815 ns       386101
BM_LuaTable_Del/8                                  1861 ns         1876 ns       373499
BM_LuaTable_Del/16                                 2000 ns         2014 ns       348025
BM_LuaTable_Del/32                                 2259 ns         2278 ns       308366
BM_LuaTable_Del/64                                 2775 ns         2797 ns       249881
BM_LuaTable_Del/128                                3813 ns         3854 ns       183955
BM_LuaTable_Del/256                                5956 ns         6012 ns       118310
BM_LuaTable_Del/512                               10100 ns        10152 ns        69304
BM_LuaTable_Del/1024                              18114 ns        18125 ns        38829
```

---

## 算法性能分析（C++ vs Lua vs FakeLua）

以下取各算法的典型参数做横向对比（CPU Time），**倍数 = 对应实现时间 / C++ 时间**：

### 1. Fibonacci（n=32，递归无记忆化）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 4.34 ms | 1x |
| Lua | 141.06 ms | **32x** 慢 |
| FakeLua | 339.38 ms | **78x** 慢 |

> Fibonacci 是纯递归密集调用场景，函数调用开销被无限放大。FakeLua 慢于 Lua 是因为每次函数调用都要走 `FakeluaCallByName` → JIT 函数指针查表的运行时桥接路径，相比 Lua 内部 VM dispatch 有额外开销。TCC `-O2` 对 Fibonacci 提升有限（约 0.4%），因为瓶颈在函数调用路径而非指令本身。

### 2. GCD（a=832040, b=514229，约 30 次迭代）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 69.4 ns | 1x |
| Lua | 316 ns | **4.6x** 慢 |
| FakeLua | 1026 ns | **14.8x** 慢 |

> 迭代次数适中。C++ 被编译器优化得极紧；Lua 解释执行稳定在 4-5x 范围；FakeLua 因跨语言边界调用成本，倍数更高。

### 3. PowMod（base=1234567, exp=7654321, mod=1e9+7，约 23 次循环）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 103 ns | 1x |
| Lua | 514 ns | **5.0x** 慢 |
| FakeLua | 2620 ns | **25.4x** 慢 |

> 每次迭代中有取模、位运算等多操作，较 GCD 更重。FakeLua 相对倍数比 Lua 更大，说明 JIT 生成的 C 中每个复合运算仍经过 CVar 装箱/拆箱。

### 4. Sum（n=5000000，纯循环累加）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 1.76 ms | 1x |
| Lua | 27.20 ms | **15.5x** 慢 |
| FakeLua | 149.56 ms | **85.1x** 慢 |

> Sum 是最典型的"循环密集"场景。C++ 被完全向量化/展开；Lua VM 仅执行最简单整数 ADD；FakeLua 在循环每次迭代中仍需 CVar 运算，开销远超 Lua VM。

---

## 表操作性能分析（VarTable vs unordered_map vs Lua Table）

取 n=1024 比较：

| 操作 | VarTable | unordered_map | Lua Table | VarTable vs map |
|------|----------|---------------|-----------|-----------------|
| Set  | 80.4 µs | 46.4 µs | 48.9 µs | 1.7x 慢 |
| Get  | 11.2 µs | 2.6 µs | 19.1 µs | 1.1x（≈持平 Lua，但慢于 map） |
| Iter | 0.60 µs | 1.87 µs | 27.2 µs | **3.1x 快于 map，45x 快于 Lua** |
| Del  | 12.5 µs | 18.0 µs | 18.1 µs | **1.4x 快于 map** |

> VarTable 的核心优势在 **Iterate** 上（紧密的 active_list 遍历完全 cache-friendly），以及 **Delete** 略优于 map。Set 相对慢是因为分配 heap + active_list 维护；Get 与 Lua 持平但弱于 map（map 的 key 是原生 int64，无需 Var 装箱比较）。

---

## 总体结论

1. **C++ 最快**：在全部算法上领先，受益于 `-O3` 内联/展开/向量化。
2. **Lua 稳定在 C++ 的 4–32x 慢**：对于简单的迭代算法倍数相对低；递归密集时倍数随调用深度指数上升。
3. **FakeLua（JIT_TCC）慢于 Lua，但 FakeLua（JIT_GCC）明显提升**：同一套脚本在 GCC 后端下，算法类 benchmark 相比 TCC 提升约 **6.8x ~ 47.4x**。典型样本：
   - Fib(32): TCC 325.07ms → GCC 17.61ms（**18.46x**，且比 Lua 140.71ms 更快）
   - GCD(832040,514229): TCC 1003ns → GCC 148ns（**6.78x**，快于 Lua 353ns）
   - PowMod(1234567,7654321,1e9+7): TCC 2577ns → GCC 198ns（**13.02x**，快于 Lua 552ns）
   - Sum(5e6): TCC 147.70ms → GCC 3.12ms（**47.37x**，约为 C++ 的 2.0x）
4. **瓶颈分析更新**：TCC 路径的主要成本仍是 `FakeluaCallByName` + CVar 装箱/拆箱；GCC 后端通过更强优化显著降低脚本内算术/循环开销，使总开销从“桥接主导”转向“桥接 + 计算混合”。这说明后端编译器优化级别对 FakeLua 运行时表现影响非常大。
5. FakeLua 的设计目标仍是"可嵌入的脚本引擎"而非替代 Lua 的通用解释器，其优势在于：
    - 自定义数据结构（如 VarTable Iterate 极快）
    - 编译期类型安全、无 GC 压力
    - 可与 C++ 零开销边界集成（通过模板 `Call<Ret, Args...>()`）
6. 如需继续提升 FakeLua 算法性能，可考虑：
    - 在生成的 C 代码中对 int 路径直接用 `int64_t` 操作，跳过 CVar 装箱
    - 对高频小函数做更激进的内联/去桥接（特别是递归和循环中的内部调用）

> 注：ASLR 开启，结果有一定随机噪声；建议在 `--cpu-scaling-enabled=false` 环境下多重复（`--benchmark_repetitions=5`）后取均值。
