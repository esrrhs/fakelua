cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(test)

aux_source_directory(./ TEST_SRC_LIST)
include_directories(./)
include_directories(../include)
include_directories(../src)
include_directories(../src/platform)

IF (WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--export-all-symbols")
ELSE ()
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-E")
ENDIF ()

add_executable(unit_tests ${TEST_SRC_LIST})

target_link_libraries(unit_tests PRIVATE fakelua gtest_main glog::glog gflags::gflags gccjit)

add_test(NAME unit COMMAND ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}/unit_tests
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})

add_custom_command(
        TARGET unit_tests
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/lua/ ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}/
)
