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
if (APPLE)
    find_path(FAKELUA_LUA_INCLUDE_DIR
            NAMES lua.hpp
            PATH_SUFFIXES lua5.4 lua
    )
    if (NOT FAKELUA_LUA_INCLUDE_DIR)
        message(FATAL_ERROR "Lua headers not found. Please install Lua development package (e.g. brew install lua).")
    endif ()
    include_directories(${FAKELUA_LUA_INCLUDE_DIR})
    find_library(FAKELUA_LUA_LIBRARY
            NAMES lua lua5.4
    )
    if (NOT FAKELUA_LUA_LIBRARY)
        message(FATAL_ERROR "Lua library not found. Please install Lua development package (e.g. brew install lua).")
    endif ()
else ()
    include_directories(/usr/include/lua5.4)
endif ()

IF (WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--export-all-symbols")
ELSEIF (UNIX AND NOT APPLE)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-E")
ENDIF ()

add_executable(bench_mark ${BENCHMARK_SRC_LIST})

if (CMAKE_DL_LIBS)
    target_link_libraries(bench_mark PRIVATE ${CMAKE_DL_LIBS})
endif ()
if (APPLE)
    target_link_libraries(bench_mark PRIVATE fakelua benchmark ${FAKELUA_LUA_LIBRARY})
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
