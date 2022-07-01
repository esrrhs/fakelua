cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(fakelua CXX)

# place binaries and libraries according to GNU standards
include(GNUInstallDirs)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})

set(ENV{FAKELUA_HOME} "${CMAKE_CURRENT_SOURCE_DIR}/")

option(USE_ASAN "use asan" OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unused-local-typedefs -Wno-unused-function -std=c++11 -DNDEBUG -O2 -g3 -ggdb -fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -DNDEBUG -O2 -g3 -ggdb -fPIC")
if (USE_ASAN)
    message("build with asan")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
endif ()

find_program(CCACHE_PROGRAM ccache)
if (CCACHE_PROGRAM)
    message("build with ccache")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif ()

include(cmake/gflags.cmake)
include(cmake/gperftools.cmake)
include(cmake/benchmark.cmake)
include(cmake/googletest.cmake)
include(cmake/glog.cmake)

add_subdirectory(src)
add_subdirectory(cmd)

enable_testing()
add_subdirectory(test)
add_subdirectory(benchmark)
