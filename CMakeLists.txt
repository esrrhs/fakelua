cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(fakelua CXX)

set(CMAKE_CXX_STANDARD 20)

# place binaries and libraries according to GNU standards
include(GNUInstallDirs)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})

set(ENV{FAKELUA_HOME} "${CMAKE_CURRENT_SOURCE_DIR}/")


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unused-local-typedefs -Wno-unused-function -std=c++2a -DNDEBUG -g3 -ggdb -fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -DNDEBUG -O2 -g3 -ggdb -fPIC")

option(USE_ASAN "use asan" OFF)
if (USE_ASAN)
    message("build with asan")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
endif ()

option(USE_COV "use cov" OFF)
if (USE_COV)
    message("build with cov")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
endif ()

IF (WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
ELSE ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
ENDIF ()

find_program(CCACHE_PROGRAM ccache)
if (CCACHE_PROGRAM)
    message("build with ccache")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif ()

include(cmake/gflags.cmake)
IF (WIN32)
ELSE ()
    include(cmake/gperftools.cmake)
ENDIF ()
include(cmake/benchmark.cmake)
include(cmake/googletest.cmake)
include(cmake/glog.cmake)
include(cmake/magic_enum.cmake)

add_subdirectory(src)
add_subdirectory(cmd)

enable_testing()
add_subdirectory(test)
add_subdirectory(benchmark)
