cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(libfakelua)

include(ExternalProject)
include(GNUInstallDirs)
include(ProcessorCount)

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

ProcessorCount(FAKELUA_TINYCC_BUILD_JOBS)
if (NOT FAKELUA_TINYCC_BUILD_JOBS GREATER 0)
    set(FAKELUA_TINYCC_BUILD_JOBS 1)
endif ()
set(FAKELUA_TINYCC_GIT_URL "https://repo.or.cz/tinycc.git" CACHE STRING "TinyCC git repository URL")
# Pinned to TinyCC mob commit validated by this project's Linux fakelua/flua builds.
set(FAKELUA_TINYCC_GIT_TAG "98765e5ebc04ea464195fa80ea5e4bbdc70a29cc" CACHE STRING "TinyCC git branch/tag/commit")
CPMAddPackage(
        NAME tinycc_src
        GIT_REPOSITORY ${FAKELUA_TINYCC_GIT_URL}
        GIT_TAG ${FAKELUA_TINYCC_GIT_TAG}
        DOWNLOAD_ONLY YES
)
if (NOT tinycc_src_ADDED)
    message(FATAL_ERROR "Failed to fetch TinyCC source from ${FAKELUA_TINYCC_GIT_URL}")
endif ()
include_directories(${tinycc_src_SOURCE_DIR})

add_library(fakelua SHARED ${FAKELUA_SRC_LIST})

find_package(Threads REQUIRED)

set(TINYCC_BUILD_DIR "${CMAKE_BINARY_DIR}/third_party/tinycc_build")

if (WIN32)
    set(TINYCC_STATIC_LIB "${TINYCC_BUILD_DIR}/libtcc.a")
    ExternalProject_Add(tinycc_external
            SOURCE_DIR ${tinycc_src_SOURCE_DIR}
            BINARY_DIR ${TINYCC_BUILD_DIR}
            CONFIGURE_COMMAND bash <SOURCE_DIR>/configure --enable-static CC=gcc AR=ar
            BUILD_COMMAND make -j${FAKELUA_TINYCC_BUILD_JOBS} libtcc.a
            INSTALL_COMMAND ""
            BUILD_BYPRODUCTS ${TINYCC_STATIC_LIB}
    )
    add_library(tinycc_embedded STATIC IMPORTED GLOBAL)
    set_target_properties(tinycc_embedded PROPERTIES IMPORTED_LOCATION ${TINYCC_STATIC_LIB})
    add_dependencies(tinycc_embedded tinycc_external)
    add_dependencies(fakelua tinycc_external)
    set(TARGET_LIB ${TARGET_LIB} tinycc_embedded)
else()
    set(TINYCC_PREFIX_DIR "${CMAKE_BINARY_DIR}/third_party/tinycc")
    set(TINYCC_STATIC_LIB "${TINYCC_PREFIX_DIR}/${CMAKE_INSTALL_LIBDIR}/libtcc.a")
    ExternalProject_Add(tinycc_external
            SOURCE_DIR ${tinycc_src_SOURCE_DIR}
            BINARY_DIR ${TINYCC_BUILD_DIR}
            CONFIGURE_COMMAND ${tinycc_src_SOURCE_DIR}/configure --prefix=${TINYCC_PREFIX_DIR} --extra-cflags=-fPIC
            BUILD_COMMAND make -j${FAKELUA_TINYCC_BUILD_JOBS}
            INSTALL_COMMAND make install
            BUILD_BYPRODUCTS ${TINYCC_STATIC_LIB}
    )
    add_library(tinycc_embedded STATIC IMPORTED GLOBAL)
    set_target_properties(tinycc_embedded PROPERTIES IMPORTED_LOCATION ${TINYCC_STATIC_LIB})
    add_dependencies(tinycc_embedded tinycc_external)
    add_dependencies(fakelua tinycc_external)
    set(TARGET_LIB ${TARGET_LIB} tinycc_embedded Threads::Threads)
    set(TARGET_LIB ${TARGET_LIB} dl m)
endif ()

if (USE_COV)
    set(TARGET_LIB ${TARGET_LIB} gcov)
endif ()

target_link_libraries(fakelua ${TARGET_LIB})
