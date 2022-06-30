include(FetchContent)

set(BENCHMARK_VER 1.6.1)
set(BENCHMARK_URL https://mirrors.tencent.com/github.com/google/benchmark/archive/refs/tags/v${BENCHMARK_VER}.tar.gz)

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)

# 定义外部库的版本
FetchContent_Declare(
        com_github_benchmark
        URL ${BENCHMARK_URL}
        SOURCE_DIR $ENV{FAKELUA_HOME}/cmake_third_party/benchmark
)
FetchContent_MakeAvailable(com_github_benchmark)

include_directories($ENV{FAKELUA_HOME}/cmake_third_party/benchmark/include)
