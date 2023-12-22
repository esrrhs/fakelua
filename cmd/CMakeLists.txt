cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

PROJECT(lua)

#add gflags
CPMAddPackage(
        NAME gflags
        GITHUB_REPOSITORY gflags/gflags
        GIT_TAG v2.2.2
        OPTIONS
        "BUILD_SHARED_LIBS OFF"
        "INSTALL_HEADERS ON"
        "INSTALL_SHARED_LIBS ON"
        "INSTALL_STATIC_LIBS ON"
        "REGISTER_INSTALL_PREFIX ON"
        "BUILD_TESTING OFF"
)

aux_source_directory(./ LUA_SRC_LIST)
include_directories(./)
include_directories(../include)

IF (WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--export-all-symbols")
ELSE ()
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-E")
ENDIF ()

add_executable(lua ${LUA_SRC_LIST})

target_link_libraries(lua PRIVATE fakelua gflags::gflags gccjit)
