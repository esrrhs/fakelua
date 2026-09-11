cmake_minimum_required(VERSION 3.16)

project(fake VERSION 1.5 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(FAKE_RELEASE "Build with optimizations (NDEBUG)" OFF)
option(FAKE_BUILD_TESTS "Build GoogleTest suite" ON)
option(FAKE_BUILD_BENCH "Build Google Benchmark binaries" ON)

if(FAKE_RELEASE AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

enable_testing()

add_subdirectory(src)

if(FAKE_BUILD_TESTS OR FAKE_BUILD_BENCH)
    include(${PROJECT_SOURCE_DIR}/cmake/ThirdParty.cmake)
endif()

add_subdirectory(test)

if(FAKE_BUILD_BENCH)
    add_subdirectory(benchmark)
endif()
