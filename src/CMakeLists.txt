cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(libfakelua)

aux_source_directory(compile/bison FAKELUA_SRC_LIST)
aux_source_directory(compile/flex FAKELUA_SRC_LIST)
aux_source_directory(./compile FAKELUA_SRC_LIST)
aux_source_directory(./state FAKELUA_SRC_LIST)
aux_source_directory(./util FAKELUA_SRC_LIST)
aux_source_directory(./fakelua FAKELUA_SRC_LIST)
aux_source_directory(./jit FAKELUA_SRC_LIST)
aux_source_directory(./var FAKELUA_SRC_LIST)
aux_source_directory(./ FAKELUA_SRC_LIST)
include_directories(./)
include_directories(../include)
include_directories(./platform)
add_library(fakelua STATIC ${FAKELUA_SRC_LIST})

IF (WIN32)
    set(TARGET_LIB glog::glog gflags::gflags gccjit)
ELSE ()
    set(TARGET_LIB profiler_static glog::glog gflags::gflags gccjit)
ENDIF ()

if (USE_COV)
    set(TARGET_LIB ${TARGET_LIB} gcov)
endif ()

target_link_libraries(fakelua ${TARGET_LIB})
