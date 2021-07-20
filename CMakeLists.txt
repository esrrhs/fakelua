cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(fakelua CXX)

# place binaries and libraries according to GNU standards
include(GNUInstallDirs)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})

IF (CMAKE_BUILD_TYPE STREQUAL "release")
    message("build release fakelua")
ELSE()
    message("build debug fakelua")
ENDIF()

add_subdirectory(src)
add_subdirectory(cmd)

enable_testing()
add_subdirectory(test)
add_subdirectory(benchmark)
