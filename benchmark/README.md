# Benchmark Results

本文件记录在本地以 **Release 模式**（`-O3 -DNDEBUG`）编译运行 `bench_mark` 的完整结果。覆盖 **6 大类共 51 个 Lua 性能场景**，每个场景均实现 C++ / Lua 5.4 / FakeLua TCC / FakeLua GCC 四种横向对比（下面的分析聚焦 GCC vs Lua、GCC vs C++）。

## 运行环境

- 日期：2026-08-13
- 机器：16 X 2000 MHz CPU s
- CPU 缓存：L1d 64 KiB (x16)，L1i 64 KiB (x16)，L2 512 KiB (x16)，L3 65536 KiB (x2)
- 构建模式：**Release**（`-O3 -DNDEBUG`），GCC 13.2.0
- FakeLua GCC JIT：**Release 模式**（`debug_mode=false`，GCC `-O3` 优化）
- 二进制：`build/bin/bench_mark`

> 绝对耗时强依赖于机器与编译器版本，跨环境不可比。本文件所有数字来自同一次运行，只有同一张表内的比值才有意义。特别地，C++ 参照实现在部分场景会被编译器整体折叠（见下文标注），此时 GCC vs C++ 一列无参考价值。

## 运行命令

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target bench_mark --parallel
build/bin/bench_mark --benchmark_repetitions=1 --benchmark_report_aggregates_only=true
```

---

## 结论

下表取各场景最大参数（最具代表性），**GCC vs Lua** 列表示 FakeLua GCC 相对 Lua 5.4 的加速倍数（>1 表示 FakeLua 更快），**GCC vs C++** 列表示 FakeLua GCC 与手写 C++ 的耗时比值（<1 表示 FakeLua 更快）。

### 算法（algo）

| 场景 | 参数 | GCC vs Lua | GCC vs C++ | 备注 |
|------|------|-----------|-----------|------|
| Fibonacci | n=32 | **43.5x** | 0.88 | 数值特化，略快于 C++ |
| GCD | 2147483647/1073741823 | 0.85x | 9.23 | 单次调用仅约 0.2 µs，被调用开销主导 |
| PowMod | 1234567/7654321/1e9+7 | 2.7x | 1.43 | |
| Sum | n=5M | **17.1x** | 0.99 | 与 C++ 持平 |
| BubbleSort | n=200 | 2.0x | 3.53 | 表下标读写拖累 |
| Sieve | n=5000 | 1.6x | 9.43 | |
| BinarySearch | n=1000 | 3.8x | 4.78 | |
| FastPow | 1234567/7654321/1e9+7 | 2.6x | 1.41 | |
| Popcount | n=100K | **18.7x** | 0.97 | 位运算，与 C++ 持平 |
| InsertionSort | n=200 | 1.8x | 27.1 | |
| MatMul | 3×3 | 3.0x | — | C++ 侧被整体折叠（4 ns），比值无意义 |
| Vector3 | n=1M | **5.9x** | 46.6 | 表特化为结构体；C++ 侧被向量化 |
| FloatPoly | n=1M | **24.3x** | 1.02 | 浮点特化，与 C++ 持平 |

### 字符串（string）

| 场景 | 参数 | GCC vs Lua | GCC vs C++ | 备注 |
|------|------|-----------|-----------|------|
| StringLen | n=10K | 2.3x | — | C++ 侧被折叠为常量（2 ns） |
| StringSub | n=10K | 1.9x | 4.74 | |
| StringRep | n=1000 | 1.2x | 1.12 | |
| StringReverse | n=10K | 1.2x | 1.22 | |
| StringLower | n=10K | 0.47x | 3.89 | 逐字符转换 |
| StringUpper | n=10K | 0.45x | 3.98 | 逐字符转换 |
| StringByte | n=1000 | 0.82x | — | C++ 侧被折叠为常量（2.4 ns） |
| StringChar | n=500 | 0.22x | 210 | 脚本层逐字符拼接 |
| StringFormat | n=500 | 0.47x | 63.5 | 格式解析开销高 |
| StringFind | n=10K | 1.2x | 16.2 | |
| StringGsub | n=1000 | 0.23x | 21.4 | 模式匹配 |
| ToNumber | n=1 | 0.32x | 30.2 | |
| ToString | n=500 | 0.99x | 0.04 | |
| StringFindPattern | n=1000 | 0.39x | 28.4 | 模式匹配 |
| StringGmatch | n=1000 | 0.45x | 21.6 | 迭代器+模式匹配 |

### 表操作（table）

| 场景 | 参数 | GCC vs Lua | GCC vs C++ | 备注 |
|------|------|-----------|-----------|------|
| TableInsert | n=5K | **4.2x** | 28.7 | |
| TableRemove | n=5K | **3.6x** | 97.6 | |
| TableConcat | n=1000 | 0.69x | 26.0 | 结果先在 std::string 拼装再整体拷入 arena |
| TablePack | n=1 | 2.6x | — | C++ 侧被折叠（0.2 ns） |
| TableMove | n=5K | 0.78x | 75.4 | 逐元素 CVar 装箱，常数项高于 Lua 的整块搬运 |
| TableSort | n=1000 | **3.6x** | 4.08 | |
| TableCreate | n=5K | 1.2x | 35.9 | |
| HashInsert | n=1000 | 1.6x | 2.07 | |
| HashLookup | n=1000 | 1.4x | 1.31 | |
| NestedTable | n=10K | 2.3x | 16.8 | |

### 函数调用（function）

| 场景 | 参数 | GCC vs Lua | GCC vs C++ | 备注 |
|------|------|-----------|-----------|------|
| EmptyCall | n=100K | **13.1x** | — | C++ 侧整个循环被消除（0.2 ns） |
| Recursion | n=25 | **53.3x** | 1.04 | 数值特化，与 C++ 持平 |
| Variadic | n=1 | 0.21x | — | vararg 构建开销大；C++ 侧被折叠 |
| MultiReturn | n=10K | **20.7x** | 7.10 | |
| Closure | n=1000 | **7.9x** | 77.5 | |
| TailRecursion | n=5K | **78.6x** | 1.08 | 尾调用转循环 |

### GC 与内存压力（gc）

| 场景 | 参数 | GCC vs Lua | GCC vs C++ | 备注 |
|------|------|-----------|-----------|------|
| TableChurn | n=1000 | **24.3x** | **0.66** | arena 分配器优势，快于 C++ |
| StringChurn | n=1000 | 2.0x | 11.1 | 字符串分配是弱项 |
| MixedAlloc | n=1000 | 3.1x | 7.84 | |

### 数学函数（math）

| 场景 | 参数 | GCC vs Lua | GCC vs C++ | 备注 |
|------|------|-----------|-----------|------|
| MathTrig (sin+cos) | n=100K | **3.0x** | 1.00 | 与 C++ 持平 |
| MathSqrt | n=100K | **20.1x** | 1.00 | 与 C++ 持平 |
| MathExpLog | n=100K | **4.0x** | 0.95 | 与 C++ 持平 |
| MathMinMax | n=100K | **7.3x** | 1.00 | 与 C++ 持平 |

### 核心发现

1. **纯数值场景全面领先 Lua 5.4 并逼近手写 C++**：Fibonacci 43.5x、TailRecursion 78.6x、Recursion 53.3x、FloatPoly 24.3x、Popcount 18.7x、Sum 17.1x。其中 Sum / Popcount / Recursion / FloatPoly / TailRecursion 与 C++ 的比值在 0.97~1.08 之间，基本持平——数值特化让这些函数生成的 C 代码与手写版本几乎一致，剩下的就交给 GCC `-O3`。

2. **math 库四项全部与 C++ 持平**（比值 0.95~1.00）。`math.exp`/`math.log` 曾是反常的性能陷阱（比 Lua 还慢），原因是它们不在 CGen 的内联白名单里，每次调用都要走 `FakeluaCallByName` 的按名字符串查找与变参装箱；补入白名单后直接内联为 libm 调用。同批修复的还有 `log10`/`asin`/`acos`/`atan`/`sinh`/`cosh`/`tanh`——它们的内联代码早已写好，但同样因为不在白名单里而是**永远执行不到的死代码**。

3. **表操作从最大短板转为优势项**：table.insert 4.2x、remove 3.6x、sort 3.6x 快于 Lua，move / concat 与 Lua 基本持平。此前这几项比 Lua 慢 2~3 个数量级，根因是算法复杂度与宿主/JIT 两侧实现不一致，详见下节。

4. **arena 分配器在表频繁创建场景优势明显**：TableChurn 快于 Lua 24.3x，且比手写 C++ 还快（0.66）——无 GC、批量释放的收益在这里体现得最直接。

5. **字符串仍是主要短板，且集中在两类**：一是**模式匹配**（Gsub 0.23x、FindPattern 0.39x、Gmatch 0.45x），二是**逐字符处理**（Char 0.22x、Lower 0.47x、Upper 0.45x）。Lua 这些函数是高度优化的 C 实现，而 fakelua 侧每个字符都要经过 CVar 装箱与 arena 分配。相对地，简单字符串操作（len/sub/rep/reverse/find）已经反超 Lua 1.2~2.3x。

6. **单次调用开销仍高于 Lua 的场景**：GCD（0.85x）、ToNumber（0.32x）、Variadic（0.21x）。这些场景单次耗时都在 1 µs 以内，被跨语言调用的固定开销主导；Variadic 额外受 vararg 的 Multi 构建拖累。

---

## 表操作性能修复（2026-08-13）

表操作一节此前记录的数据（TableInsert 0.02x、TableMove ≈0x 等）源于两个缺陷，均已修复。

**根因一：`#t` 是 O(n)，使 table.insert/remove 退化为 O(n²)。**
`FlGetTableSeqLen` 每次都从 0 开始逐个向上探测整数键。CGen 把 `table.insert(t, v)` 内联成 `FlLenInt` + `FlSetTableInt`，于是每追加一个元素都要重新数一遍整张表。现在 `VarTable` 缓存连续整数键前缀长度，在整数键写入路径上增量维护，`#t` 变为 O(1)、追加整体 O(n)。缓存以「0 = 无效」编码，任何把 `VarTable` 清零的分配路径都会自动落到重算分支——漏挂钩只会退化性能，不会算错长度。

