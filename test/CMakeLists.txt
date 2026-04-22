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

aux_source_directory(./ TEST_SRC_LIST)
include_directories(./)
include_directories(../include)
include_directories(../src)
include_directories(../src/platform)
if (APPLE)
    find_path(FAKELUA_LUA_INCLUDE_DIR
            NAMES lua.hpp
            PATH_SUFFIXES lua5.4 lua
    )
    if (NOT FAKELUA_LUA_INCLUDE_DIR)
        message(FATAL_ERROR "Lua headers not found. Please install Lua development package (e.g. brew install lua).")
    endif ()
    include_directories(${FAKELUA_LUA_INCLUDE_DIR})
    find_library(FAKELUA_LUA_LIBRARY
            NAMES lua lua5.4
    )
    if (NOT FAKELUA_LUA_LIBRARY)
        message(FATAL_ERROR "Lua library not found. Please install Lua development package (e.g. brew install lua).")
    endif ()
else ()
    include_directories(/usr/include/lua5.4)
endif ()

IF (WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--export-all-symbols")
ELSEIF (UNIX AND NOT APPLE)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-E")
ENDIF ()

add_executable(unit_tests ${TEST_SRC_LIST})

if (APPLE)
    target_link_libraries(unit_tests PRIVATE fakelua gtest_main ${FAKELUA_LUA_LIBRARY})
else ()
    target_link_libraries(unit_tests PRIVATE fakelua gtest_main lua)
endif ()

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
