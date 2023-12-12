cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(benchmark)

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

target_link_libraries(bench_mark PRIVATE fakelua benchmark glog::glog gflags::gflags gccjit)

add_custom_command(
        TARGET bench_mark
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/benchmark.lua ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}/benchmark.lua
)
