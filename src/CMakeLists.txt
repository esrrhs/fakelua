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
aux_source_directory(./native FAKELUA_SRC_LIST)
aux_source_directory(./native/basic FAKELUA_SRC_LIST)
aux_source_directory(./native/io FAKELUA_SRC_LIST)
aux_source_directory(./native/math FAKELUA_SRC_LIST)
aux_source_directory(./native/net FAKELUA_SRC_LIST)
aux_source_directory(./native/object FAKELUA_SRC_LIST)
aux_source_directory(./native/os FAKELUA_SRC_LIST)
aux_source_directory(./native/string FAKELUA_SRC_LIST)
aux_source_directory(./native/table FAKELUA_SRC_LIST)
aux_source_directory(./native/timer FAKELUA_SRC_LIST)
aux_source_directory(./native/serialize FAKELUA_SRC_LIST)
aux_source_directory(./native/protobuf FAKELUA_SRC_LIST)
aux_source_directory(./native/crypto FAKELUA_SRC_LIST)
aux_source_directory(./native/mysql FAKELUA_SRC_LIST)
aux_source_directory(./native/compress FAKELUA_SRC_LIST)
aux_source_directory(./native/utf8 FAKELUA_SRC_LIST)
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

set(TINYCC_PREFIX_DIR "${CMAKE_BINARY_DIR}/third_party/tinycc")
set(TINYCC_BUILD_DIR "${CMAKE_BINARY_DIR}/third_party/tinycc_build")
if (APPLE)
    # On macOS, adding tinycc source directory to include path causes conflicts
    # with standard headers (e.g. the 'version' file conflicts with <version>).
    # Use the installed header path as a SYSTEM include instead.
    include_directories(SYSTEM ${TINYCC_PREFIX_DIR}/include)
else ()
    include_directories(${tinycc_src_SOURCE_DIR})
endif ()

add_library(fakelua SHARED ${FAKELUA_SRC_LIST})

find_package(Threads REQUIRED)

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
    # Do NOT use hardcoded paths - let find_program search PATH to find the actual MSYS2 installation.
    find_program(BASH_EXECUTABLE bash)
    if (NOT BASH_EXECUTABLE)
        message(FATAL_ERROR "bash not found. MSYS2 bash is required to build TinyCC on Windows.")
    endif ()
    # MSYSTEM=MINGW64 ensures bash login shell loads MinGW64 PATH (where gcc lives),
    # not the MSYS2 native environment which lacks gcc.
    # Inside bash, we explicitly prepend /usr/bin and /mingw64/bin to PATH to ensure
    # uname, gcc, make etc are found. /usr/bin contains uname and other MSYS2 utilities.
    set(MSYS2_BASH ${CMAKE_COMMAND} -E env MSYSTEM=MINGW64 ${BASH_EXECUTABLE} -l)
    ExternalProject_Add(tinycc_external
            SOURCE_DIR ${tinycc_src_SOURCE_DIR}
            BINARY_DIR ${TINYCC_BUILD_DIR}
            CONFIGURE_COMMAND ${CMAKE_COMMAND} -E copy_directory ${tinycc_src_SOURCE_DIR} ${TINYCC_BUILD_DIR}
                COMMAND ${MSYS2_BASH} -c "cd ${TINYCC_BUILD_DIR} && export PATH='/usr/bin:/mingw64/bin:$PATH' && ./configure --prefix=${TINYCC_PREFIX_DIR} --enable-static --extra-cflags=-fPIC"
            BUILD_COMMAND ${MSYS2_BASH} -c "cd ${TINYCC_BUILD_DIR} && export PATH='/usr/bin:/mingw64/bin:$PATH' && make -j${FAKELUA_TINYCC_BUILD_JOBS}"
            INSTALL_COMMAND ${MSYS2_BASH} -c "cd ${TINYCC_BUILD_DIR} && export PATH='/usr/bin:/mingw64/bin:$PATH' && make install"
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
endif ()

if (WIN32)
    set(TARGET_LIB ${TARGET_LIB} ws2_32)
else ()
    set(TARGET_LIB ${TARGET_LIB} Threads::Threads)
    if (NOT APPLE)
        set(TARGET_LIB ${TARGET_LIB} dl m)
    endif ()
endif ()

add_library(tinycc_embedded STATIC IMPORTED GLOBAL)
set_target_properties(tinycc_embedded PROPERTIES IMPORTED_LOCATION ${TINYCC_STATIC_LIB})
add_dependencies(tinycc_embedded tinycc_external)
add_dependencies(fakelua tinycc_external)
set(TARGET_LIB ${TARGET_LIB} tinycc_embedded)

if (USE_COV)
    set(TARGET_LIB ${TARGET_LIB} gcov)
endif ()

# ── Compression libraries ──

# LZ4 — fetched via CPM (no system dev headers guaranteed)
CPMAddPackage(
        NAME lz4_src
        GITHUB_REPOSITORY lz4/lz4
        GIT_TAG v1.9.4
        DOWNLOAD_ONLY YES
)
if (NOT lz4_src_ADDED)
    message(FATAL_ERROR "Failed to fetch LZ4 source")
endif ()
add_library(lz4 STATIC
        ${lz4_src_SOURCE_DIR}/lib/lz4.c
        ${lz4_src_SOURCE_DIR}/lib/lz4hc.c
        ${lz4_src_SOURCE_DIR}/lib/lz4frame.c
        ${lz4_src_SOURCE_DIR}/lib/xxhash.c
)
target_include_directories(lz4 PUBLIC ${lz4_src_SOURCE_DIR}/lib)
set(TARGET_LIB ${TARGET_LIB} lz4)

# zlib — system library (universally available)
find_package(ZLIB REQUIRED)
set(TARGET_LIB ${TARGET_LIB} ZLIB::ZLIB)

# Zstd — use CPM to fetch and build from source (consistent across all platforms)
CPMAddPackage(
        NAME zstd_src
        GITHUB_REPOSITORY facebook/zstd
        GIT_TAG v1.5.6
        DOWNLOAD_ONLY YES
)
if (zstd_src_ADDED)
    set(ZSTD_SRC_DIR ${zstd_src_SOURCE_DIR})
    file(GLOB_RECURSE ZSTD_SRCS
            "${ZSTD_SRC_DIR}/lib/common/*.c"
            "${ZSTD_SRC_DIR}/lib/compress/*.c"
            "${ZSTD_SRC_DIR}/lib/decompress/*.c"
    )
    add_library(zstd STATIC ${ZSTD_SRCS})
    target_include_directories(zstd PUBLIC ${ZSTD_SRC_DIR}/lib)
    target_include_directories(zstd PRIVATE ${ZSTD_SRC_DIR}/lib/common)
    # Disable ASM optimizations — we only compile C sources, not assembly
    target_compile_definitions(zstd PRIVATE ZSTD_DISABLE_ASM)
    set(TARGET_LIB ${TARGET_LIB} zstd)
else ()
    message(WARNING "Failed to fetch Zstd source — zstd support disabled")
endif ()

target_link_libraries(fakelua ${TARGET_LIB})
