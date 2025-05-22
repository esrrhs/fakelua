#include "benchmark/benchmark.h"
#include "fakelua.h"
#include "util/common.h"

using namespace fakelua;

static fakelua_state_ptr load_fakelua_file(const std::string &file) {
    auto L = fakelua_newstate();
    try {
        L->compile_file(file, {.debug_mode = false});
    } catch (...) {
        L->compile_file("bin/" + file, {.debug_mode = false});
    }
    return L;
}

// static void BM_fakelua_fibonacci(benchmark::State &state) {
//     auto L = load_fakelua_file("algo/fibonacci.lua");
//     int ret = 0;
//     for (auto _: state) {
//         L->call("fibonacci", std::tie(ret), 30);
//     }
//     state.SetItemsProcessed(state.iterations());
//     state.SetLabel("fibonacci");
//     std::cout << "Fibonacci result: " << ret << std::endl;
// }
//
// BENCHMARK(BM_fakelua_fibonacci);
