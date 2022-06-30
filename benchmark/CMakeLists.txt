cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(benchmark)

aux_source_directory(./ BENCHMARK_SRC_LIST)
include_directories(./)
include_directories(../src)

add_executable(bench_mark ${BENCHMARK_SRC_LIST})

target_link_libraries(bench_mark fakelua benchmark_main)

add_custom_command(
        TARGET bench_mark
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/benchmark.lua ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}/benchmark.lua
)
