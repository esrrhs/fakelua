cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

PROJECT(lua)

aux_source_directory(./ LUA_SRC_LIST)
include_directories(./)
include_directories(../include)

IF (WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--export-all-symbols")
ELSE ()
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-E")
ENDIF ()

add_executable(lua ${LUA_SRC_LIST})

target_link_libraries(lua PRIVATE fakelua gtest_main glog::glog gflags::gflags gccjit)
