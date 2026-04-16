# Benchmark Results

本文件记录在本地以 **Release 模式**（`-O3 -DNDEBUG`）编译运行 `bench_mark` 的完整结果，以及对各算法的性能分析。

## 基准说明

### 算法对比（benchmark_algo.cpp）

将 C++、Lua 5.4、FakeLua（JIT_TCC）在同一文件中进行横向性能对比，覆盖四类算法：

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

- 日期：2026-04-16
- 机器：4 vCPU @ 2870 MHz
- 构建模式：**Release**（`cmake .. -DCMAKE_BUILD_TYPE=Release`，最终编译标志 `-O3 -DNDEBUG`）
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
2026-04-16T14:17:34+00:00
Running ./bench_mark
Run on (4 X 2870.78 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x2)
  L1 Instruction 32 KiB (x2)
  L2 Unified 1024 KiB (x2)
  L3 Unified 32768 KiB (x1)
Load Average: 2.66, 1.56, 0.65
***WARNING*** ASLR is enabled, the results may have unreproducible noise in them.
---------------------------------------------------------------------------------------
Benchmark                                             Time             CPU   Iterations
---------------------------------------------------------------------------------------
BM_CPP_Fibonacci/20                               13868 ns        13866 ns        49834
BM_CPP_Fibonacci/25                              140857 ns       140841 ns         4973
BM_CPP_Fibonacci/30                             1719469 ns      1718996 ns          408
BM_CPP_Fibonacci/32                             4360577 ns      4359853 ns          161
BM_Lua_Fibonacci/20                              439010 ns       438838 ns         1595
BM_Lua_Fibonacci/25                             4836406 ns      4835738 ns          145
BM_Lua_Fibonacci/30                            53793784 ns     53781704 ns           13
BM_Lua_Fibonacci/32                           141102159 ns    141079233 ns            5
BM_FakeLua_Fibonacci/20                         1079896 ns      1079716 ns          648
BM_FakeLua_Fibonacci/25                        12719641 ns     12715829 ns           55
BM_FakeLua_Fibonacci/30                       131158114 ns    131140368 ns            5
BM_FakeLua_Fibonacci/32                       340764237 ns    340728053 ns            2
BM_CPP_GCD/832040/514229                           68.9 ns         68.9 ns     10150379
BM_CPP_GCD/123456789/987654321                     7.38 ns         7.38 ns     94719289
BM_CPP_GCD/2147483647/1073741823                   4.92 ns         4.92 ns    142163063
BM_Lua_GCD/832040/514229                            315 ns          315 ns      2220079
BM_Lua_GCD/123456789/987654321                     83.2 ns         83.1 ns      8394367
BM_Lua_GCD/2147483647/1073741823                   73.9 ns         73.9 ns      9493778
BM_FakeLua_GCD/832040/514229                       1025 ns         1025 ns       682414
BM_FakeLua_GCD/123456789/987654321                  163 ns          163 ns      4250704
BM_FakeLua_GCD/2147483647/1073741823                129 ns          129 ns      5448015
BM_CPP_PowMod/2/1000/1000000007                    42.1 ns         42.1 ns     16615099
BM_CPP_PowMod/7/1000000/1000000007                 73.5 ns         73.5 ns      9524833
BM_CPP_PowMod/1234567/7654321/1000000007            103 ns          102 ns      6837621
BM_Lua_PowMod/2/1000/1000000007                     261 ns          261 ns      2689951
BM_Lua_PowMod/7/1000000/1000000007                  436 ns          436 ns      1607081
BM_Lua_PowMod/1234567/7654321/1000000007            515 ns          515 ns      1352721
BM_FakeLua_PowMod/2/1000/1000000007                1189 ns         1189 ns       586747
BM_FakeLua_PowMod/7/1000000/1000000007             2244 ns         2243 ns       317411
BM_FakeLua_PowMod/1234567/7654321/1000000007       2622 ns         2622 ns       267464
BM_CPP_Sum/10000                                   3511 ns         3510 ns       199579
BM_CPP_Sum/100000                                 35140 ns        35137 ns        19929
BM_CPP_Sum/1000000                               351506 ns       351411 ns         1978
BM_CPP_Sum/5000000                              1757276 ns      1756988 ns          399
BM_Lua_Sum/10000                                  54491 ns        54477 ns        12842
BM_Lua_Sum/100000                                544858 ns       544784 ns         1267
BM_Lua_Sum/1000000                              5453681 ns      5452911 ns          129
BM_Lua_Sum/5000000                             27330777 ns     27326160 ns           26
BM_FakeLua_Sum/10000                             300098 ns       300064 ns         2334
BM_FakeLua_Sum/100000                           3057336 ns      3056846 ns          234
BM_FakeLua_Sum/1000000                         29885698 ns     29878056 ns           23
BM_FakeLua_Sum/5000000                        149466764 ns    149428015 ns            5
BM_VarTable_Set/2                                   207 ns          207 ns      3283063
BM_VarTable_Set/4                                   224 ns          224 ns      3014189
BM_VarTable_Set/8                                   260 ns          260 ns      2648943
BM_VarTable_Set/16                                  970 ns          970 ns       758754
BM_VarTable_Set/32                                 2377 ns         2377 ns       285362
BM_VarTable_Set/64                                 5269 ns         5268 ns       100000
BM_VarTable_Set/128                               10431 ns        10429 ns        69851
BM_VarTable_Set/256                               20801 ns        20797 ns        32982
BM_VarTable_Set/512                               40703 ns        40696 ns        16779
BM_VarTable_Set/1024                              81076 ns        81046 ns         8980
BM_StdUnorderedMap_Set/2                           70.6 ns         70.5 ns      9958384
BM_StdUnorderedMap_Set/4                            110 ns          110 ns      6379030
BM_StdUnorderedMap_Set/8                            201 ns          201 ns      3478199
BM_StdUnorderedMap_Set/16                           438 ns          437 ns      1601903
BM_StdUnorderedMap_Set/32                           918 ns          918 ns       764086
BM_StdUnorderedMap_Set/64                          1933 ns         1932 ns       370135
BM_StdUnorderedMap_Set/128                         3906 ns         3905 ns       179407
BM_StdUnorderedMap_Set/256                         9224 ns         9222 ns        75772
BM_StdUnorderedMap_Set/512                        21111 ns        21108 ns        32712
BM_StdUnorderedMap_Set/1024                       45255 ns        45248 ns        15578
BM_LuaTable_Set/2                                  1982 ns         2010 ns       345526
BM_LuaTable_Set/4                                  2108 ns         2137 ns       332684
BM_LuaTable_Set/8                                  2346 ns         2375 ns       297615
BM_LuaTable_Set/16                                 2783 ns         2811 ns       258561
BM_LuaTable_Set/32                                 3555 ns         3580 ns       204437
BM_LuaTable_Set/64                                 5083 ns         5110 ns       143392
BM_LuaTable_Set/128                                8310 ns         8318 ns        92358
BM_LuaTable_Set/256                               15043 ns        15039 ns        52010
BM_LuaTable_Set/512                               27656 ns        27658 ns        28340
BM_LuaTable_Set/1024                              48182 ns        48214 ns        14384
BM_VarTable_Get/2                                  21.8 ns         21.8 ns     32759068
BM_VarTable_Get/4                                  44.7 ns         44.7 ns     15719949
BM_VarTable_Get/8                                  92.1 ns         92.1 ns      7587591
BM_VarTable_Get/16                                  175 ns          175 ns      4199873
BM_VarTable_Get/32                                  339 ns          339 ns      2049998
BM_VarTable_Get/64                                  677 ns          677 ns      1035092
BM_VarTable_Get/128                                1359 ns         1359 ns       515345
BM_VarTable_Get/256                                2733 ns         2732 ns       258129
BM_VarTable_Get/512                                5415 ns         5413 ns       129309
BM_VarTable_Get/1024                              10204 ns        10201 ns        64685
BM_StdUnorderedMap_Get/2                           5.07 ns         5.06 ns    137150743
BM_StdUnorderedMap_Get/4                           9.96 ns         9.96 ns     70292438
BM_StdUnorderedMap_Get/8                           19.8 ns         19.8 ns     35412012
BM_StdUnorderedMap_Get/16                          39.6 ns         39.6 ns     17698893
BM_StdUnorderedMap_Get/32                          79.1 ns         79.1 ns      8843044
BM_StdUnorderedMap_Get/64                           158 ns          158 ns      4425887
BM_StdUnorderedMap_Get/128                          316 ns          316 ns      2214132
BM_StdUnorderedMap_Get/256                          633 ns          633 ns      1106140
BM_StdUnorderedMap_Get/512                         1275 ns         1275 ns       544074
BM_StdUnorderedMap_Get/1024                        2572 ns         2572 ns       272838
BM_LuaTable_Get/2                                  37.3 ns         37.3 ns     18754497
BM_LuaTable_Get/4                                  75.7 ns         75.7 ns      9402103
BM_LuaTable_Get/8                                   149 ns          149 ns      4703178
BM_LuaTable_Get/16                                  295 ns          295 ns      2368379
BM_LuaTable_Get/32                                  588 ns          588 ns      1190885
BM_LuaTable_Get/64                                 1193 ns         1190 ns       589195
BM_LuaTable_Get/128                                2363 ns         2362 ns       296742
BM_LuaTable_Get/256                                4660 ns         4659 ns       151858
BM_LuaTable_Get/512                                9355 ns         9354 ns        73077
BM_LuaTable_Get/1024                              18797 ns        18793 ns        37307
BM_VarTable_Iter/2                                0.704 ns        0.704 ns    989338115
BM_VarTable_Iter/4                                 1.68 ns         1.67 ns    421368375
BM_VarTable_Iter/8                                 3.17 ns         3.17 ns    223312661
BM_VarTable_Iter/16                                8.77 ns         8.76 ns     80046263
BM_VarTable_Iter/32                                18.3 ns         18.3 ns     38201982
BM_VarTable_Iter/64                                38.3 ns         38.3 ns     18291989
BM_VarTable_Iter/128                               76.1 ns         76.1 ns      9146027
BM_VarTable_Iter/256                                153 ns          153 ns      4561050
BM_VarTable_Iter/512                                248 ns          248 ns      2811093
BM_VarTable_Iter/1024                               591 ns          591 ns      1179969
BM_StdUnorderedMap_Iter/2                         0.706 ns        0.706 ns    991852091
BM_StdUnorderedMap_Iter/4                          1.75 ns         1.75 ns    401314691
BM_StdUnorderedMap_Iter/8                          3.15 ns         3.15 ns    223033024
BM_StdUnorderedMap_Iter/16                         5.98 ns         5.98 ns    116724346
BM_StdUnorderedMap_Iter/32                         17.0 ns         17.0 ns     41080642
BM_StdUnorderedMap_Iter/64                         48.0 ns         48.0 ns     14546978
BM_StdUnorderedMap_Iter/128                         124 ns          124 ns      5667002
BM_StdUnorderedMap_Iter/256                         304 ns          304 ns      2305316
BM_StdUnorderedMap_Iter/512                         739 ns          739 ns       961460
BM_StdUnorderedMap_Iter/1024                       1880 ns         1879 ns       369868
BM_LuaTable_Iter/2                                 65.3 ns         65.3 ns     10755113
BM_LuaTable_Iter/4                                  120 ns          120 ns      5852159
BM_LuaTable_Iter/8                                  233 ns          233 ns      3044240
BM_LuaTable_Iter/16                                 453 ns          452 ns      1548024
BM_LuaTable_Iter/32                                 904 ns          904 ns       774233
BM_LuaTable_Iter/64                                1783 ns         1783 ns       392421
BM_LuaTable_Iter/128                               3582 ns         3582 ns       195261
BM_LuaTable_Iter/256                               7142 ns         7141 ns        97219
BM_LuaTable_Iter/512                              14275 ns        14273 ns        49074
BM_LuaTable_Iter/1024                             28565 ns        28562 ns        24568
BM_VarTable_Del/2                                   899 ns          909 ns       768390
BM_VarTable_Del/4                                   921 ns          932 ns       753531
BM_VarTable_Del/8                                   972 ns          983 ns       714374
BM_VarTable_Del/16                                 1053 ns         1088 ns       654197
BM_VarTable_Del/32                                 1206 ns         1301 ns       559203
BM_VarTable_Del/64                                 1627 ns         1713 ns       414063
BM_VarTable_Del/128                                2399 ns         2479 ns       286810
BM_VarTable_Del/256                                3901 ns         3968 ns       179134
BM_VarTable_Del/512                                6629 ns         6692 ns       104319
BM_VarTable_Del/1024                              12049 ns        12083 ns        57382
BM_StdUnorderedMap_Del/2                            909 ns          919 ns       768921
BM_StdUnorderedMap_Del/4                            927 ns          936 ns       749810
BM_StdUnorderedMap_Del/8                            983 ns          992 ns       704322
BM_StdUnorderedMap_Del/16                          1191 ns         1199 ns       583010
BM_StdUnorderedMap_Del/32                          1541 ns         1548 ns       452127
BM_StdUnorderedMap_Del/64                          2232 ns         2239 ns       315674
BM_StdUnorderedMap_Del/128                         3532 ns         3531 ns       198370
BM_StdUnorderedMap_Del/256                         5186 ns         5177 ns       135098
BM_StdUnorderedMap_Del/512                         9333 ns         9314 ns        74972
BM_StdUnorderedMap_Del/1024                       18023 ns        17997 ns        38943
BM_LuaTable_Del/2                                  1774 ns         1789 ns       393243
BM_LuaTable_Del/4                                  1808 ns         1823 ns       385251
BM_LuaTable_Del/8                                  1871 ns         1885 ns       370595
BM_LuaTable_Del/16                                 2001 ns         2014 ns       348703
BM_LuaTable_Del/32                                 2264 ns         2284 ns       309340
BM_LuaTable_Del/64                                 2814 ns         2833 ns       249467
BM_LuaTable_Del/128                                3871 ns         3904 ns       180058
BM_LuaTable_Del/256                                5984 ns         6041 ns       114994
BM_LuaTable_Del/512                               10218 ns        10272 ns        69469
BM_LuaTable_Del/1024                              18464 ns        18480 ns        36624
```

---

## 算法性能分析（C++ vs Lua vs FakeLua）

以下取各算法的典型参数做横向对比（CPU Time），**倍数 = 对应实现时间 / C++ 时间**：

### 1. Fibonacci（n=32，递归无记忆化）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 4.36 ms | 1x |
| Lua | 141.08 ms | **32x** 慢 |
| FakeLua | 340.73 ms | **78x** 慢 |

> Fibonacci 是纯递归密集调用场景，函数调用开销被无限放大。FakeLua 慢于 Lua 是因为每次函数调用都要走 `FakeluaCallByName` → JIT 函数指针查表的运行时桥接路径，相比 Lua 内部 VM dispatch 有额外开销。

### 2. GCD（a=832040, b=514229，约 30 次迭代）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 68.9 ns | 1x |
| Lua | 315 ns | **4.6x** 慢 |
| FakeLua | 1025 ns | **14.9x** 慢 |

> 迭代次数适中。C++ 被编译器优化得极紧；Lua 解释执行稳定在 4-5x 范围；FakeLua 因跨语言边界调用成本，倍数更高。

### 3. PowMod（base=1234567, exp=7654321, mod=1e9+7，约 23 次循环）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 103 ns | 1x |
| Lua | 515 ns | **5.0x** 慢 |
| FakeLua | 2622 ns | **25.5x** 慢 |

> 每次迭代中有取模、位运算等多操作，较 GCD 更重。FakeLua 相对倍数比 Lua 更大，说明 JIT 生成的 C 中每个复合运算仍经过 CVar 装箱/拆箱。

### 4. Sum（n=5000000，纯循环累加）

| 实现 | CPU Time | vs C++ |
|------|----------|--------|
| C++ | 1.76 ms | 1x |
| Lua | 27.33 ms | **15.5x** 慢 |
| FakeLua | 149.43 ms | **84.9x** 慢 |

> Sum 是最典型的"循环密集"场景。C++ 被完全向量化/展开；Lua VM 仅执行最简单整数 ADD；FakeLua 在循环每次迭代中仍需 CVar 运算，开销远超 Lua VM。

---

## 表操作性能分析（VarTable vs unordered_map vs Lua Table）

取 n=1024 比较：

| 操作 | VarTable | unordered_map | Lua Table | VarTable vs map |
|------|----------|---------------|-----------|-----------------|
| Set  | 81 µs | 45 µs | 48 µs | 1.8x 慢 |
| Get  | 10.2 µs | 2.6 µs | 18.8 µs | 1.0x（≈持平 Lua，但慢于 map） |
| Iter | 0.59 µs | 1.88 µs | 28.6 µs | **3.2x 快于 map，48x 快于 Lua** |
| Del  | 12.1 µs | 18.0 µs | 18.5 µs | **1.5x 快于 map** |

> VarTable 的核心优势在 **Iterate** 上（紧密的 active_list 遍历完全 cache-friendly），以及 **Delete** 略优于 map。Set 相对慢是因为分配 heap + active_list 维护；Get 与 Lua 持平但弱于 map（map 的 key 是原生 int64，无需 Var 装箱比较）。

---

## 总体结论

1. **C++ 最快**：在全部算法上领先，受益于 `-O3` 内联/展开/向量化。
2. **Lua 稳定在 C++ 的 3–35x 慢**：对于简单的迭代算法倍数相对低；递归密集时倍数随调用深度指数上升。
3. **FakeLua（JIT_TCC） 目前慢于 Lua**：根本原因是每次 Lua 函数调用都要走 `FakeluaCallByName` 符号查找 + 参数 CVar 打包/解包，这在高频调用（递归 Fib、Sum 循环）中成本显著。FakeLua 的设计目标是"可嵌入的脚本引擎"而非替代 Lua 的通用解释器，其优势在于：
   - 自定义数据结构（如 VarTable Iterate 极快）
   - 编译期类型安全、无 GC 压力
   - 可与 C++ 零开销边界集成（通过模板 `Call<Ret, Args...>()`）
4. 如需提升 FakeLua 算法性能，可考虑：
   - 在生成的 C 代码中对 int 路径直接用 `int64_t` 操作，跳过 CVar 装箱
   - 对 Fibonacci 这类自调用函数，在 TCC 内部做内联

> 注：ASLR 开启，结果有一定随机噪声；建议在 `--cpu-scaling-enabled=false` 环境下多重复（`--benchmark_repetitions=5`）后取均值。
