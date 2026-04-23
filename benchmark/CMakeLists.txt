cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(benchmark)

#add benchmark
CPMAddPackage(
        NAME benchmark
        GITHUB_REPOSITORY google/benchmark
        GIT_TAG v1.9.4
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

aux_source_directory(./ BENCHMARK_SRC_LIST)
include_directories(./)
include_directories(../include)
include_directories(../src)
include_directories(../src/platform)

find_package(PkgConfig QUIET)
if (PkgConfig_FOUND)
    # Try different Lua versions
    pkg_check_modules(LUA_BENCH lua5.4)
    if (NOT LUA_BENCH_FOUND)
        pkg_check_modules(LUA_BENCH lua-5.4)
    endif ()
    if (NOT LUA_BENCH_FOUND)
        pkg_check_modules(LUA_BENCH lua5.5)
    endif ()
    if (NOT LUA_BENCH_FOUND)
        pkg_check_modules(LUA_BENCH lua-5.5)
    endif ()
    if (NOT LUA_BENCH_FOUND)
        pkg_check_modules(LUA_BENCH lua)
    endif ()
endif ()
if (LUA_BENCH_FOUND)
    include_directories(${LUA_BENCH_INCLUDE_DIRS})
else ()
    include_directories(/usr/include/lua5.4)
    include_directories(/usr/local/include/lua5.4)
    # macOS Homebrew paths (Lua 5.5 or 5.4)
    if (APPLE)
        include_directories(/opt/homebrew/include/lua)
        include_directories(/opt/homebrew/include/lua5.5)
        include_directories(/opt/homebrew/include/lua5.4)
        include_directories(/usr/local/opt/lua/include)
    endif ()
endif ()

IF (WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--export-all-symbols")
ELSEIF (APPLE)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-export_dynamic")
ELSE ()
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-E")
ENDIF ()

add_executable(bench_mark ${BENCHMARK_SRC_LIST})

if (NOT WIN32 AND NOT APPLE)
    target_link_libraries(bench_mark PRIVATE dl)
endif ()
if (LUA_BENCH_FOUND)
    target_link_libraries(bench_mark PRIVATE fakelua benchmark ${LUA_BENCH_LIBRARIES})
else ()
    target_link_libraries(bench_mark PRIVATE fakelua benchmark lua)
endif ()

IF (WIN32)
    add_custom_command(
            TARGET bench_mark
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_BINARY_DIR}/third_party/tinycc/ ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}/
    )
ENDIF ()
