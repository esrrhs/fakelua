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

find_package(PkgConfig QUIET)
if (PkgConfig_FOUND)
    pkg_check_modules(LUA lua5.4)
    if (NOT LUA_FOUND)
        pkg_check_modules(LUA lua-5.4)
    endif ()
endif ()
if (LUA_FOUND)
    include_directories(${LUA_INCLUDE_DIRS})
else ()
    # Fallback for systems where Lua is installed from source without pkg-config
    include_directories(/usr/include/lua5.4)
    include_directories(/usr/local/include/lua5.4)
    # macOS Homebrew paths
    if (APPLE)
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
