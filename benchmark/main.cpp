#include "benchmark/benchmark.h"
#include <iostream>

int main(int argc, char **argv) {
    std::cout << "Starting benchmarks..." << std::endl;
    ::benchmark::Initialize(&argc, argv);
    if (::benchmark::ReportUnrecognizedArguments(argc, argv)) {
        return 1;
    }
    ::benchmark::RunSpecifiedBenchmarks();
    ::benchmark::Shutdown();
    return 0;
}
