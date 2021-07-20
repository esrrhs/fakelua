cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

PROJECT(lua)

aux_source_directory(./ LUA_SRC_LIST)
include_directories(./)
include_directories(../src/)

add_executable(lua ${LUA_SRC_LIST})

IF (CMAKE_BUILD_TYPE STREQUAL "release")
    target_compile_options(lua PRIVATE -std=c++2a -g3 -Wall -Werror -O3)
ELSE()
    target_compile_options(lua PRIVATE -std=c++2a -g3 -Wall -Werror -O0)
ENDIF()

target_link_libraries(lua fakelua)
