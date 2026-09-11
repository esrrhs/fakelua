#include "benchmark/benchmark.h"
#include "fake-inc.h"

namespace {

fake *make(const char *src) {
    fake *fk = newfake();
    if (!fkparsestr(fk, src)) {
        return fk;
    }
    return fk;
}

void BM_ForLoop(benchmark::State &st) {
    fake *fk = make(
            "func loop(n)\n"
            "	var c = 0\n"
            "	for var i = 0, i < n, i++ then\n"
            "		c++\n"
            "	end\n"
            "	return c\n"
            "end\n");
    int n = static_cast<int>(st.range(0));
    for (auto _ : st) {
        benchmark::DoNotOptimize(fkrun<int>(fk, "loop", n));
    }
    delfake(fk);
}

void BM_Call(benchmark::State &st) {
    fake *fk = make(
            "func add(a, b)\n"
            "	return a + b\n"
            "end\n"
            "func loop(n)\n"
            "	var s = 0\n"
            "	for var i = 0, i < n, i++ then\n"
            "		s = add(s, 1)\n"
            "	end\n"
            "	return s\n"
            "end\n");
    int n = static_cast<int>(st.range(0));
    for (auto _ : st) {
        benchmark::DoNotOptimize(fkrun<int>(fk, "loop", n));
    }
    delfake(fk);
}

void BM_ArrayWrite(benchmark::State &st) {
    fake *fk = make(
            "func loop(n)\n"
            "	var a = array()\n"
            "	for var i = 0, i < n, i++ then\n"
            "		a[i] = i\n"
            "	end\n"
            "	return size(a)\n"
            "end\n");
    int n = static_cast<int>(st.range(0));
    for (auto _ : st) {
        benchmark::DoNotOptimize(fkrun<int>(fk, "loop", n));
    }
    delfake(fk);
}

void BM_StringConcat(benchmark::State &st) {
    fake *fk = make(
            "func loop(n)\n"
            "	var s = \"\"\n"
            "	for var i = 0, i < n, i++ then\n"
            "		s = s..\"x\"\n"
            "	end\n"
            "	return size(s)\n"
            "end\n");
    int n = static_cast<int>(st.range(0));
    for (auto _ : st) {
        benchmark::DoNotOptimize(fkrun<int>(fk, "loop", n));
    }
    delfake(fk);
}

void BM_Parse(benchmark::State &st) {
    const char *src =
            "func add(a, b)\n"
            "	return a + b\n"
            "end\n"
            "func main()\n"
            "	var s = 0\n"
            "	for var i = 0, i < 10, i++ then\n"
            "		s = add(s, i)\n"
            "	end\n"
            "	return s\n"
            "end\n";
    for (auto _ : st) {
        fake *fk = newfake();
        benchmark::DoNotOptimize(fkparsestr(fk, src));
        delfake(fk);
    }
}

}  // namespace

BENCHMARK(BM_ForLoop)->Arg(1000)->Arg(10000);
BENCHMARK(BM_Call)->Arg(1000)->Arg(10000);
BENCHMARK(BM_ArrayWrite)->Arg(1000)->Arg(10000);
BENCHMARK(BM_StringConcat)->Arg(100)->Arg(1000);
BENCHMARK(BM_Parse);

BENCHMARK_MAIN();
