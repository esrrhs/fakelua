cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(test)

#add gtest
CPMAddPackage(
        NAME googletest
        GITHUB_REPOSITORY google/googletest
        GIT_TAG v1.17.0
        VERSION 1.17.0
        OPTIONS
        "INSTALL_GTEST OFF"
        "gtest_force_shared_crt ON"
)

file(GLOB TEST_SRC_LIST "*.cpp")
include_directories(./)
include_directories(../include)
include_directories(../src)
include_directories(../src/platform)

# Boost.MySQL include directories (copied from src/CMakeLists.txt)
set(BOOST_SOURCE_DIR "${boost_SOURCE_DIR}")
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

find_package(PkgConfig QUIET)
set(LUA_FOUND FALSE)
if (PkgConfig_FOUND)
    # Try different Lua versions
    pkg_check_modules(LUA54 QUIET lua5.4)
    if (LUA54_FOUND)
        set(LUA_FOUND TRUE)
        set(LUA_INCLUDE_DIRS ${LUA54_INCLUDE_DIRS})
        set(LUA_LIBRARIES ${LUA54_LIBRARIES})
        set(LUA_LIBRARY_DIRS ${LUA54_LIBRARY_DIRS})
    endif ()
    if (NOT LUA_FOUND)
        pkg_check_modules(LUA55 QUIET lua5.5)
        if (LUA55_FOUND)
            set(LUA_FOUND TRUE)
            set(LUA_INCLUDE_DIRS ${LUA55_INCLUDE_DIRS})
            set(LUA_LIBRARIES ${LUA55_LIBRARIES})
            set(LUA_LIBRARY_DIRS ${LUA55_LIBRARY_DIRS})
        endif ()
    endif ()
    if (NOT LUA_FOUND)
        pkg_check_modules(LUAGEN QUIET lua)
        if (LUAGEN_FOUND)
            set(LUA_FOUND TRUE)
            set(LUA_INCLUDE_DIRS ${LUAGEN_INCLUDE_DIRS})
            set(LUA_LIBRARIES ${LUAGEN_LIBRARIES})
            set(LUA_LIBRARY_DIRS ${LUAGEN_LIBRARY_DIRS})
        endif ()
    endif ()
endif ()
if (LUA_FOUND)
    include_directories(${LUA_INCLUDE_DIRS})
    link_directories(${LUA_LIBRARY_DIRS})
    message(STATUS "Found Lua: includes=${LUA_INCLUDE_DIRS}, libs=${LUA_LIBRARIES}, libdirs=${LUA_LIBRARY_DIRS}")
else ()
    # Fallback for systems where Lua is installed from source without pkg-config
    include_directories(/usr/include/lua5.4)
    include_directories(/usr/local/include/lua5.4)
    # macOS Homebrew paths (Lua 5.5 or 5.4)
    if (APPLE)
        include_directories(/opt/homebrew/include/lua)
        include_directories(/opt/homebrew/include/lua5.5)
        include_directories(/opt/homebrew/include/lua5.4)
        include_directories(/usr/local/opt/lua/include)
    endif ()
    set(LUA_LIBRARIES lua)
endif ()

IF (WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--export-all-symbols")
ELSEIF (APPLE)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-export_dynamic")
ELSE ()
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-E")
ENDIF ()

add_executable(unit_tests ${TEST_SRC_LIST})

target_link_libraries(unit_tests PRIVATE fakelua gtest_main ${LUA_LIBRARIES})

# Boost.MySQL include directories (from CPM boost package)
# Find the boost source directory
set(BOOST_SOURCE_DIR "${CPM_PACKAGE_boost_SOURCE_DIR}")
if (NOT EXISTS "${BOOST_SOURCE_DIR}/libs/mysql/include/boost/mysql.hpp")
    # Try alternative path in build tree
    set(BOOST_SOURCE_DIR "${CMAKE_BINARY_DIR}/_deps/boost-src")
endif()
if (EXISTS "${BOOST_SOURCE_DIR}/libs/mysql/include/boost/mysql.hpp")
    target_include_directories(unit_tests PRIVATE ${BOOST_SOURCE_DIR}/libs/mysql/include)
    target_include_directories(unit_tests PRIVATE ${BOOST_SOURCE_DIR}/libs/asio/include)
    target_include_directories(unit_tests PRIVATE ${BOOST_SOURCE_DIR}/libs/system/include)
    target_include_directories(unit_tests PRIVATE ${BOOST_SOURCE_DIR}/libs/assert/include)
    target_include_directories(unit_tests PRIVATE ${BOOST_SOURCE_DIR}/libs/charconv/include)
    target_include_directories(unit_tests PRIVATE ${BOOST_SOURCE_DIR}/libs/compat/include)
    target_include_directories(unit_tests PRIVATE ${BOOST_SOURCE_DIR}/libs/config/include)
    target_include_directories(unit_tests PRIVATE ${BOOST_SOURCE_DIR}/libs/container/include)
    target_include_directories(unit_tests PRIVATE ${BOOST_SOURCE_DIR}/libs/core/include)
    target_include_directories(unit_tests PRIVATE ${BOOST_SOURCE_DIR}/libs/describe/include)
    target_include_directories(unit_tests PRIVATE ${BOOST_SOURCE_DIR}/libs/endian/include)
    target_include_directories(unit_tests PRIVATE ${BOOST_SOURCE_DIR}/libs/intrusive/include)
    target_include_directories(unit_tests PRIVATE ${BOOST_SOURCE_DIR}/libs/mp11/include)
    target_include_directories(unit_tests PRIVATE ${BOOST_SOURCE_DIR}/libs/optional/include)
    target_include_directories(unit_tests PRIVATE ${BOOST_SOURCE_DIR}/libs/throw_exception/include)
    target_include_directories(unit_tests PRIVATE ${BOOST_SOURCE_DIR}/libs/variant2/include)
    target_include_directories(unit_tests PRIVATE ${BOOST_SOURCE_DIR}/libs/pfr/include)
    target_include_directories(unit_tests PRIVATE ${BOOST_SOURCE_DIR}/libs/type_traits/include)
    target_include_directories(unit_tests PRIVATE ${BOOST_SOURCE_DIR}/libs/container_hash/include)
    target_include_directories(unit_tests PRIVATE ${BOOST_SOURCE_DIR}/libs/static_assert/include)
    target_include_directories(unit_tests PRIVATE ${BOOST_SOURCE_DIR}/libs/move/include)
    target_include_directories(unit_tests PRIVATE ${BOOST_SOURCE_DIR}/libs/iterator/include)
    target_include_directories(unit_tests PRIVATE ${BOOST_SOURCE_DIR}/libs/utility/include)
    target_include_directories(unit_tests PRIVATE ${BOOST_SOURCE_DIR}/libs/predef/include)
endif()

add_test(NAME unit COMMAND ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}/unit_tests
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})

add_custom_command(
        TARGET unit_tests
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/lua/ ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}/
)

IF (WIN32)
    add_custom_command(
            TARGET unit_tests
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_BINARY_DIR}/third_party/tinycc/ ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}/
    )
ENDIF ()
