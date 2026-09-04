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
aux_source_directory(./native/json FAKELUA_SRC_LIST)
aux_source_directory(./native/csv FAKELUA_SRC_LIST)
aux_source_directory(./native/sqlite FAKELUA_SRC_LIST)
aux_source_directory(./native/event FAKELUA_SRC_LIST)
aux_source_directory(./native/random FAKELUA_SRC_LIST)
aux_source_directory(./native/utf8 FAKELUA_SRC_LIST)
aux_source_directory(./native/yaml FAKELUA_SRC_LIST)
aux_source_directory(./native/xml FAKELUA_SRC_LIST)
aux_source_directory(./native/toml FAKELUA_SRC_LIST)
aux_source_directory(./native/ini FAKELUA_SRC_LIST)
aux_source_directory(./native/log FAKELUA_SRC_LIST)
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
# Create a dedicated include directory for libtcc.h only.
# We can't use either the source dir (Windows case-insensitive FS matches VERSION to <version>
# C++20 header) or the install dir (contains win32 headers wchar.h/time.h that conflict with
# system headers when added as SYSTEM include). So we copy libtcc.h to its own dir.
set(TINYCC_LIBTCC_INCLUDE_DIR "${CMAKE_BINARY_DIR}/third_party/tinycc_include")
include_directories(${TINYCC_LIBTCC_INCLUDE_DIR})

# SQLite3 amalgamation via CPM — compiled directly into fakelua (not a separate static lib)
# to avoid CRT mismatch issues on Windows/MSYS2.
CPMAddPackage(
    NAME sqlite3_amalgamation
    URL https://www.sqlite.org/2024/sqlite-amalgamation-3450000.zip
    DOWNLOAD_ONLY YES
)
if (NOT sqlite3_amalgamation_ADDED)
    message(FATAL_ERROR "Failed to fetch SQLite3 amalgamation")
endif ()
include_directories(${sqlite3_amalgamation_SOURCE_DIR})
set(FAKELUA_SRC_LIST ${FAKELUA_SRC_LIST} ${sqlite3_amalgamation_SOURCE_DIR}/sqlite3.c)

# INI (inih) — pure C single-file, compile ini.c directly (like sqlite3)
CPMAddPackage(
    NAME inih
    GITHUB_REPOSITORY benhoyt/inih
    GIT_TAG r56
    DOWNLOAD_ONLY YES
)
if (NOT inih_ADDED)
    message(FATAL_ERROR "Failed to fetch inih")
endif ()
include_directories(${inih_SOURCE_DIR})
set(FAKELUA_SRC_LIST ${FAKELUA_SRC_LIST} ${inih_SOURCE_DIR}/ini.c)

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
            COMMAND ${CMAKE_COMMAND} -E make_directory ${TINYCC_LIBTCC_INCLUDE_DIR}
            COMMAND ${CMAKE_COMMAND} -E copy ${TINYCC_PREFIX_DIR}/include/libtcc.h ${TINYCC_LIBTCC_INCLUDE_DIR}/libtcc.h
            BUILD_BYPRODUCTS ${TINYCC_STATIC_LIB}
    )
