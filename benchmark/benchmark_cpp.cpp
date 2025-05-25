#include "benchmark/benchmark.h"
#include "fakelua.h"
#include "util/common.h"

using namespace fakelua;

static int fibonacci(int n) {
    if (n <= 1) {
        return n;
    } else {
        return fibonacci(n - 1) + fibonacci(n - 2);
    }
}

static void BM_cpp_fibonacci(benchmark::State &state) {
    for (auto _: state) {
        int ret = 0;
        ret = fibonacci(30);
        if (ret != 832040) {
            throw std::runtime_error("C++ Fibonacci result is incorrect: " + std::to_string(ret));
        }
    }
}

BENCHMARK(BM_cpp_fibonacci);
