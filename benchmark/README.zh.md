# 基准

只有字节码 VM（没有 JIT）。下面的数字来自 **release** 编译（`-O3 -flto`，`FAKE_RELEASE=ON`）。

```bash
./build.sh release
./bin/fake_bench --benchmark_min_time=0.5s
# 或者：cd benchmark && ./benchmark.sh --benchmark_min_time=0.5s
```

CI 用 `--benchmark_min_time=0.1s` 做冒烟，那些时间不能当成绩。

同目录下的 `loop` / `prime` / `string` 脚本用来和 Lua 对比。

## 机器

- AMD EPYC 7551，2.0 GHz，16 vCPU（KVM）
- Linux，Lua 5.4.7，fake 1.5
- 日期：2026-09-11
- 跑的时候 load 大约 4–5，数字按 ±10% 看

## 微基准（`fake_bench`）

每个 `BM_*` 先 parse 一次，再反复 `fkrun` `loop(n)`（或 `primes(n)`）。**inner iter/s** 是每次调用里 `for` 走的次数，Parse 除外。

| 用例 | n | 每次 `fkrun` | inner iter/s | 测什么 |
|------|--:|-------------:|-------------:|--------|
| ForLoop | 10000 | 45.1 μs | 222 M | C 风格 `for` + `c++`（快路径） |
| RangeFor | 10000 | 45.2 μs | 221 M | `for i = 0 -> n`（`OPCODE_FOR`） |
| Call | 10000 | 46.2 μs | 216 M | 热循环里叶子 `add(s, 1)` |
| FoldedAdd | 10000 | 45.5 μs | 220 M | SCCP 折叠 `s + (1+2*3-4)` |
| DeadBranch | 10000 | 45.1 μs | 222 M | `if false then i*i*i` 被删掉 |
| ArrayWrite | 10000 | 125 μs | 80.1 M | packed `a[i] = i` |
| MapWrite | 10000 | 4.51 ms | 2.22 M | 哈希 `m[i] = i` |
| StringConcat | 1000 | 235 μs | 4.26 M | `s = s.."x"` bump 拼接 |
| Gvn | 10000 | 680 μs | 14.7 M | 每轮两次 `(i+1)*(i+1)` |
| Switch | 10000 | 1.46 ms | 6.85 M | `switch i % 4` |
| Recurse | 10000 | 13.4 ms | 0.745 M | 每轮 `rec(16)`（约 12 M 帧/s） |
| Prime | 400 | 580 μs | — | 嵌套 `%` + 调用（`primes(400)`） |
| Parse | — | 359 μs | — | `newfake` + 小脚本 parse/opt |
| ParseHeavy | — | 870 μs | — | 带 const/SCCP、多个函数 |

## 脚本负载 vs Lua

和 `loop.fk` / `loop.lua`（以及 prime、string）同一套程序。一次性墙钟，计时不含看 stdout。

| 负载 | Lua 5.4.7 | **Fake** |
|------|----------:|---------:|
| Loop `n = 1e8` 自增 | 0.678 s | **0.447 s** |
| String `n = 1e6` 拼接 | 0.736 s | **0.476 s** |
| Prime `n = 1e5` 试除 | **6.34 s** | 17.2 s |

循环边界差 1 次（`2 -> n` 不含右端；Lua `for i = 2, n` 含）。Fake **少做**内层试除，所以 2.7 倍差距是每次迭代更贵，不是多跑了圈。

## 为什么 Prime 跑不过 Lua

`isprime` 是核：对每个 `n` 从 2 试除到第一个因子（或到 `n`）。时间几乎全在内层 `%` 循环上，不在那约 1e5 次 `isprime` 调用。SSA 折不了（`n`、`i` 不是常量）。叶子内联也不适用（`isprime` 是循环，不是 `return a+b`）。

Lua 5.4 内层（`luac -l`）：

