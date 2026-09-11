add_executable(fakebin fakebin.cpp)
target_link_libraries(fakebin PRIVATE fake)

set(_bin ${PROJECT_SOURCE_DIR}/bin)
if(APPLE)
    set(_install_rpath "@loader_path")
else()
    set(_install_rpath "$ORIGIN")
endif()

function(fake_set_bin_flags tgt)
    if(FAKE_RELEASE)
        target_compile_options(${tgt} PRIVATE -O3 -Wall)
        target_compile_definitions(${tgt} PRIVATE NDEBUG)
    else()
        target_compile_options(${tgt} PRIVATE -g3 -O0 -Wall)
        target_compile_definitions(${tgt} PRIVATE _DEBUG)
    endif()
    set_target_properties(${tgt} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${_bin}
        BUILD_RPATH ${_bin}
        INSTALL_RPATH ${_install_rpath}
    )
endfunction()

fake_set_bin_flags(fakebin)

if(FAKE_BUILD_TESTS)
    add_executable(fake_tests
        host.cpp
        script_test.cpp
        unit/api_test.cpp
        unit/bind_test.cpp
        unit/hotreload_test.cpp
        unit/language_test.cpp
        unit/optimizer_test.cpp
        unit/reset_test.cpp
    )
    target_include_directories(fake_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_definitions(fake_tests PRIVATE FAKE_SCRIPT_DIR="${CMAKE_CURRENT_SOURCE_DIR}/scripts")
    target_link_libraries(fake_tests PRIVATE fake GTest::gtest_main)
    fake_set_bin_flags(fake_tests)

    add_test(
        NAME fake_tests
        COMMAND fake_tests --gtest_color=yes
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/scripts
    )
    set_tests_properties(fake_tests PROPERTIES TIMEOUT 120)
endif()
