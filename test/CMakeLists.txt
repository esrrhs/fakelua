cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(test)

aux_source_directory(./ TEST_SRC_LIST)
include_directories(../src)

add_executable(unit_tests ${TEST_SRC_LIST})

target_link_libraries(unit_tests gtest_main fakelua)

add_test(NAME unit COMMAND ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}/unit_tests)
