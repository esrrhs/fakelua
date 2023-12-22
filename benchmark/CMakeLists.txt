cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(benchmark)

#add benchmark
CPMAddPackage(
        NAME benchmark
        GITHUB_REPOSITORY google/benchmark
        GIT_TAG v1.8.3
        OPTIONS
        "BENCHMARK_ENABLE_TESTING Off"
        "BENCHMARK_ENABLE_GTEST_TESTS Off"
        "BENCHMARK_ENABLE_ASSEMBLY_TESTS Off"
        "BENCHMARK_ENABLE_EXCEPTIONS Off"
        "BENCHMARK_ENABLE_INSTALL Off"
        "BENCHMARK_ENABLE_LTO Off"
        "BENCHMARK_ENABLE_NO_EXCEPTIONS Off"
        "BENCHMARK_ENABLE_TESTING Off"
        "BENCHMARK_ENABLE_TESTING Off"
        "BENCHMARK_USE_LIBCXX Off"
        "BENCHMARK_USE_LIBCXXABI Off"
        "BENCHMARK_USE_SANITIZER Off"
        "BENCHMARK_USE_VALGRIND Off"
        "BENCHMARK_DOWNLOAD_DEPENDENCIES Off"
        "BENCHMARK_ENABLE_GTEST_TESTS Off"
        "BENCHMARK_ENABLE_EXCEPTIONS Off"
        "BENCHMARK_ENABLE_INSTALL Off"
        "BENCHMARK_ENABLE_LTO Off"
        "BENCHMARK_ENABLE_NO_EXCEPTIONS Off"
        "BENCHMARK_ENABLE_TESTING Off"
        "BENCHMARK_ENABLE_TESTING Off"
        "BENCHMARK_USE_LIBCXX Off"
        "BENCHMARK_USE_LIBCXXABI Off"
        "BENCHMARK_USE_SANITIZER Off"
        "BENCHMARK_USE_VALGRIND Off"
        "BENCHMARK_DOWNLOAD_DEPENDENCIES Off"
)

include_directories(${magic_enum_SOURCE_DIR}/include/magic_enum)

aux_source_directory(./ BENCHMARK_SRC_LIST)
include_directories(./)
include_directories(../include)
include_directories(../src)
include_directories(../src/platform)

IF (WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--export-all-symbols")
ELSE ()
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-E")
ENDIF ()

add_executable(bench_mark ${BENCHMARK_SRC_LIST})

target_link_libraries(bench_mark PRIVATE fakelua benchmark gccjit)

add_custom_command(
        TARGET bench_mark
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/benchmark.lua ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}/benchmark.lua
)
