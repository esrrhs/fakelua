#include "benchmark/benchmark.h"
#include "fakelua.h"
#include "util/common.h"

using namespace fakelua;

static void load_fakelua_file(fakelua_state_ptr L, const std::string &file) {
    try {
        L->compile_file(file, {.debug_mode = false});
    } catch (...) {
        L->compile_file("bin/" + file, {.debug_mode = false});
    }
}

struct FakeLuaGlobalIni {
    FakeLuaGlobalIni() {
        L = fakelua_newstate();
        load_fakelua_file(L, "bench_algo/fibonacci.lua");
    }
    fakelua_state_ptr L;
};

static FakeLuaGlobalIni fakelua_global_ini;

static void BM_fakelua_fibonacci(benchmark::State &state) {
    int ret = 0;
    for (auto _: state) {
        fakelua_global_ini.L->call("fibonacci", std::tie(ret), 30);
    }
}

BENCHMARK(BM_fakelua_fibonacci);
