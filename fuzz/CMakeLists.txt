cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(fuzz)

include_directories(../include)
include_directories(../src)
include_directories(../src/platform)
include_directories(./)

# =========================================================================
# fuzz_bridge — compiled with GCC (needs <format> from GCC 15 libstdc++)
# =========================================================================
# fakelua.h requires C++20 <format>. clang 12 can't use GCC 15's libstdc++
# headers due to compatibility issues. So fuzz_bridge (the fakelua C++ glue)
# is compiled with GCC, and the actual libFuzzer fuzz targets (which only
# include the C-compatible fuzz_bridge.h) are built separately with clang
# via the build_fuzz.sh script.

add_library(fuzz_bridge STATIC fuzz_bridge.cpp fuzz_bridge.h)
target_link_libraries(fuzz_bridge PRIVATE fakelua)
