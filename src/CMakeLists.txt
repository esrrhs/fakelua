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
if (WIN32)
    include_directories(./platform/third_party/prebuilt/win_x64/tinycc/libtcc)
endif ()
add_library(fakelua SHARED ${FAKELUA_SRC_LIST})

find_package(Threads REQUIRED)

if (WIN32)
    set(TCC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/platform/third_party/prebuilt/win_x64/tinycc")
    set(TCC_IMPLIB "${CMAKE_CURRENT_BINARY_DIR}/libtcc.dll.a")
    add_custom_command(
        OUTPUT "${TCC_IMPLIB}"
        COMMAND dlltool -D "${TCC_DIR}/libtcc.dll" -d "${TCC_DIR}/libtcc/libtcc.def" -l "${TCC_IMPLIB}"
        DEPENDS "${TCC_DIR}/libtcc.dll" "${TCC_DIR}/libtcc/libtcc.def"
        COMMENT "Generating libtcc import library"
        VERBATIM
    )
    add_custom_target(tcc_implib DEPENDS "${TCC_IMPLIB}")
    add_dependencies(fakelua tcc_implib)
    set(TARGET_LIB ${TARGET_LIB} "${TCC_IMPLIB}")
else()
    set(TARGET_LIB ${TARGET_LIB} tcc Threads::Threads)
    set(TARGET_LIB ${TARGET_LIB} dl m)
endif ()

if (USE_COV)
    set(TARGET_LIB ${TARGET_LIB} gcov)
endif ()

target_link_libraries(fakelua ${TARGET_LIB})
