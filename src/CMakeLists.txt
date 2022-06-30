cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(libfakelua)

aux_source_directory(./ FAKELUA_SRC_LIST)
include_directories(./fakelua)
include_directories(./)
add_library(fakelua STATIC ${FAKELUA_SRC_LIST})
