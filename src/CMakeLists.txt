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
set(FAKELUA_TINYCC_GIT_URL "https://github.com/TinyCC/tinycc.git" CACHE STRING "TinyCC git repository URL")
set(FAKELUA_TINYCC_GIT_TAG "mob" CACHE STRING "TinyCC git branch/tag/commit")
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

set(TINYCC_PREFIX_DIR "${CMAKE_BINARY_DIR}/third_party/tinycc")
set(TINYCC_BUILD_DIR "${CMAKE_BINARY_DIR}/third_party/tinycc_build")
if (WIN32)
    # On Windows, TCC installs libtcc.a to PREFIX/libtcc/ instead of PREFIX/lib/
    set(TINYCC_STATIC_LIB "${TINYCC_PREFIX_DIR}/libtcc/libtcc.a")
else ()
    set(TINYCC_STATIC_LIB "${TINYCC_PREFIX_DIR}/lib/libtcc.a")
endif ()

if (WIN32)
    # On Windows (MSYS2/MinGW), TCC's configure auto-detects Windows and generates CONFIG_WIN32=yes.
    # TCC does not support true out-of-source builds, so we copy source to the build directory.
    # We must invoke configure/make through bash since MinGW Makefiles runs commands via CMD.
    find_program(BASH_EXECUTABLE bash PATHS C:/msys64/usr/bin C:/msys64/bin NO_DEFAULT_PATH)
    if (NOT BASH_EXECUTABLE)
        find_program(BASH_EXECUTABLE bash)
    endif ()
    if (NOT BASH_EXECUTABLE)
        message(FATAL_ERROR "bash not found. MSYS2 bash is required to build TinyCC on Windows.")
    endif ()
    # MSYSTEM=MINGW64 ensures bash login shell loads MinGW64 PATH (where gcc lives),
    # not the MSYS2 native environment which lacks gcc.
    set(MSYS2_BASH ${CMAKE_COMMAND} -E env MSYSTEM=MINGW64 ${BASH_EXECUTABLE} -l)
    ExternalProject_Add(tinycc_external
            SOURCE_DIR ${tinycc_src_SOURCE_DIR}
            BINARY_DIR ${TINYCC_BUILD_DIR}
            CONFIGURE_COMMAND ${CMAKE_COMMAND} -E copy_directory ${tinycc_src_SOURCE_DIR} ${TINYCC_BUILD_DIR}
                COMMAND ${MSYS2_BASH} -c "cd ${TINYCC_BUILD_DIR} && ./configure --prefix=${TINYCC_PREFIX_DIR} --enable-static --extra-cflags=-fPIC"
            BUILD_COMMAND ${MSYS2_BASH} -c "cd ${TINYCC_BUILD_DIR} && make -j${FAKELUA_TINYCC_BUILD_JOBS}"
            INSTALL_COMMAND ${MSYS2_BASH} -c "cd ${TINYCC_BUILD_DIR} && make install"
            BUILD_BYPRODUCTS ${TINYCC_STATIC_LIB}
    )
else ()
    ExternalProject_Add(tinycc_external
            SOURCE_DIR ${tinycc_src_SOURCE_DIR}
            BINARY_DIR ${TINYCC_BUILD_DIR}
            CONFIGURE_COMMAND ${tinycc_src_SOURCE_DIR}/configure --prefix=${TINYCC_PREFIX_DIR} --extra-cflags=-fPIC
            BUILD_COMMAND make -j${FAKELUA_TINYCC_BUILD_JOBS}
            INSTALL_COMMAND make install
            BUILD_BYPRODUCTS ${TINYCC_STATIC_LIB}
    )
    set(TARGET_LIB ${TARGET_LIB} Threads::Threads ${CMAKE_DL_LIBS} m)
endif ()

add_library(tinycc_embedded STATIC IMPORTED GLOBAL)
set_target_properties(tinycc_embedded PROPERTIES IMPORTED_LOCATION ${TINYCC_STATIC_LIB})
add_dependencies(tinycc_embedded tinycc_external)
add_dependencies(fakelua tinycc_external)
set(TARGET_LIB ${TARGET_LIB} tinycc_embedded)

if (USE_COV)
    set(TARGET_LIB ${TARGET_LIB} gcov)
endif ()

target_link_libraries(fakelua ${TARGET_LIB})
