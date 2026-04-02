#include "benchmark/benchmark.h"
#include "fakelua.h"
#include "util/common.h"

using namespace fakelua;

static void LoadFakeluaFile(State *L, const std::string &file) {
    try {
        CompileFile(L, file, {.debug_mode = false});
    } catch (...) {
        CompileFile(L, "bin/" + file, {.debug_mode = false});
    }
}

struct FakeLuaGlobalIni {
    FakeLuaGlobalIni() {
        L = FakeluaNewState();
        LoadFakeluaFile(L, "bench_algo/fibonacci.lua");
    }
    ~FakeLuaGlobalIni() {
        FakeluaDeleteState(L);
    }
    State *L;
};

static FakeLuaGlobalIni fakelua_global_ini;

static void BM_fakelua_fibonacci(benchmark::State &state) {
    for (auto _: state) {
        int ret = 0;
        Call(fakelua_global_ini.L, "main", ret, 30);
        if (ret != 832040) {
            throw std::runtime_error("fakelua Fibonacci result is incorrect: " + std::to_string(ret));
        }
    }
}

BENCHMARK(BM_fakelua_fibonacci);
