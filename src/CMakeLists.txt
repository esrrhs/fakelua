cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(libfakelua)

aux_source_directory(./internal FAKELUA_SRC_LIST)
include_directories(./internal)
include_directories(./)
add_library(fakelua SHARED ${FAKELUA_SRC_LIST})
target_compile_options(fakelua PRIVATE -std=c++2a -g3 -Wall)
