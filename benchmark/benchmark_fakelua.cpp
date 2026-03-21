#include "benchmark/benchmark.h"
#include "fakelua.h"
#include "util/common.h"

using namespace fakelua;

static void LoadFakeluaFile(FakeluaStatePtr L, const std::string &file) {
    try {
        L->CompileFile(file, {.debug_mode = false});
    } catch (...) {
        L->CompileFile("bin/" + file, {.debug_mode = false});
    }
}

struct FakeLuaGlobalIni {
    FakeLuaGlobalIni() {
        L = FakeluaNewstate();
        LoadFakeluaFile(L, "bench_algo/fibonacci.lua");
    }
    FakeluaStatePtr L;
};

static FakeLuaGlobalIni fakelua_global_ini;

static void BM_fakelua_fibonacci(benchmark::State &state) {
    for (auto _: state) {
        int ret = 0;
        fakelua_global_ini.L->call("main", std::tie(ret), 30);
        if (ret != 832040) {
            throw std::runtime_error("fakelua Fibonacci result is incorrect: " + std::to_string(ret));
        }
    }
}

BENCHMARK(BM_fakelua_fibonacci);
