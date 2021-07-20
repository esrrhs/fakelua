cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(benchmark)

aux_source_directory(./ BENCHMARK_SRC_LIST)
include_directories(../src)
include_directories(../src/internal)

add_executable(bench_mark ${BENCHMARK_SRC_LIST})

IF (CMAKE_BUILD_TYPE STREQUAL "release")
    target_compile_options(bench_mark PRIVATE -std=c++2a -g3 -Wall -Werror -O3)
ELSE()
    target_compile_options(bench_mark PRIVATE -std=c++2a -g3 -Wall -Werror -O0)
ENDIF()

target_link_libraries(bench_mark fakelua)

add_custom_command(
        TARGET bench_mark
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/benchmark.lua ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}/benchmark.lua
)
