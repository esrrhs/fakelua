# Benchmarks

Bytecode VM only (no JIT). Numbers below are from a **release** build (`-O3 -flto`, `FAKE_RELEASE=ON`).

```bash
./build.sh release
./bin/fake_bench --benchmark_min_time=0.5s
# or: cd benchmark && ./benchmark.sh --benchmark_min_time=0.5s
```

CI uses `--benchmark_min_time=0.1s` as a smoke run; do not treat those times as the scoreboard.

Cross-language scripts (`loop` / `prime` / `string`) stay next to this file for a Lua comparison.

## Machine

- AMD EPYC 7551, 2.0 GHz, 16 vCPU (KVM)
- Linux, Lua 5.4.7, fake 1.5
- Date: 2026-09-11
- Load average during the run was ~4–5; treat figures as ±10%

## Microbenches (`fake_bench`)

Each `BM_*` parses once, then `fkrun`s a `loop(n)` (or `primes(n)`). **inner iter/s** is `n` work items per call (the `for` body), except Parse.

| Benchmark | n | time / `fkrun` | inner iter/s | What it measures |
|-----------|--:|---------------:|-------------:|------------------|
| ForLoop | 10000 | 45.1 μs | 222 M | C-style `for` + `c++` (fast path) |
| RangeFor | 10000 | 45.2 μs | 221 M | `for i = 0 -> n` (`OPCODE_FOR`) |
| Call | 10000 | 46.2 μs | 216 M | Leaf `add(s, 1)` in a hot loop |
| FoldedAdd | 10000 | 45.5 μs | 220 M | `s + (1+2*3-4)` after SCCP |
| DeadBranch | 10000 | 45.1 μs | 222 M | `if false then i*i*i` removed |
| ArrayWrite | 10000 | 125 μs | 80.1 M | Packed `a[i] = i` |
| MapWrite | 10000 | 4.51 ms | 2.22 M | Hash `m[i] = i` |
| StringConcat | 1000 | 235 μs | 4.26 M | `s = s.."x"` bump concat |
| Gvn | 10000 | 680 μs | 14.7 M | `(i+1)*(i+1)` twice per iter |
| Switch | 10000 | 1.46 ms | 6.85 M | `switch i % 4` |
| Recurse | 10000 | 13.4 ms | 0.745 M | `rec(16)` per iter (~12 M frames/s) |
| Prime | 400 | 580 μs | — | Nested `%` + calls (`primes(400)`) |
| Parse | — | 359 μs | — | `newfake` + parse/opt small script |
| ParseHeavy | — | 870 μs | — | Same with const/SCCP/several funcs |

## Script workloads vs Lua

Same programs as `loop.fk` / `loop.lua` (and prime, string). One-shot wall time, stdout discarded from the timer.

| Workload | Lua 5.4.7 | **Fake** |
|----------|----------:|---------:|
| Loop `n = 1e8` increments | 0.678 s | **0.447 s** |
| String `n = 1e6` concat | 0.736 s | **0.476 s** |
| Prime `n = 1e5` trial division | **6.34 s** | 17.2 s |

Range bounds differ by one iteration (`2 -> n` is end-exclusive; Lua `for i = 2, n` is inclusive). Fake does **less** inner work, so the 2.7× gap is per-iteration cost, not extra trials.

## Why Prime loses to Lua

`isprime` is the kernel: for each `n`, trial-divide from 2 until the first factor (or `n`). Almost all time is that inner `%` loop, not the ~1e5 calls to `isprime`. SSA cannot fold it (`n` and `i` are not constant). Leaf inlining does not apply (`isprime` is a loop, not `return a+b`).

Lua 5.4 inner loop (`luac -l`):

```
FORPREP
  MOD / EQI 0 / JMP     -- integer % and compare-to-0
FORLOOP                 -- integer increment + compare in one op
```

Fake inner loop (`fkdumpfunc isprime`):

```
DIVIDE_MOD n, i         -- always double
NOT_JNE                 -- if not (n % i)
RETURN false
FOR i, nn, 1            -- bytecode FOR, not the C tight loop
```

What that costs:

1. **Numbers are `double` (`variant::REAL`).** `%` is `(int64_t)left % (int64_t)right` every time, plus type and divide-by-zero checks (`V_DIVIDE_MOD`). Lua 5.4 keeps these locals as integers (`LOADI` / `ADDI` / `MOD` / `EQI` / `FORLOOP`).
2. **`vm_for_tight` does not fire.** That C loop only matches `c++`, `s = s+x`, and `a[i] = i`. A body of `DIVIDE_MOD` + `NOT_JNE` + `RETURN` stays in the computed-goto dispatcher.
3. **Lua’s `FORLOOP` is one specialized integer instruction.** Fake `OPCODE_FOR` still decodes three operands, adds doubles, compares, then jumps back to `DIVIDE_MOD`.
4. **`PLUS` has a REAL fast path; `DIVIDE_MOD` does not** — it always goes through the assert macros.

Closing the gap would mean integer-tagged arithmetic (or a `%` fast path like `PLUS`) and/or a tight C loop for “mod + compare + for”. Neither exists today. Changing the *algorithm* to `sqrt(n)` would speed both languages and hide the VM gap; the bench is meant to stress `%`/call, so it stays naive.

## Conclusions

1. **Tight integer loops are the fast path.** C-style `for` and range-for are the same (~220 M iter/s). The old `loop.fk` workload beats Lua 5.4 on this machine (~1.5×).
2. **Leaf calls are inlined.** `BM_Call` matches `BM_ForLoop`, so `return a+b` does not pay a full call.
3. **SSA SCCP / DCE show up in the hot loop.** `FoldedAdd` and `DeadBranch` match `ForLoop`: a folded `1+2*3-4` and a constant-false body are free. (Divide-by-zero in dead code still errors at compile time; the bench uses a multiply.)
4. **GVN does not turn a loop into a closed form.** Redundant `(i+1)*(i+1)` stays in the interpreter (~15 M iter/s, ~15× slower than `c++`).
5. **Containers:** packed arrays ~80 M writes/s; maps ~2.2 M/s (~35×). Maps are the hash table, not a packed array.
6. **Strings:** bump concat, not interned. Fake is ahead of Lua on this pattern (~1.5×).
7. **`switch` and recursion are not on the loop fast path.** Switch is ~7 M iter/s; a 16-deep rec chain is ~12 M frames/s.
8. **Prime-style bytecode still favors Lua (~2.7×).** See [Why Prime loses to Lua](#why-prime-loses-to-lua). Fast paths help loops / arrays / concat, not general `%`.
9. **Parse + SSA optimize is cheap** next to a long run (~0.4–0.9 ms per `newfake`+parse).

No JIT; infinite loops hang `fkrun`. Debug (`-O0`) numbers are not comparable.

## Case list (C++ microbenches)

Added on top of the original ForLoop / Call / ArrayWrite / StringConcat / Parse:

- **RangeFor** — `loop.fk` iterator shape
- **MapWrite** — `map()[i] = i`
- **FoldedAdd / DeadBranch / Gvn** — optimizer effects vs ForLoop
- **Switch / Recurse / Prime** — control flow and the classic integer kernel
- **ParseHeavy** — compile+optimize cost with const and SCCP
