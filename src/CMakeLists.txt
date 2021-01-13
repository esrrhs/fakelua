cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(libfakelua)

aux_source_directory(./ FAKELUA_SRC_LIST)
include_directories(./)
add_library(fakelua SHARED ${FAKELUA_SRC_LIST})
target_compile_options(fakelua PRIVATE -std=c++20 -g3 -Wall)
