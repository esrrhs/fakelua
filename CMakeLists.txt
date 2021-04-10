cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(fakelua CXX)

# place binaries and libraries according to GNU standards
include(GNUInstallDirs)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})

add_subdirectory(src)

include(cmake/googletest.cmake)
fetch_googletest(${PROJECT_SOURCE_DIR}/cmake
        ${PROJECT_BINARY_DIR}/googletest)

set(BENCHMARK_ENABLE_TESTING OFF)
include(cmake/benchmark.cmake)
fetch_benchmark(${PROJECT_SOURCE_DIR}/cmake
        ${PROJECT_BINARY_DIR}/benchmark)

enable_testing()
add_subdirectory(test)
add_subdirectory(benchmark)
