cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

PROJECT(lua)

aux_source_directory(./ LUA_SRC_LIST)
include_directories(./)
include_directories(../src/)

add_executable(lua ${LUA_SRC_LIST})

target_link_libraries(lua PRIVATE -static fakelua gflags::gflags)