else ()
    ExternalProject_Add(tinycc_external
            SOURCE_DIR ${tinycc_src_SOURCE_DIR}
            BINARY_DIR ${TINYCC_BUILD_DIR}
            CONFIGURE_COMMAND ${tinycc_src_SOURCE_DIR}/configure --prefix=${TINYCC_PREFIX_DIR} --extra-cflags=-fPIC
            BUILD_COMMAND make -j${FAKELUA_TINYCC_BUILD_JOBS}
            INSTALL_COMMAND make install
            COMMAND ${CMAKE_COMMAND} -E make_directory ${TINYCC_LIBTCC_INCLUDE_DIR}
            COMMAND ${CMAKE_COMMAND} -E copy ${TINYCC_PREFIX_DIR}/include/libtcc.h ${TINYCC_LIBTCC_INCLUDE_DIR}/libtcc.h
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

# ── Compression libraries via CPM ──

# LZ4
CPMAddPackage(
    NAME lz4
    GITHUB_REPOSITORY lz4/lz4
    GIT_TAG v1.9.4
    SOURCE_SUBDIR build/cmake
    OPTIONS
        "BUILD_SHARED_LIBS OFF"
        "CMAKE_POSITION_INDEPENDENT_CODE ON"
        "LZ4_BUILD_CLI OFF"
        "LZ4_BUILD_LEGACY_LZ4C OFF"
)
if (NOT lz4_ADDED)
    message(FATAL_ERROR "Failed to fetch LZ4")
endif ()
set(TARGET_LIB ${TARGET_LIB} lz4_static)

# zlib
CPMAddPackage(
    NAME zlib
    GITHUB_REPOSITORY madler/zlib
    GIT_TAG v1.3.1
    OPTIONS
        "BUILD_SHARED_LIBS OFF"
        "CMAKE_POSITION_INDEPENDENT_CODE ON"
        "ZLIB_BUILD_EXAMPLES OFF"
)
if (NOT zlib_ADDED)
    message(FATAL_ERROR "Failed to fetch zlib")
endif ()
set(TARGET_LIB ${TARGET_LIB} zlibstatic)

# Zstd
CPMAddPackage(
    NAME zstd
    GITHUB_REPOSITORY facebook/zstd
    GIT_TAG v1.5.6
    SOURCE_SUBDIR build/cmake
    OPTIONS
        "ZSTD_BUILD_SHARED_LIB OFF"
        "ZSTD_BUILD_PROGRAMS OFF"
        "ZSTD_BUILD_CONTRIB OFF"
        "ZSTD_BUILD_TESTS OFF"
        "ZSTD_DISABLE_ASM ON"
        "BUILD_SHARED_LIBS OFF"
        "CMAKE_POSITION_INDEPENDENT_CODE ON"
)
if (NOT zstd_ADDED)
    message(FATAL_ERROR "Failed to fetch zstd")
endif ()
set(TARGET_LIB ${TARGET_LIB} libzstd_static)

# ── Config format libraries via CPM ──

# YAML (yaml-cpp)
CPMAddPackage(
    NAME yaml_cpp
    GITHUB_REPOSITORY jbeder/yaml-cpp
    GIT_TAG 0.8.0
    OPTIONS
        "BUILD_SHARED_LIBS OFF"
        "CMAKE_POSITION_INDEPENDENT_CODE ON"
        "YAML_CPP_BUILD_TESTS OFF"
        "YAML_CPP_BUILD_TOOLS OFF"
        "YAML_CPP_BUILD_CONTRIB OFF"
        "YAML_CPP_INSTALL OFF"
)
if (NOT yaml_cpp_ADDED)
    message(FATAL_ERROR "Failed to fetch yaml-cpp")
endif ()
set(TARGET_LIB ${TARGET_LIB} yaml-cpp::yaml-cpp)

# XML (pugixml)
CPMAddPackage(
    NAME pugixml
    GITHUB_REPOSITORY zeux/pugixml
    GIT_TAG v1.14
    OPTIONS
        "BUILD_SHARED_LIBS OFF"
        "CMAKE_POSITION_INDEPENDENT_CODE ON"
)
if (NOT pugixml_ADDED)
    message(FATAL_ERROR "Failed to fetch pugixml")
endif ()
set(TARGET_LIB ${TARGET_LIB} pugixml::pugixml)

# TOML (toml++) — header-only
CPMAddPackage(
    NAME tomlplusplus
    GITHUB_REPOSITORY marzer/tomlplusplus
    GIT_TAG v3.4.0
    OPTIONS
        "BUILD_SHARED_LIBS OFF"
        "CMAKE_POSITION_INDEPENDENT_CODE ON"
)
if (NOT tomlplusplus_ADDED)
    message(FATAL_ERROR "Failed to fetch tomlplusplus")
endif ()
set(TARGET_LIB ${TARGET_LIB} tomlplusplus::tomlplusplus)

# Windows MinGW workarounds for GCC 16.2.0 stdlib bugs
# - _errno macro conflict: _GLIBCXX_USE_C99_STDIO=0
# - snprintf not in std namespace: __USE_MINGW_ANSI_STDIO=1
# - CRT secure warnings: _CRT_SECURE_NO_WARNINGS
if (MINGW)
    # Set cache variables BEFORE CPMAddPackage so they propagate to Boost's build
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_USE_C99_STDIO=0 -D__USE_MINGW_ANSI_STDIO=1 -D_CRT_SECURE_NO_WARNINGS" CACHE STRING "" FORCE)
    add_compile_definitions(_GLIBCXX_USE_C99_STDIO=0 __USE_MINGW_ANSI_STDIO=1 _CRT_SECURE_NO_WARNINGS)
    add_compile_options(-D_GLIBCXX_USE_C99_STDIO=0 -D__USE_MINGW_ANSI_STDIO=1 -D_CRT_SECURE_NO_WARNINGS)
endif()

# Fetch Boost 1.90+ source for MySQL headers (header-only mode for Asio/System/MySQL)
if (MINGW)
    CPMAddPackage(
        NAME boost
        GITHUB_REPOSITORY boostorg/boost
        GIT_TAG boost-1.90.0
        OPTIONS
            "BUILD_SHARED_LIBS=OFF"
            "CMAKE_POSITION_INDEPENDENT_CODE=ON"
            "CMAKE_CXX_FLAGS=-D_GLIBCXX_USE_C99_STDIO=0 -D__USE_MINGW_ANSI_STDIO=1 -D_CRT_SECURE_NO_WARNINGS"
    )
else()
    CPMAddPackage(
        NAME boost
        GITHUB_REPOSITORY boostorg/boost
        GIT_TAG boost-1.90.0
        OPTIONS
            "BUILD_SHARED_LIBS=OFF"
            "CMAKE_POSITION_INDEPENDENT_CODE=ON"
    )
endif()
if (NOT boost_ADDED)
    message(FATAL_ERROR "Failed to fetch boost")
endif()
set(BOOST_SOURCE_DIR "${boost_SOURCE_DIR}")

# Workaround for MinGW GCC 16.2.0: std::snprintf is not declared because
# MinGW's <cstdio> doesn't put snprintf in std namespace even with
# __USE_MINGW_ANSI_STDIO=1. Patch the boost headers that use std::snprintf.
if (MINGW)
    # snprintf.hpp - change "using std::snprintf;" to "using ::snprintf;"
    file(READ "${BOOST_SOURCE_DIR}/libs/system/include/boost/system/detail/snprintf.hpp" _SNPRINTF_HPP)
    string(REPLACE "using std::snprintf;" "using ::snprintf;" _SNPRINTF_HPP "${_SNPRINTF_HPP}")
    file(WRITE "${BOOST_SOURCE_DIR}/libs/system/include/boost/system/detail/snprintf.hpp" "${_SNPRINTF_HPP}")

    # source_location.hpp - change "std::snprintf(" to "::snprintf("
    file(READ "${BOOST_SOURCE_DIR}/libs/assert/include/boost/assert/source_location.hpp" _ASSERT_HPP)
    string(REPLACE "std::snprintf(" "::snprintf(" _ASSERT_HPP "${_ASSERT_HPP}")
    file(WRITE "${BOOST_SOURCE_DIR}/libs/assert/include/boost/assert/source_location.hpp" "${_ASSERT_HPP}")

    # fallback_routines.hpp - change "std::snprintf" to "::snprintf"
    file(READ "${BOOST_SOURCE_DIR}/libs/charconv/include/boost/charconv/detail/fallback_routines.hpp" _FALLBACK_HPP)
    string(REPLACE "std::snprintf" "::snprintf" _FALLBACK_HPP "${_FALLBACK_HPP}")
    file(WRITE "${BOOST_SOURCE_DIR}/libs/charconv/include/boost/charconv/detail/fallback_routines.hpp" "${_FALLBACK_HPP}")

    # config.ipp - change "std::snprintf" to "::snprintf"
    file(READ "${BOOST_SOURCE_DIR}/libs/asio/include/boost/asio/impl/config.ipp" _CONFIG_HPP)
    string(REPLACE "std::snprintf" "::snprintf" _CONFIG_HPP "${_CONFIG_HPP}")
    file(WRITE "${BOOST_SOURCE_DIR}/libs/asio/include/boost/asio/impl/config.ipp" "${_CONFIG_HPP}")
endif()

# Include ALL Boost headers needed for MySQL + Asio + System
# This is the comprehensive include path approach since FindBoost doesn't work with CPM
include_directories(${BOOST_SOURCE_DIR})
include_directories(${BOOST_SOURCE_DIR}/libs/mysql/include)
include_directories(${BOOST_SOURCE_DIR}/libs/asio/include)
include_directories(${BOOST_SOURCE_DIR}/libs/system/include)
include_directories(${BOOST_SOURCE_DIR}/libs/assert/include)
include_directories(${BOOST_SOURCE_DIR}/libs/charconv/include)
include_directories(${BOOST_SOURCE_DIR}/libs/compat/include)
include_directories(${BOOST_SOURCE_DIR}/libs/config/include)
include_directories(${BOOST_SOURCE_DIR}/libs/container/include)
include_directories(${BOOST_SOURCE_DIR}/libs/core/include)
include_directories(${BOOST_SOURCE_DIR}/libs/describe/include)
include_directories(${BOOST_SOURCE_DIR}/libs/endian/include)
include_directories(${BOOST_SOURCE_DIR}/libs/intrusive/include)
include_directories(${BOOST_SOURCE_DIR}/libs/mp11/include)
include_directories(${BOOST_SOURCE_DIR}/libs/optional/include)
include_directories(${BOOST_SOURCE_DIR}/libs/throw_exception/include)
include_directories(${BOOST_SOURCE_DIR}/libs/variant2/include)
include_directories(${BOOST_SOURCE_DIR}/libs/pfr/include)
include_directories(${BOOST_SOURCE_DIR}/libs/type_traits/include)
include_directories(${BOOST_SOURCE_DIR}/libs/align/include)
include_directories(${BOOST_SOURCE_DIR}/libs/detail/include)
include_directories(${BOOST_SOURCE_DIR}/libs/smart_ptr/include)
include_directories(${BOOST_SOURCE_DIR}/libs/utility/include)
include_directories(${BOOST_SOURCE_DIR}/libs/move/include)
include_directories(${BOOST_SOURCE_DIR}/libs/function/include)
include_directories(${BOOST_SOURCE_DIR}/libs/bind/include)
include_directories(${BOOST_SOURCE_DIR}/libs/ref/include)
include_directories(${BOOST_SOURCE_DIR}/libs/predef/include)
include_directories(${BOOST_SOURCE_DIR}/libs/integer/include)
include_directories(${BOOST_SOURCE_DIR}/libs/cstdint/include)
include_directories(${BOOST_SOURCE_DIR}/libs/winapi/include)
include_directories(${BOOST_SOURCE_DIR}/libs/nowide/include)
include_directories(${BOOST_SOURCE_DIR}/libs/static_assert/include)
include_directories(${BOOST_SOURCE_DIR}/libs/exception/include)
include_directories(${BOOST_SOURCE_DIR}/libs/iterator/include)
include_directories(${BOOST_SOURCE_DIR}/libs/mpl/include)
include_directories(${BOOST_SOURCE_DIR}/libs/serialization/include)
include_directories(${BOOST_SOURCE_DIR}/libs/any/include)
include_directories(${BOOST_SOURCE_DIR}/libs/pool/include)
include_directories(${BOOST_SOURCE_DIR}/libs/array/include)

# OpenSSL and Threads required by Boost.MySQL
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

# Boost.MySQL is header-only, but boost_charconv has compiled components for float parsing
# Link boost_charconv library (required for boost::charconv::from_chars/to_chars)
set(TARGET_LIB ${TARGET_LIB} OpenSSL::SSL OpenSSL::Crypto Threads::Threads boost_charconv)

target_link_libraries(fakelua PRIVATE ${TARGET_LIB})
