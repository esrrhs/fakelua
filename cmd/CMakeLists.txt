cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

PROJECT(lua)

aux_source_directory(./ LUA_SRC_LIST)
include_directories(./)
include_directories(../src/)

add_executable(lua ${LUA_SRC_LIST})
target_compile_options(lua PRIVATE -std=c++2a -g3 -Wall -Werror)
target_link_libraries(lua fakelua)