**根因二：宿主 C++ 侧与 JIT 侧是两套不兼容的表实现，且会静默丢数据。**
`TableHelper::GetTableInt` 是全表线性扫描而非哈希定位，让 table.concat/move/sort/unpack 这些逐元素读取的函数整体退化为 O(n²)。更严重的是 `TableHelper::SetTableInt` 在 `quick_data_` 的 8 个槽用满后，会把整数键 `std::to_string` 成十进制字符串再按 StringId 存入——JIT 侧按 `VAR_INT` 键哈希查找时永远匹配不上。**这不只是性能问题：`table.move` 复制 20 个元素时，第 9 个及之后全部丢失，`#dst` 返回 8。** 而 `bench_table_move` 返回的正是 `#dst`，所以它一直在给一个错误结果计时。现在宿主侧与 `c_runtime_header.h` 中的 `Fl*` 运行时函数共用同一套哈希、桶布局与 rehash 策略。

回归测试见 `test/lua/table/test_table_large_seq.lua` 与 `test_table_seqlen.lua`。此前的 `table.move` 用例最多只用 5 个元素，正好落在 8 槽以内，所以这个 bug 一直没被暴露。

修复前后在同一台机器、同一次会话中连续测得（`--benchmark_min_time=0.15s`，CPU 时间）：

| 场景 | 参数 | 修复前 | 修复后 | 提升 | 修复前 vs Lua | 修复后 vs Lua |
|------|------|-------:|-------:|-----:|-------------:|-------------:|
| TableInsert | n=5000 | 25114.8 µs | 119.7 µs | 210x | 0.02x | **4.24x** |
| TableMove | n=5000 | 46355.6 µs | 377.5 µs | 123x | 0.006x | 0.82x |
| TableRemove | n=5000 | 20647.6 µs | 226.7 µs | 91x | 0.04x | **3.62x** |
| TableSort | n=1000 | 3301.0 µs | 99.4 µs | 33x | 0.11x | **3.73x** |
| TableConcat | n=1000 | 2424.9 µs | 589.3 µs | 4.1x | 0.16x | 0.69x |
| MathExpLog | n=100K | 57520.1 µs | 5228.8 µs | 11x | 0.38x | **4.08x** |

伸缩性也从二次变为线性：TableInsert 的 n 从 1000 增到 5000（5 倍）时，耗时此前涨 24.6 倍，现在涨 6.7 倍。

仍略慢于 Lua 的 TableConcat 与 TableMove 已是线性复杂度，剩余差距来自逐元素 CVar 装箱和结果字符串的二次拷贝，属于常数项优化。

---

## 完整原始输出

以下为 `benchmark_algo.cpp` / `benchmark_string.cpp` / `benchmark_table.cpp` / `benchmark_function.cpp` / `benchmark_gc.cpp` / `benchmark_math.cpp` 全部 51 个场景的完整 google benchmark 输出（含 TCC 数据）：

