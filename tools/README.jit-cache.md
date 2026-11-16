# GCC shared-library cache benchmark

Build the project in Release, then build this standalone public-API harness:

```sh
c++ -O3 -std=c++23 -Iinclude tools/benchmark_jit_cache.cpp -Lbuild/lib -lfakelua \
  -Wl,-rpath,"$PWD/build/lib" -o /tmp/benchmark-jit-cache
/tmp/benchmark-jit-cache off
/tmp/benchmark-jit-cache cold /tmp/fakelua-cache-benchmark-cold
/tmp/benchmark-jit-cache warm /tmp/fakelua-cache-benchmark-warm
/tmp/benchmark-jit-cache warm /tmp/fakelua-cache-benchmark-warm  # independent process
```

Use dedicated, disposable benchmark directories: `cold` removes its supplied
cache directory before each sample (its basename must start with `fakelua-cache-benchmark-`). Do not point it at a real application's cache.
Each run prints nine samples, in milliseconds; report medians (exclude the first
sample when reporting same-process warm compilation). Repeat on an idle host;
compare steady-state columns with both caching off and on. This exercises native
arithmetic, dynamic lookup of a 16-field specialized table, string comparisons, and varying interned-string table keys.
It is not a claim about all workloads. The frontend still runs on cache hits.

For an unmodified older source baseline, compile this same file with
`-DFAKELUA_CACHE_BASELINE` and that baseline's headers/library. This excludes only
configuration of the new cache API, not any measured Lua code or loop.
