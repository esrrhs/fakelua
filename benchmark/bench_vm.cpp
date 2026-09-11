#include "benchmark/benchmark.h"
#include "fake-inc.h"

#include <cstdlib>
#include <cstdio>

namespace {

fake *make(const char *src) {
    fake *fk = newfake();
    if (!fkparsestr(fk, src)) {
        std::fprintf(stderr, "parse failed: %s\n%s\n", fkerrorstr(fk), src);
        std::abort();
    }
    return fk;
}

void run_named(benchmark::State &st, fake *fk, const char *func) {
    int n = static_cast<int>(st.range(0));
    for (auto _ : st) {
        benchmark::DoNotOptimize(fkrun<int>(fk, func, n));
    }
    st.SetItemsProcessed(static_cast<int64_t>(st.iterations()) * n);
    delfake(fk);
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
    run_named(st, fk, "loop");
}

void BM_RangeFor(benchmark::State &st) {
    fake *fk = make(
            "func loop(n)\n"
            "	var c = 0\n"
            "	for var i = 0 -> n, 1 then\n"
            "		c++\n"
            "	end\n"
            "	return c\n"
            "end\n");
    run_named(st, fk, "loop");
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
    run_named(st, fk, "loop");
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
    run_named(st, fk, "loop");
}

void BM_MapWrite(benchmark::State &st) {
    fake *fk = make(
            "func loop(n)\n"
            "	var m = map()\n"
            "	for var i = 0, i < n, i++ then\n"
            "		m[i] = i\n"
            "	end\n"
            "	return size(m)\n"
            "end\n");
    run_named(st, fk, "loop");
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
    run_named(st, fk, "loop");
}

void BM_FoldedAdd(benchmark::State &st) {
    fake *fk = make(
            "func loop(n)\n"
            "	var s = 0\n"
            "	for var i = 0, i < n, i++ then\n"
            "		s = s + (1 + 2 * 3 - 4)\n"
            "	end\n"
            "	return s\n"
            "end\n");
    run_named(st, fk, "loop");
}

void BM_DeadBranch(benchmark::State &st) {
    fake *fk = make(
            "func loop(n)\n"
            "	var s = 0\n"
            "	for var i = 0, i < n, i++ then\n"
            "		if false then\n"
            "			s = s + i * i * i\n"
            "		end\n"
            "		s++\n"
            "	end\n"
            "	return s\n"
            "end\n");
    run_named(st, fk, "loop");
}

void BM_Gvn(benchmark::State &st) {
    fake *fk = make(
            "func loop(n)\n"
            "	var s = 0\n"
            "	for var i = 0, i < n, i++ then\n"
            "		s = s + (i + 1) * (i + 1) + (i + 1) * (i + 1)\n"
            "	end\n"
            "	return s\n"
            "end\n");
    run_named(st, fk, "loop");
}

void BM_Switch(benchmark::State &st) {
    fake *fk = make(
            "func loop(n)\n"
            "	var s = 0\n"
            "	for var i = 0, i < n, i++ then\n"
            "		switch i % 4\n"
            "			case 0 then\n"
            "				s++\n"
            "			case 1 then\n"
            "				s = s + 2\n"
            "			case 2 then\n"
            "				s = s + 3\n"
            "			case 3 then\n"
            "				s = s + 4\n"
            "			default\n"
            "				s = s + 5\n"
            "		end\n"
            "	end\n"
            "	return s\n"
            "end\n");
    run_named(st, fk, "loop");
}

void BM_Recurse(benchmark::State &st) {
    fake *fk = make(
            "func rec(k)\n"
            "	if k <= 1 then\n"
            "		return 1\n"
            "	end\n"
            "	return k + rec(k - 1)\n"
            "end\n"
            "func loop(n)\n"
            "	var s = 0\n"
            "	for var i = 0, i < n, i++ then\n"
            "		s = s + rec(16)\n"
            "	end\n"
            "	return s\n"
            "end\n");
    run_named(st, fk, "loop");
}

void BM_Prime(benchmark::State &st) {
    fake *fk = make(
            "func isprime(n)\n"
            "	for var i = 2 -> n, 1 then\n"
            "		if not (n % i) then\n"
            "			return false\n"
            "		end\n"
            "	end\n"
            "	return true\n"
            "end\n"
            "func primes(n)\n"
            "	var count = 0\n"
            "	for var i = 2 -> n, 1 then\n"
            "		if is isprime(i) then\n"
            "			count++\n"
            "		end\n"
            "	end\n"
            "	return count\n"
            "end\n");
    run_named(st, fk, "primes");
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

void BM_ParseHeavy(benchmark::State &st) {
    const char *src =
            "const c = 40\n"
            "func add(a, b)\n"
            "	return a + b\n"
            "end\n"
            "func absval(x)\n"
            "	if x < 0 then\n"
            "		return 0 - x\n"
            "	end\n"
            "	return x\n"
            "end\n"
            "func main()\n"
            "	var s = 0\n"
            "	for var i = 0, i < 20, i++ then\n"
            "		if i % 2 != 0 then\n"
            "			s = add(s, i * i + c)\n"
            "		else\n"
            "			s = add(s, absval(i - c))\n"
            "		end\n"
            "		if false then\n"
            "			s = s + i * i * i\n"
            "		end\n"
            "	end\n"
            "	return s + (1 + 2 * 3 - 4)\n"
            "end\n";
    for (auto _ : st) {
        fake *fk = newfake();
        bool ok = fkparsestr(fk, src);
        benchmark::DoNotOptimize(ok);
        if (!ok) {
            std::fprintf(stderr, "ParseHeavy failed: %s\n", fkerrorstr(fk));
            delfake(fk);
            std::abort();
        }
        delfake(fk);
    }
}

}  // namespace

BENCHMARK(BM_ForLoop)->Arg(1000)->Arg(10000);
BENCHMARK(BM_RangeFor)->Arg(1000)->Arg(10000);
BENCHMARK(BM_Call)->Arg(1000)->Arg(10000);
BENCHMARK(BM_ArrayWrite)->Arg(1000)->Arg(10000);
BENCHMARK(BM_MapWrite)->Arg(1000)->Arg(10000);
BENCHMARK(BM_StringConcat)->Arg(100)->Arg(1000);
BENCHMARK(BM_FoldedAdd)->Arg(1000)->Arg(10000);
BENCHMARK(BM_DeadBranch)->Arg(1000)->Arg(10000);
BENCHMARK(BM_Gvn)->Arg(1000)->Arg(10000);
BENCHMARK(BM_Switch)->Arg(1000)->Arg(10000);
BENCHMARK(BM_Recurse)->Arg(1000)->Arg(10000);
BENCHMARK(BM_Prime)->Arg(100)->Arg(400);
BENCHMARK(BM_Parse);
BENCHMARK(BM_ParseHeavy);

BENCHMARK_MAIN();