```text
<string>:1305: warning: assignment of read-only location
Starting benchmarks...
2026-08-13T12:41:03+08:00
Running ./bench_mark
Run on (16 X 2000 MHz CPU s)
CPU Caches:
  L1 Data 64 KiB (x16)
  L1 Instruction 64 KiB (x16)
  L2 Unified 512 KiB (x16)
  L3 Unified 65536 KiB (x2)
Load Average: 2.40, 2.39, 2.66
***WARNING*** ASLR is enabled, the results may have unreproducible noise in them.
--------------------------------------------------------------------------------------------
Benchmark                                                  Time             CPU   Iterations
--------------------------------------------------------------------------------------------
BM_CPP_Fibonacci/20                                    23448 ns        23448 ns        28253
BM_CPP_Fibonacci/25                                   228036 ns       228034 ns         3085
BM_CPP_Fibonacci/30                                  2943099 ns      2943004 ns          237
BM_CPP_Fibonacci/32                                  7810644 ns      7810518 ns           94
BM_Lua_Fibonacci/20                                   918043 ns       918034 ns          779
BM_Lua_Fibonacci/25                                 10157119 ns     10156878 ns           70
BM_Lua_Fibonacci/30                                112859326 ns    112856781 ns            6
BM_Lua_Fibonacci/32                                297933601 ns    297920424 ns            2
BM_FakeLua_Fibonacci_TCC/20                            83003 ns        83003 ns         8464
BM_FakeLua_Fibonacci_TCC/25                           927875 ns       927847 ns          760
BM_FakeLua_Fibonacci_TCC/30                         10206390 ns     10206083 ns           66
BM_FakeLua_Fibonacci_TCC/32                         26690824 ns     26690406 ns           26
BM_FakeLua_Fibonacci_GCC/20                            26429 ns        26428 ns        26723
BM_FakeLua_Fibonacci_GCC/25                           240342 ns       240337 ns         2789
BM_FakeLua_Fibonacci_GCC/30                          2682346 ns      2682284 ns          261
BM_FakeLua_Fibonacci_GCC/32                          6847151 ns      6847069 ns          103
BM_CPP_GCD/832040/514229                                 169 ns          169 ns      4164423
BM_CPP_GCD/123456789/987654321                          21.6 ns         21.6 ns     32586825
BM_CPP_GCD/2147483647/1073741823                        16.9 ns         16.9 ns     41541471
BM_Lua_GCD/832040/514229                                 551 ns          551 ns      1302529
BM_Lua_GCD/123456789/987654321                           149 ns          149 ns      4694108
BM_Lua_GCD/2147483647/1073741823                         133 ns          133 ns      5219657
BM_FakeLua_GCD_TCC/832040/514229                         717 ns          717 ns       971748
BM_FakeLua_GCD_TCC/123456789/987654321                   215 ns          215 ns      3249237
BM_FakeLua_GCD_TCC/2147483647/1073741823                 199 ns          199 ns      3534737
BM_FakeLua_GCD_GCC/832040/514229                         314 ns          314 ns      2247000
BM_FakeLua_GCD_GCC/123456789/987654321                   158 ns          158 ns      4457195
BM_FakeLua_GCD_GCC/2147483647/1073741823                 156 ns          156 ns      4565894
BM_CPP_PowMod/2/1000/1000000007                          147 ns          147 ns      4699163
BM_CPP_PowMod/7/1000000/1000000007                       277 ns          277 ns      2539766
BM_CPP_PowMod/1234567/7654321/1000000007                 393 ns          393 ns      1763337
BM_Lua_PowMod/2/1000/1000000007                          699 ns          699 ns       923950
BM_Lua_PowMod/7/1000000/1000000007                      1174 ns         1174 ns       594117
BM_Lua_PowMod/1234567/7654321/1000000007                1491 ns         1491 ns       468742
BM_FakeLua_PowMod_TCC/2/1000/1000000007                  488 ns          488 ns      1443214
BM_FakeLua_PowMod_TCC/7/1000000/1000000007               908 ns          908 ns       771265
BM_FakeLua_PowMod_TCC/1234567/7654321/1000000007        1101 ns         1101 ns       617278
BM_FakeLua_PowMod_GCC/2/1000/1000000007                  299 ns          299 ns      2359596
BM_FakeLua_PowMod_GCC/7/1000000/1000000007               435 ns          435 ns      1613037
BM_FakeLua_PowMod_GCC/1234567/7654321/1000000007         561 ns          561 ns      1239629
BM_CPP_Sum/10000                                        3956 ns         3956 ns       176808
BM_CPP_Sum/100000                                      40071 ns        40070 ns        17640
BM_CPP_Sum/1000000                                    400079 ns       400067 ns         1762
BM_CPP_Sum/5000000                                   2004005 ns      2003974 ns          347
BM_Lua_Sum/10000                                       70261 ns        70259 ns        10143
BM_Lua_Sum/100000                                     687978 ns       687979 ns         1022
BM_Lua_Sum/1000000                                   6919193 ns      6918733 ns           97
BM_Lua_Sum/5000000                                  33892687 ns     33892314 ns           21
BM_FakeLua_Sum_TCC/10000                               36777 ns        36775 ns        19119
BM_FakeLua_Sum_TCC/100000                             362841 ns       362842 ns         1928
BM_FakeLua_Sum_TCC/1000000                           3632964 ns      3632403 ns          193
BM_FakeLua_Sum_TCC/5000000                          18352614 ns     18351620 ns           39
BM_FakeLua_Sum_GCC/10000                                4129 ns         4128 ns       169062
BM_FakeLua_Sum_GCC/100000                              39780 ns        39779 ns        17542
BM_FakeLua_Sum_GCC/1000000                            397137 ns       397132 ns         1761
BM_FakeLua_Sum_GCC/5000000                           1985968 ns      1985911 ns          352
BM_CPP_BubbleSort/50                                   12592 ns        12592 ns        55616
BM_CPP_BubbleSort/100                                  51750 ns        51750 ns        13412
BM_CPP_BubbleSort/200                                 209427 ns       209422 ns         3352
BM_Lua_BubbleSort/50                                   93690 ns        93688 ns         7525
BM_Lua_BubbleSort/100                                 366238 ns       366232 ns         1901
BM_Lua_BubbleSort/200                                1469298 ns      1469274 ns          479
BM_FakeLua_BubbleSort_TCC/50                          210959 ns       210951 ns         3319
BM_FakeLua_BubbleSort_TCC/100                         829677 ns       829662 ns          844
BM_FakeLua_BubbleSort_TCC/200                        3295617 ns      3295509 ns          213
BM_FakeLua_BubbleSort_GCC/50                           47763 ns        47762 ns        14614
BM_FakeLua_BubbleSort_GCC/100                         188518 ns       188509 ns         3741
BM_FakeLua_BubbleSort_GCC/200                         738797 ns       738769 ns          931
BM_CPP_Sieve/100                                         411 ns          411 ns      1672708
BM_CPP_Sieve/500                                        2080 ns         2080 ns       335285
BM_CPP_Sieve/1000                                       4326 ns         4326 ns       161503
BM_CPP_Sieve/5000                                      23263 ns        23263 ns        30718
BM_Lua_Sieve/100                                        9443 ns         9443 ns        73719
BM_Lua_Sieve/500                                       35571 ns        35569 ns        19705
BM_Lua_Sieve/1000                                      67011 ns        67009 ns        10263
BM_Lua_Sieve/5000                                     353454 ns       353448 ns         2049
BM_FakeLua_Sieve_TCC/100                               18205 ns        18205 ns        36664
BM_FakeLua_Sieve_TCC/500                               89823 ns        89820 ns         8117
BM_FakeLua_Sieve_TCC/1000                             174502 ns       174502 ns         3696
BM_FakeLua_Sieve_TCC/5000                            1032165 ns      1032134 ns          682
BM_FakeLua_Sieve_GCC/100                                3358 ns         3358 ns       202991
BM_FakeLua_Sieve_GCC/500                               14746 ns        14745 ns        47845
BM_FakeLua_Sieve_GCC/1000                              29187 ns        29184 ns        24056
BM_FakeLua_Sieve_GCC/5000                             219340 ns       219330 ns         3194
BM_CPP_BinarySearch/100                                 1383 ns         1383 ns       508016
BM_CPP_BinarySearch/500                                13013 ns        13013 ns        54135
BM_CPP_BinarySearch/1000                               34365 ns        34365 ns        20491
BM_Lua_BinarySearch/100                                39235 ns        39233 ns        18183
BM_Lua_BinarySearch/500                               260218 ns       260214 ns         2556
BM_Lua_BinarySearch/1000                              626541 ns       626532 ns         1155
BM_FakeLua_BinarySearch_TCC/100                        58028 ns        58023 ns        11337
BM_FakeLua_BinarySearch_TCC/500                       402336 ns       402317 ns         1750
BM_FakeLua_BinarySearch_TCC/1000                      928548 ns       928524 ns          752
BM_FakeLua_BinarySearch_GCC/100                        10776 ns        10775 ns        64635
BM_FakeLua_BinarySearch_GCC/500                        72243 ns        72242 ns         9720
BM_FakeLua_BinarySearch_GCC/1000                      164325 ns       164317 ns         4298
BM_CPP_FastPow/2/1000/1000000007                         146 ns          146 ns      4799939
BM_CPP_FastPow/7/1000000/1000000007                      277 ns          277 ns      2553631
BM_CPP_FastPow/1234567/7654321/1000000007                392 ns          392 ns      1777542
BM_Lua_FastPow/2/1000/1000000007                         702 ns          702 ns       988405
BM_Lua_FastPow/7/1000000/1000000007                     1094 ns         1094 ns       642296
BM_Lua_FastPow/1234567/7654321/1000000007               1455 ns         1455 ns       477513
BM_FakeLua_FastPow_TCC/2/1000/1000000007                 436 ns          436 ns      1596274
BM_FakeLua_FastPow_TCC/7/1000000/1000000007              691 ns          691 ns      1015705
BM_FakeLua_FastPow_TCC/1234567/7654321/1000000007        842 ns          842 ns       825392
BM_FakeLua_FastPow_GCC/2/1000/1000000007                 296 ns          296 ns      2369271
BM_FakeLua_FastPow_GCC/7/1000000/1000000007              435 ns          435 ns      1614054
BM_FakeLua_FastPow_GCC/1234567/7654321/1000000007        553 ns          553 ns      1259467
BM_CPP_Popcount/1000                                    5714 ns         5713 ns       122571
BM_CPP_Popcount/10000                                  74206 ns        74203 ns         9391
BM_CPP_Popcount/100000                               1007616 ns      1007594 ns          698
BM_Lua_Popcount/1000                                  124298 ns       124287 ns         5651
BM_Lua_Popcount/10000                                1519720 ns      1519703 ns          460
BM_Lua_Popcount/100000                              18193979 ns     18190900 ns           39
BM_FakeLua_Popcount_TCC/1000                           19266 ns        19265 ns        35109
BM_FakeLua_Popcount_TCC/10000                         255268 ns       255219 ns         2682
BM_FakeLua_Popcount_TCC/100000                       3098954 ns      3098923 ns          221
BM_FakeLua_Popcount_GCC/1000                            5845 ns         5845 ns       122702
BM_FakeLua_Popcount_GCC/10000                          73689 ns        73680 ns         9489
BM_FakeLua_Popcount_GCC/100000                        974000 ns       973979 ns          727
BM_CPP_InsertionSort/50                                 1451 ns         1451 ns       482970
BM_CPP_InsertionSort/100                                4194 ns         4194 ns       165415
BM_CPP_InsertionSort/200                               14650 ns        14649 ns        48412
BM_Lua_InsertionSort/50                                48067 ns        48062 ns        14592
BM_Lua_InsertionSort/100                              181870 ns       181846 ns         3812
BM_Lua_InsertionSort/200                              709371 ns       709361 ns          981
BM_FakeLua_InsertionSort_TCC/50                       141583 ns       141581 ns         4797
BM_FakeLua_InsertionSort_TCC/100                      540111 ns       540088 ns         1284
BM_FakeLua_InsertionSort_TCC/200                     2109588 ns      2109392 ns          331
BM_FakeLua_InsertionSort_GCC/50                        26881 ns        26876 ns        26062
BM_FakeLua_InsertionSort_GCC/100                      102536 ns       102533 ns         6931
BM_FakeLua_InsertionSort_GCC/200                      397055 ns       397043 ns         1768
BM_CPP_MatMul                                           4.05 ns         4.05 ns    173836790
BM_Lua_MatMul                                           4293 ns         4293 ns       163660
BM_FakeLua_MatMul_TCC                                   4159 ns         4159 ns       168098
BM_FakeLua_MatMul_GCC                                   1452 ns         1451 ns       484116
BM_CPP_Vector3/10000                                    6060 ns         6060 ns       112241
BM_CPP_Vector3/100000                                  59787 ns        59784 ns        11756
BM_CPP_Vector3/1000000                                607901 ns       607903 ns         1163
BM_Lua_Vector3/10000                                 1674702 ns      1674708 ns          418
BM_Lua_Vector3/100000                               16607335 ns     16606582 ns           42
BM_Lua_Vector3/1000000                             165439401 ns    165432945 ns            4
BM_FakeLua_Vector3_TCC/10000                         1912983 ns      1912938 ns          382
BM_FakeLua_Vector3_TCC/100000                       18861872 ns     18861625 ns           33
BM_FakeLua_Vector3_TCC/1000000                     183414545 ns    183411916 ns            4
BM_FakeLua_Vector3_GCC/10000                          286361 ns       286345 ns         2446
BM_FakeLua_Vector3_GCC/100000                        2956614 ns      2956583 ns          238
BM_FakeLua_Vector3_GCC/1000000                      28300363 ns     28300020 ns           25
BM_CPP_FloatPoly/1000000                             3693155 ns      3693069 ns          190
BM_Lua_FloatPoly/1000000                            91612112 ns     91607843 ns            8
BM_FakeLua_FloatPoly_TCC/1000000                    16616843 ns     16616407 ns           43
BM_FakeLua_FloatPoly_GCC/1000000                     3767688 ns      3767567 ns          184
BM_CPP_EmptyCall/10000                                 0.222 ns        0.222 ns   3499422665
BM_CPP_EmptyCall/100000                                0.200 ns        0.200 ns   3525871683
BM_Lua_EmptyCall/10000                                431027 ns       431008 ns         1626
BM_Lua_EmptyCall/100000                              4303441 ns      4303364 ns          163
BM_FakeLua_EmptyCall_TCC/10000                        290018 ns       290012 ns         2438
BM_FakeLua_EmptyCall_TCC/100000                      2896763 ns      2896656 ns          243
BM_FakeLua_EmptyCall_GCC/10000                         31291 ns        31291 ns        23354
BM_FakeLua_EmptyCall_GCC/100000                       328617 ns       328614 ns         2338
BM_CPP_Recursion/10                                      134 ns          134 ns      4986925
BM_CPP_Recursion/20                                    24102 ns        24101 ns        29991
BM_CPP_Recursion/25                                   228827 ns       228826 ns         3009
BM_Lua_Recursion/10                                     9411 ns         9410 ns        74605
BM_Lua_Recursion/20                                  1148983 ns      1148947 ns          605
BM_Lua_Recursion/25                                 12744926 ns     12744638 ns           55
BM_FakeLua_Recursion_TCC/10                              827 ns          827 ns       846367
BM_FakeLua_Recursion_TCC/20                            83094 ns        83093 ns         8367
BM_FakeLua_Recursion_TCC/25                           919871 ns       919846 ns          761
BM_FakeLua_Recursion_GCC/10                              322 ns          322 ns      2207860
BM_FakeLua_Recursion_GCC/20                            27194 ns        27107 ns        26526
BM_FakeLua_Recursion_GCC/25                           238925 ns       238915 ns         2901
BM_CPP_Variadic/1                                       1.21 ns         1.21 ns    574333423
BM_Lua_Variadic/1                                        583 ns          583 ns      1223109
BM_FakeLua_Variadic_TCC/1                               3079 ns         3079 ns       230944
BM_FakeLua_Variadic_GCC/1                               2758 ns         2758 ns       254929
BM_CPP_MultiReturn/1000                                  388 ns          388 ns      1802900
BM_CPP_MultiReturn/10000                                3975 ns         3975 ns       176186
BM_Lua_MultiReturn/1000                                58192 ns        58186 ns        12114
BM_Lua_MultiReturn/10000                              583214 ns       583036 ns         1192
BM_FakeLua_MultiReturn_TCC/1000                         5204 ns         5204 ns       135619
BM_FakeLua_MultiReturn_TCC/10000                       49808 ns        49807 ns        13823
BM_FakeLua_MultiReturn_GCC/1000                         3008 ns         3008 ns       233149
BM_FakeLua_MultiReturn_GCC/10000                       28228 ns        28228 ns        23477
BM_CPP_Closure/100                                      36.1 ns         36.1 ns     19319413
BM_CPP_Closure/1000                                      387 ns          387 ns      1811220
BM_Lua_Closure/100                                     23924 ns        23923 ns        29362
BM_Lua_Closure/1000                                   236957 ns       236935 ns         2979
BM_FakeLua_Closure_TCC/100                             15302 ns        15299 ns        45844
BM_FakeLua_Closure_TCC/1000                           150466 ns       150458 ns         4656
BM_FakeLua_Closure_GCC/100                              3078 ns         3078 ns       226996
BM_FakeLua_Closure_GCC/1000                            29995 ns        29994 ns        22419
BM_CPP_TailRecursion/100                                35.9 ns         35.9 ns     19241962
BM_CPP_TailRecursion/1000                                386 ns          386 ns      1817784
BM_CPP_TailRecursion/5000                               1971 ns         1971 ns       355368
BM_Lua_TailRecursion/100                                3524 ns         3524 ns       202886
BM_Lua_TailRecursion/1000                              33439 ns        33439 ns        19585
BM_Lua_TailRecursion/5000                             167089 ns       167084 ns         4352
BM_FakeLua_TailRecursion_TCC/100                         807 ns          807 ns       872605
BM_FakeLua_TailRecursion_TCC/1000                       6679 ns         6678 ns       106132
BM_FakeLua_TailRecursion_TCC/5000                      32568 ns        32567 ns        21424
BM_FakeLua_TailRecursion_GCC/100                         180 ns          180 ns      3877267
BM_FakeLua_TailRecursion_GCC/1000                        534 ns          534 ns      1333255
BM_FakeLua_TailRecursion_GCC/5000                       2125 ns         2125 ns       331399
BM_CPP_TableChurn/100                                   3699 ns         3699 ns       193690
BM_CPP_TableChurn/500                                  18289 ns        18290 ns        39018
BM_CPP_TableChurn/1000                                 36041 ns        36041 ns        19490
BM_Lua_TableChurn/100                                  56653 ns        56651 ns        12277
BM_Lua_TableChurn/500                                 303969 ns       303967 ns         2428
BM_Lua_TableChurn/1000                                578096 ns       578073 ns         1229
BM_FakeLua_TableChurn_TCC/100                          37069 ns        37069 ns        19087
BM_FakeLua_TableChurn_TCC/500                         181043 ns       181041 ns         3993
BM_FakeLua_TableChurn_TCC/1000                        342193 ns       342166 ns         2055
BM_FakeLua_TableChurn_GCC/100                           2493 ns         2493 ns       277851
BM_FakeLua_TableChurn_GCC/500                          11589 ns        11589 ns        60846
BM_FakeLua_TableChurn_GCC/1000                         23841 ns        23840 ns        29109
BM_CPP_StringChurn/100                                  5491 ns         5490 ns       123880
BM_CPP_StringChurn/500                                 28134 ns        28133 ns        25082
BM_CPP_StringChurn/1000                                56065 ns        56065 ns        12756
BM_Lua_StringChurn/100                                 84397 ns        84397 ns         8216
BM_Lua_StringChurn/500                                611891 ns       611867 ns         1167
BM_Lua_StringChurn/1000                              1234751 ns      1234736 ns          553
BM_FakeLua_StringChurn_TCC/100                         72777 ns        72776 ns         9673
BM_FakeLua_StringChurn_TCC/500                        359442 ns       359436 ns         1972
BM_FakeLua_StringChurn_TCC/1000                       735873 ns       735875 ns          958
BM_FakeLua_StringChurn_GCC/100                         61317 ns        61315 ns        11382
BM_FakeLua_StringChurn_GCC/500                        314030 ns       314031 ns         2331
BM_FakeLua_StringChurn_GCC/1000                       622228 ns       622207 ns         1122
BM_CPP_MixedAlloc/100                                   4192 ns         4192 ns       169726
BM_CPP_MixedAlloc/500                                  22689 ns        22688 ns        30371
BM_CPP_MixedAlloc/1000                                 45676 ns        45671 ns        15242
BM_Lua_MixedAlloc/100                                 106468 ns       106467 ns         6577
BM_Lua_MixedAlloc/500                                 551505 ns       551499 ns         1266
BM_Lua_MixedAlloc/1000                               1090600 ns      1090587 ns          641
BM_FakeLua_MixedAlloc_TCC/100                          53395 ns        53392 ns        13024
BM_FakeLua_MixedAlloc_TCC/500                         267721 ns       267722 ns         2611
BM_FakeLua_MixedAlloc_TCC/1000                        534591 ns       534556 ns         1307
BM_FakeLua_MixedAlloc_GCC/100                          35900 ns        35900 ns        19471
BM_FakeLua_MixedAlloc_GCC/500                         181545 ns       181536 ns         3928
BM_FakeLua_MixedAlloc_GCC/1000                        358008 ns       358008 ns         1956
BM_CPP_MathTrig/100000                               9049413 ns      9047792 ns           78
BM_Lua_MathTrig/100000                              27048566 ns     27045176 ns           26
BM_FakeLua_MathTrig_TCC/100000                      17831836 ns     17829881 ns           39
BM_FakeLua_MathTrig_GCC/100000                       9013421 ns      9013169 ns           77
BM_CPP_MathSqrt/100000                                318098 ns       318095 ns         2202
BM_Lua_MathSqrt/100000                               6388741 ns      6388569 ns          109
BM_FakeLua_MathSqrt_TCC/100000                       3593146 ns      3593100 ns          196
BM_FakeLua_MathSqrt_GCC/100000                        318609 ns       318605 ns         2201
BM_CPP_MathExpLog/100000                             5549176 ns      5548985 ns          135
BM_Lua_MathExpLog/100000                            20934475 ns     20934201 ns           33
BM_FakeLua_MathExpLog_TCC/100000                    12447864 ns     12447895 ns           56
BM_FakeLua_MathExpLog_GCC/100000                     5253030 ns      5252881 ns          134
BM_CPP_MathMinMax/100000                             4396264 ns      4396275 ns          159
BM_Lua_MathMinMax/100000                            32266020 ns     32266103 ns           22
BM_FakeLua_MathMinMax_TCC/100000                    13331562 ns     13331583 ns           53
BM_FakeLua_MathMinMax_GCC/100000                     4414305 ns      4414035 ns          157
BM_CPP_StringLen/10                                     2.00 ns         2.00 ns    349606996
BM_CPP_StringLen/100                                    1.99 ns         1.99 ns    349525421
BM_CPP_StringLen/1000                                   2.00 ns         2.00 ns    351603567
BM_CPP_StringLen/10000                                  1.99 ns         1.99 ns    352652643
BM_Lua_StringLen/10                                      130 ns          130 ns      5392496
BM_Lua_StringLen/100                                     289 ns          289 ns      2427809
BM_Lua_StringLen/1000                                    567 ns          567 ns      1235957
BM_Lua_StringLen/10000                                  2317 ns         2317 ns       298197
BM_FakeLua_StringLen_TCC/10                              240 ns          240 ns      2920493
BM_FakeLua_StringLen_TCC/100                             294 ns          294 ns      2389720
BM_FakeLua_StringLen_TCC/1000                            473 ns          473 ns      1472415
BM_FakeLua_StringLen_TCC/10000                           986 ns          986 ns       708947
BM_FakeLua_StringLen_GCC/10                              238 ns          238 ns      2920104
BM_FakeLua_StringLen_GCC/100                             291 ns          291 ns      2402533
BM_FakeLua_StringLen_GCC/1000                            471 ns          471 ns      1490079
BM_FakeLua_StringLen_GCC/10000                           996 ns          996 ns       706850
BM_CPP_StringSub/10                                     12.0 ns         12.0 ns     58369303
BM_CPP_StringSub/100                                    54.4 ns         54.4 ns     12901913
BM_CPP_StringSub/1000                                    122 ns          122 ns      5724642
BM_CPP_StringSub/10000                                   410 ns          410 ns      1704141
BM_Lua_StringSub/10                                      262 ns          262 ns      2710130
BM_Lua_StringSub/100                                     500 ns          500 ns      1401916
BM_Lua_StringSub/1000                                   1135 ns         1135 ns       613412
BM_Lua_StringSub/10000                                  3734 ns         3734 ns       188214
BM_FakeLua_StringSub_TCC/10                              683 ns          683 ns      1017430
BM_FakeLua_StringSub_TCC/100                             773 ns          773 ns       913360
BM_FakeLua_StringSub_TCC/1000                           1103 ns         1103 ns       647986
BM_FakeLua_StringSub_TCC/10000                          1964 ns         1964 ns       334601
BM_FakeLua_StringSub_GCC/10                              688 ns          688 ns      1031617
BM_FakeLua_StringSub_GCC/100                             806 ns          806 ns       897353
BM_FakeLua_StringSub_GCC/1000                           1081 ns         1081 ns       631892
BM_FakeLua_StringSub_GCC/10000                          1944 ns         1944 ns       358118
BM_CPP_StringRep/10                                     48.1 ns         48.1 ns     14698895
BM_CPP_StringRep/100                                     456 ns          456 ns      1620132
BM_CPP_StringRep/1000                                   3972 ns         3972 ns       167249
BM_Lua_StringRep/10                                      317 ns          317 ns      2228563
BM_Lua_StringRep/100                                     917 ns          917 ns       757483
BM_Lua_StringRep/1000                                   5199 ns         5199 ns       134239
BM_FakeLua_StringRep_TCC/10                              701 ns          701 ns       991714
BM_FakeLua_StringRep_TCC/100                            1104 ns         1104 ns       639275
BM_FakeLua_StringRep_TCC/1000                           4455 ns         4455 ns       157038
BM_FakeLua_StringRep_GCC/10                              701 ns          701 ns       997384
BM_FakeLua_StringRep_GCC/100                            1101 ns         1101 ns       638397
BM_FakeLua_StringRep_GCC/1000                           4468 ns         4468 ns       156861
BM_CPP_StringReverse/10                                 17.5 ns         17.5 ns     40011324
BM_CPP_StringReverse/100                                 134 ns          134 ns      5200653
BM_CPP_StringReverse/1000                                987 ns          987 ns       711282
BM_CPP_StringReverse/10000                              9051 ns         9051 ns        76924
BM_Lua_StringReverse/10                                  260 ns          260 ns      2656053
BM_Lua_StringReverse/100                                 649 ns          649 ns      1075631
BM_Lua_StringReverse/1000                               1883 ns         1883 ns       373183
BM_Lua_StringReverse/10000                             12999 ns        12999 ns        53622
BM_FakeLua_StringReverse_TCC/10                          599 ns          599 ns      1165184
BM_FakeLua_StringReverse_TCC/100                         807 ns          807 ns       867418
BM_FakeLua_StringReverse_TCC/1000                       1915 ns         1915 ns       362284
BM_FakeLua_StringReverse_TCC/10000                     10889 ns        10889 ns        64111
BM_FakeLua_StringReverse_GCC/10                          627 ns          627 ns      1147938
BM_FakeLua_StringReverse_GCC/100                         844 ns          844 ns       811140
BM_FakeLua_StringReverse_GCC/1000                       1989 ns         1989 ns       353609
BM_FakeLua_StringReverse_GCC/10000                     11069 ns        11068 ns        62432
BM_CPP_StringLower/10                                   19.7 ns         19.7 ns     35911746
BM_CPP_StringLower/100                                   162 ns          162 ns      4319270
BM_CPP_StringLower/1000                                 1101 ns         1101 ns       649730
BM_CPP_StringLower/10000                                8816 ns         8816 ns        76059
BM_Lua_StringLower/10                                    266 ns          266 ns      2598407
BM_Lua_StringLower/100                                   705 ns          705 ns       992430
BM_Lua_StringLower/1000                                 2221 ns         2221 ns       312290
BM_Lua_StringLower/10000                               16045 ns        16044 ns        43657
BM_FakeLua_StringLower_TCC/10                            603 ns          603 ns      1160459
BM_FakeLua_StringLower_TCC/100                          1052 ns         1052 ns       664499
BM_FakeLua_StringLower_TCC/1000                         4277 ns         4277 ns       164083
BM_FakeLua_StringLower_TCC/10000                       34182 ns        34182 ns        20536
BM_FakeLua_StringLower_GCC/10                            604 ns          604 ns      1155650
BM_FakeLua_StringLower_GCC/100                          1070 ns         1070 ns       653141
BM_FakeLua_StringLower_GCC/1000                         4264 ns         4264 ns       163765
BM_FakeLua_StringLower_GCC/10000                       34323 ns        34320 ns        20303
BM_CPP_StringUpper/10                                   19.5 ns         19.5 ns     33976523
BM_CPP_StringUpper/100                                   162 ns          162 ns      4367579
BM_CPP_StringUpper/1000                                 1102 ns         1102 ns       648411
BM_CPP_StringUpper/10000                                8809 ns         8809 ns        76881
BM_Lua_StringUpper/10                                    255 ns          255 ns      2690398
BM_Lua_StringUpper/100                                   717 ns          717 ns      1023142
BM_Lua_StringUpper/1000                                 2217 ns         2217 ns       305715
BM_Lua_StringUpper/10000                               15644 ns        15644 ns        42477
BM_FakeLua_StringUpper_TCC/10                            579 ns          579 ns      1215305
BM_FakeLua_StringUpper_TCC/100                          1048 ns         1048 ns       676528
BM_FakeLua_StringUpper_TCC/1000                         4243 ns         4243 ns       161448
BM_FakeLua_StringUpper_TCC/10000                       35185 ns        35184 ns        18865
BM_FakeLua_StringUpper_GCC/10                            561 ns          561 ns      1230634
BM_FakeLua_StringUpper_GCC/100                          1039 ns         1039 ns       679848
BM_FakeLua_StringUpper_GCC/1000                         4222 ns         4222 ns       159341
BM_FakeLua_StringUpper_GCC/10000                       35037 ns        35042 ns        20428
BM_CPP_StringByte/10                                    2.40 ns         2.40 ns    291503130
BM_CPP_StringByte/100                                   2.39 ns         2.39 ns    293083993
BM_CPP_StringByte/1000                                  2.43 ns         2.43 ns    293592175
BM_Lua_StringByte/10                                     223 ns          223 ns      3165090
BM_Lua_StringByte/100                                    395 ns          395 ns      1778369
BM_Lua_StringByte/1000                                   703 ns          703 ns       993678
BM_FakeLua_StringByte_TCC/10                             599 ns          599 ns      1164394
BM_FakeLua_StringByte_TCC/100                            661 ns          661 ns      1062358
BM_FakeLua_StringByte_TCC/1000                           843 ns          843 ns       832446
BM_FakeLua_StringByte_GCC/10                             594 ns          594 ns      1186046
BM_FakeLua_StringByte_GCC/100                            652 ns          652 ns      1075317
BM_FakeLua_StringByte_GCC/1000                           854 ns          854 ns       831032
BM_CPP_StringChar/10                                    39.3 ns         39.3 ns     18452694
BM_CPP_StringChar/100                                    392 ns          392 ns      1800630
BM_CPP_StringChar/500                                   1794 ns         1794 ns       391008
BM_Lua_StringChar/10                                    3131 ns         3131 ns       220690
BM_Lua_StringChar/100                                  18150 ns        18150 ns        38223
BM_Lua_StringChar/500                                  82537 ns        82534 ns         8886
BM_FakeLua_StringChar_TCC/10                            9888 ns         9888 ns        70813
BM_FakeLua_StringChar_TCC/100                          90064 ns        90062 ns         7793
BM_FakeLua_StringChar_TCC/500                         440341 ns       440331 ns         1593
BM_FakeLua_StringChar_GCC/10                            8478 ns         8477 ns        83924
BM_FakeLua_StringChar_GCC/100                          75736 ns        75734 ns         9119
BM_FakeLua_StringChar_GCC/500                         377420 ns       377414 ns         1866
BM_CPP_StringFormat/10                                   104 ns          104 ns      6633283
BM_CPP_StringFormat/100                                 1073 ns         1073 ns       654918
BM_CPP_StringFormat/500                                 5726 ns         5726 ns       122420
BM_Lua_StringFormat/10                                  3374 ns         3374 ns       206280
BM_Lua_StringFormat/100                                32490 ns        32490 ns        21582
BM_Lua_StringFormat/500                               169496 ns       169490 ns         4231
BM_FakeLua_StringFormat_TCC/10                          7505 ns         7505 ns        93652
BM_FakeLua_StringFormat_TCC/100                        72496 ns        72495 ns         9148
BM_FakeLua_StringFormat_TCC/500                       362207 ns       362201 ns         1952
BM_FakeLua_StringFormat_GCC/10                          7121 ns         7121 ns        97855
BM_FakeLua_StringFormat_GCC/100                        69386 ns        69385 ns        10154
BM_FakeLua_StringFormat_GCC/500                       363436 ns       363428 ns         1996
BM_CPP_StringFind/10                                    5.95 ns         5.95 ns    117224430
BM_CPP_StringFind/100                                   7.27 ns         7.27 ns     97286022
BM_CPP_StringFind/1000                                  19.0 ns         19.0 ns     36723023
BM_CPP_StringFind/10000                                  131 ns          131 ns      5479989
BM_Lua_StringFind/10                                     361 ns          361 ns      1912950
BM_Lua_StringFind/100                                    523 ns          523 ns      1450308
BM_Lua_StringFind/1000                                   785 ns          784 ns       852870
BM_Lua_StringFind/10000                                 2580 ns         2580 ns       274506
BM_FakeLua_StringFind_TCC/10                             898 ns          898 ns       751426
BM_FakeLua_StringFind_TCC/100                            912 ns          912 ns       732935
BM_FakeLua_StringFind_TCC/1000                          1218 ns         1218 ns       542871
BM_FakeLua_StringFind_TCC/10000                         2127 ns         2127 ns       329652
BM_FakeLua_StringFind_GCC/10                             873 ns          873 ns       798487
BM_FakeLua_StringFind_GCC/100                            901 ns          901 ns       771165
BM_FakeLua_StringFind_GCC/1000                          1194 ns         1194 ns       586240
BM_FakeLua_StringFind_GCC/10000                         2117 ns         2117 ns       332489
BM_CPP_StringGsub/10                                    94.1 ns         94.1 ns      7467784
BM_CPP_StringGsub/100                                    874 ns          874 ns       809925
BM_CPP_StringGsub/1000                                  8433 ns         8433 ns        82811
BM_Lua_StringGsub/10                                     819 ns          819 ns       841061
BM_Lua_StringGsub/100                                   4765 ns         4765 ns       146633
BM_Lua_StringGsub/1000                                 41781 ns        41780 ns        16662
BM_FakeLua_StringGsub_TCC/10                            4066 ns         4066 ns       173223
BM_FakeLua_StringGsub_TCC/100                          20620 ns        20620 ns        34183
BM_FakeLua_StringGsub_TCC/1000                        180038 ns       180027 ns         3885
BM_FakeLua_StringGsub_GCC/10                            4062 ns         4062 ns       171912
BM_FakeLua_StringGsub_GCC/100                          20142 ns        20141 ns        33817
BM_FakeLua_StringGsub_GCC/1000                        180090 ns       180084 ns         3870
BM_CPP_ToNumber/1                                       20.3 ns         20.3 ns     34540879
BM_Lua_ToNumber/1                                        198 ns          198 ns      3550813
BM_FakeLua_ToNumber_TCC/1                                643 ns          643 ns      1127325
BM_FakeLua_ToNumber_GCC/1                                613 ns          613 ns      1172773
BM_CPP_ToString/10                                       125 ns          124 ns      5657004
BM_CPP_ToString/100                                     2238 ns         2238 ns       318079
BM_CPP_ToString/500                                    11505 ns        11505 ns        60766
BM_Lua_ToString/10                                       492 ns          492 ns      1441661
BM_Lua_ToString/100                                      514 ns          514 ns      1000000
BM_Lua_ToString/500                                      509 ns          509 ns      1331600
BM_FakeLua_ToString_TCC/10                               556 ns          556 ns      1352558
BM_FakeLua_ToString_TCC/100                              532 ns          532 ns      1135566
BM_FakeLua_ToString_TCC/500                              522 ns          522 ns      1336091
BM_FakeLua_ToString_GCC/10                               513 ns          513 ns      1352147
BM_FakeLua_ToString_GCC/100                              523 ns          523 ns      1363697
BM_FakeLua_ToString_GCC/500                              512 ns          512 ns      1358145
BM_CPP_StringFindPattern/1000                          78932 ns        78930 ns         8804
BM_Lua_StringFindPattern/1000                         883956 ns       883929 ns          809
BM_FakeLua_StringFindPattern_TCC/1000                2287871 ns      2287838 ns          305
BM_FakeLua_StringFindPattern_GCC/1000                2238483 ns      2238407 ns          313
BM_CPP_StringGmatch/1000                              119471 ns       119466 ns         5860
BM_Lua_StringGmatch/1000                             1147726 ns      1147728 ns          638
BM_FakeLua_StringGmatch_TCC/1000                     2729516 ns      2729522 ns          245
BM_FakeLua_StringGmatch_GCC/1000                     2575468 ns      2575473 ns          272
BM_CPP_TableInsert/100                                   164 ns          164 ns      4319263
BM_CPP_TableInsert/500                                   495 ns          495 ns      1435682
BM_CPP_TableInsert/1000                                  935 ns          935 ns       798366
BM_CPP_TableInsert/5000                                 4192 ns         4192 ns       171970
BM_Lua_TableInsert/100                                 12118 ns        12117 ns        57044
BM_Lua_TableInsert/500                                 52168 ns        52165 ns        13599
BM_Lua_TableInsert/1000                               102273 ns       102272 ns         6849
BM_Lua_TableInsert/5000                               504263 ns       504258 ns         1373
BM_FakeLua_TableInsert_TCC/100                         13730 ns        13730 ns        51009
BM_FakeLua_TableInsert_TCC/500                         61016 ns        61013 ns        11453
BM_FakeLua_TableInsert_TCC/1000                       121330 ns       121329 ns         5746
BM_FakeLua_TableInsert_TCC/5000                       742623 ns       742612 ns          944
BM_FakeLua_TableInsert_GCC/100                          2392 ns         2392 ns       292599
BM_FakeLua_TableInsert_GCC/500                          9216 ns         9215 ns        75542
BM_FakeLua_TableInsert_GCC/1000                        18501 ns        18500 ns        38964
BM_FakeLua_TableInsert_GCC/5000                       120229 ns       120229 ns         5478
BM_CPP_TableRemove/100                                   126 ns          126 ns      5526505
BM_CPP_TableRemove/500                                   308 ns          308 ns      2273394
BM_CPP_TableRemove/1000                                  526 ns          526 ns      1325355
BM_CPP_TableRemove/5000                                 2303 ns         2303 ns       303642
BM_Lua_TableRemove/100                                 15973 ns        15973 ns        43049
BM_Lua_TableRemove/500                                 71964 ns        71962 ns         9586
BM_Lua_TableRemove/1000                               141747 ns       141747 ns         5114
BM_Lua_TableRemove/5000                               808997 ns       808971 ns          887
BM_FakeLua_TableRemove_TCC/100                         24225 ns        24224 ns        28592
BM_FakeLua_TableRemove_TCC/500                        110171 ns       110169 ns         6379
BM_FakeLua_TableRemove_TCC/1000                       218363 ns       218362 ns         3211
BM_FakeLua_TableRemove_TCC/5000                      1220703 ns      1220686 ns          572
BM_FakeLua_TableRemove_GCC/100                          4499 ns         4499 ns       155451
BM_FakeLua_TableRemove_GCC/500                         19688 ns        19688 ns        35556
BM_FakeLua_TableRemove_GCC/1000                        38936 ns        38935 ns        17956
BM_FakeLua_TableRemove_GCC/5000                       224693 ns       224679 ns         3100
BM_CPP_TableConcat/100                                  2237 ns         2237 ns       314761
BM_CPP_TableConcat/500                                 11537 ns        11537 ns        60514
BM_CPP_TableConcat/1000                                22752 ns        22752 ns        30943
BM_Lua_TableConcat/100                                 40269 ns        40268 ns        17428
BM_Lua_TableConcat/500                                207664 ns       207662 ns         3400
BM_Lua_TableConcat/1000                               407484 ns       407479 ns         1672
BM_FakeLua_TableConcat_TCC/100                         69549 ns        69546 ns        10061
BM_FakeLua_TableConcat_TCC/500                        338087 ns       338084 ns         2083
BM_FakeLua_TableConcat_TCC/1000                       671658 ns       671630 ns         1050
BM_FakeLua_TableConcat_GCC/100                         59136 ns        59136 ns        11692
BM_FakeLua_TableConcat_GCC/500                        294541 ns       294542 ns         2413
BM_FakeLua_TableConcat_GCC/1000                       592466 ns       592467 ns         1186
BM_CPP_TablePack/1                                     0.201 ns        0.201 ns   3496468637
BM_Lua_TablePack/1                                      1325 ns         1325 ns       522209
BM_FakeLua_TablePack_TCC/1                              1637 ns         1637 ns       428361
BM_FakeLua_TablePack_GCC/1                               509 ns          509 ns      1353885
BM_CPP_TableMove/100                                     263 ns          263 ns      2701182
BM_CPP_TableMove/500                                     639 ns          639 ns      1093129
BM_CPP_TableMove/1000                                   1109 ns         1109 ns       627062
BM_CPP_TableMove/5000                                   5081 ns         5081 ns       134124
BM_Lua_TableMove/100                                    8675 ns         8675 ns        78179
BM_Lua_TableMove/500                                   28499 ns        28498 ns        26827
BM_Lua_TableMove/1000                                  50304 ns        50302 ns        10000
BM_Lua_TableMove/5000                                 298681 ns       298681 ns         2713
BM_FakeLua_TableMove_TCC/100                           17933 ns        17933 ns        38864
BM_FakeLua_TableMove_TCC/500                           77110 ns        77108 ns         9073
BM_FakeLua_TableMove_TCC/1000                         153812 ns       153810 ns         4523
BM_FakeLua_TableMove_TCC/5000                         961660 ns       961647 ns          735
BM_FakeLua_TableMove_GCC/100                            7635 ns         7634 ns        91675
BM_FakeLua_TableMove_GCC/500                           31130 ns        31129 ns        22345
BM_FakeLua_TableMove_GCC/1000                          60490 ns        60490 ns        11477
BM_FakeLua_TableMove_GCC/5000                         383139 ns       383121 ns         1821
BM_CPP_TableSort/100                                    1530 ns         1530 ns       456716
BM_CPP_TableSort/500                                   10300 ns        10300 ns        67772
BM_CPP_TableSort/1000                                  26645 ns        26645 ns        26171
BM_Lua_TableSort/100                                   28514 ns        28513 ns        24685
BM_Lua_TableSort/500                                  169163 ns       169160 ns         4152
BM_Lua_TableSort/1000                                 386656 ns       386657 ns         1825
BM_FakeLua_TableSort_TCC/100                           22262 ns        22262 ns        31400
BM_FakeLua_TableSort_TCC/500                          106215 ns       106212 ns         6578
BM_FakeLua_TableSort_TCC/1000                         219189 ns       219181 ns         3220
BM_FakeLua_TableSort_GCC/100                            9653 ns         9653 ns        72302
BM_FakeLua_TableSort_GCC/500                           50162 ns        50160 ns        14079
BM_FakeLua_TableSort_GCC/1000                         108682 ns       108680 ns         6696
BM_CPP_TableCreate/1000                                  723 ns          723 ns       975158
BM_CPP_TableCreate/3000                                 1905 ns         1905 ns       365676
BM_CPP_TableCreate/5000                                 3173 ns         3172 ns       224077
BM_Lua_TableCreate/1000                                25098 ns        25097 ns        27478
BM_Lua_TableCreate/3000                                77200 ns        77196 ns         8687
BM_Lua_TableCreate/5000                               132461 ns       132458 ns         5334
BM_FakeLua_TableCreate_TCC/1000                       116806 ns       116804 ns         6005
BM_FakeLua_TableCreate_TCC/3000                       394034 ns       394030 ns         1777
BM_FakeLua_TableCreate_TCC/5000                       714630 ns       714615 ns          984
BM_FakeLua_TableCreate_GCC/1000                        16521 ns        16520 ns        42831
BM_FakeLua_TableCreate_GCC/3000                        60780 ns        60779 ns        11829
BM_FakeLua_TableCreate_GCC/5000                       113794 ns       113794 ns         6226
BM_CPP_HashInsert/100                                  14791 ns        14791 ns        44909
BM_CPP_HashInsert/500                                  81170 ns        81165 ns         8574
BM_CPP_HashInsert/1000                                165554 ns       165552 ns         4217
BM_Lua_HashInsert/100                                  52521 ns        52521 ns        13160
BM_Lua_HashInsert/500                                 271768 ns       271766 ns         2630
BM_Lua_HashInsert/1000                                544207 ns       544189 ns         1296
BM_FakeLua_HashInsert_TCC/100                          48351 ns        48349 ns        14870
BM_FakeLua_HashInsert_TCC/500                         227888 ns       227887 ns         2970
BM_FakeLua_HashInsert_TCC/1000                        553943 ns       553930 ns         1270
BM_FakeLua_HashInsert_GCC/100                          33415 ns        33415 ns        20308
BM_FakeLua_HashInsert_GCC/500                         164284 ns       164280 ns         4208
BM_FakeLua_HashInsert_GCC/1000                        342718 ns       342716 ns         2047
BM_CPP_HashLookup/100                                 168817 ns       168817 ns         4130
BM_CPP_HashLookup/500                                 202448 ns       202448 ns         3458
BM_CPP_HashLookup/1000                                237979 ns       237974 ns         2946
BM_Lua_HashLookup/100                                  41636 ns        41636 ns        17032
BM_Lua_HashLookup/500                                 217220 ns       217220 ns         3273
BM_Lua_HashLookup/1000                                446199 ns       446194 ns         1612
BM_FakeLua_HashLookup_TCC/100                          38179 ns        38177 ns        18426
BM_FakeLua_HashLookup_TCC/500                         201308 ns       201305 ns         3481
BM_FakeLua_HashLookup_TCC/1000                        411160 ns       411153 ns         1701
BM_FakeLua_HashLookup_GCC/100                          30858 ns        30857 ns        22567
BM_FakeLua_HashLookup_GCC/500                         155474 ns       155468 ns         4513
BM_FakeLua_HashLookup_GCC/1000                        312845 ns       312841 ns         2236
BM_CPP_NestedTable/1000                                 6705 ns         6705 ns       108543
BM_CPP_NestedTable/10000                               62760 ns        62760 ns        11328
BM_Lua_NestedTable/1000                               240516 ns       240508 ns         2858
BM_Lua_NestedTable/10000                             2371001 ns      2371006 ns          294
BM_FakeLua_NestedTable_TCC/1000                       460111 ns       460112 ns         1457
BM_FakeLua_NestedTable_TCC/10000                     4710622 ns      4710581 ns          152
BM_FakeLua_NestedTable_GCC/1000                       102850 ns       102848 ns         6792
BM_FakeLua_NestedTable_GCC/10000                     1051592 ns      1051562 ns          680
```
