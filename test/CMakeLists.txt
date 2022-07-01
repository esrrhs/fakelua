cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(test)

aux_source_directory(./ TEST_SRC_LIST)
include_directories(./)
include_directories(../src)

add_executable(unit_tests ${TEST_SRC_LIST})

target_link_libraries(unit_tests PRIVATE fakelua gtest)

add_test(NAME unit COMMAND ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}/unit_tests WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})

add_custom_command(
        TARGET unit_tests
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/test.lua ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}/test.lua
)