```
FORPREP
  MOD / EQI 0 / JMP     -- 整数 % 和与 0 比较
FORLOOP                 -- 一条指令里整数自增 + 比较
```

Fake 内层（`fkdumpfunc isprime`）：

```
DIVIDE_MOD n, i         -- 一律 double
NOT_JNE                 -- if not (n % i)
RETURN false
FOR i, nn, 1            -- 字节码 FOR，不是 C 紧循环
```

贵在哪里：

1. **数都是 `double`（`variant::REAL`）。** `%` 每次都是 `(int64_t)left % (int64_t)right`，再加类型检查和除零检查（`V_DIVIDE_MOD`）。Lua 5.4 这些局部量是整数（`LOADI` / `ADDI` / `MOD` / `EQI` / `FORLOOP`）。
2. **`vm_for_tight` 匹配不上。** 那段 C 循环只认 `c++`、`s = s+x`、`a[i] = i`。`DIVIDE_MOD` + `NOT_JNE` + `RETURN` 只能走 computed-goto 分发。
3. **Lua 的 `FORLOOP` 是一条专用整数指令。** Fake 的 `OPCODE_FOR` 仍要解码三个操作数、double 加减比较，再跳回 `DIVIDE_MOD`。
4. **`PLUS` 有 REAL 快路径，`DIVIDE_MOD` 没有**，每次都走 assert 宏。

要追上，需要带整数标记的算术（或像 `PLUS` 那样给 `%` 开快路径），和/或给「取模 + 比较 + for」写紧循环。现在都没有。把算法改成 `sqrt(n)` 两边都会变快，VM 差距会被盖住；这个 bench 就是为了压 `%`/call，所以保持朴素试除。

## 结论

1. **整数紧循环是快路径。** C 风格 `for` 和 range-for 一样（约 220 M iter/s）。这台机器上旧的 `loop.fk` 比 Lua 5.4 快约 1.5 倍。
2. **叶子调用被内联。** `BM_Call` 和 `BM_ForLoop` 持平，`return a+b` 不付完整调用价。
3. **SSA 的 SCCP / DCE 在热循环里看得见。** `FoldedAdd`、`DeadBranch` 和 `ForLoop` 一样：折叠后的 `1+2*3-4` 和恒假分支是零成本。（死代码里的除零仍会在编译期报错，这里用的是乘法。）
4. **GVN 不会把循环收成闭式。** 重复的 `(i+1)*(i+1)` 还在解释器里跑（约 15 M iter/s，比 `c++` 慢约 15 倍）。
5. **容器：** packed 数组约 80 M 写入/s，map 约 2.2 M/s（约 35 倍）。map 是哈希表，不是 packed 数组。
6. **字符串：** bump 拼接、不 intern。这条 concat 上 Fake 比 Lua 快约 1.5 倍。
7. **`switch` 和递归不在循环快路径上。** switch 约 7 M iter/s；16 层 rec 约 12 M 帧/s。
8. **质数这种字节码仍是 Lua 强（约 2.7 倍）。** 见 [为什么 Prime 跑不过 Lua](#为什么-prime-跑不过-lua)。快路径更帮循环 / 数组 / 拼接，帮不上一般的 `%`。
9. **Parse + SSA 优化相对一次长跑很便宜**（每次 `newfake`+parse 约 0.4–0.9 ms）。

没有 JIT；死循环会把 `fkrun` 挂住。Debug（`-O0`）的数字不能对比。

## 用例列表（C++ 微基准）

在原来的 ForLoop / Call / ArrayWrite / StringConcat / Parse 之外加了：

- **RangeFor** — `loop.fk` 那种迭代
- **MapWrite** — `map()[i] = i`
- **FoldedAdd / DeadBranch / Gvn** — 相对 ForLoop 看优化器
- **Switch / Recurse / Prime** — 控制流和经典整数核
- **ParseHeavy** — 带 const 和 SCCP 的编译+优化成本
