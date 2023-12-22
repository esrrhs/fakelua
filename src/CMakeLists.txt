cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(libfakelua)

#add magic_enum
CPMAddPackage(
        NAME magic_enum
        GITHUB_REPOSITORY Neargye/magic_enum
        VERSION 0.9.5
)
include_directories(${magic_enum_SOURCE_DIR}/include/magic_enum)

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
add_library(fakelua SHARED ${FAKELUA_SRC_LIST})

IF (WIN32)
    set(TARGET_LIB gccjit)
ENDIF ()

if (USE_COV)
    set(TARGET_LIB ${TARGET_LIB} gcov)
endif ()

target_link_libraries(fakelua ${TARGET_LIB})
