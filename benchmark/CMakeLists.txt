add_executable(fake_bench bench_vm.cpp)
target_link_libraries(fake_bench PRIVATE fake benchmark::benchmark)
target_include_directories(fake_bench PRIVATE ${PROJECT_SOURCE_DIR}/include)

set(_bin ${PROJECT_SOURCE_DIR}/bin)
if(APPLE)
    set(_install_rpath "@loader_path")
else()
    set(_install_rpath "$ORIGIN")
endif()

if(FAKE_RELEASE)
    target_compile_options(fake_bench PRIVATE -O3 -Wall)
    target_compile_definitions(fake_bench PRIVATE NDEBUG)
else()
    target_compile_options(fake_bench PRIVATE -g3 -O0 -Wall)
    target_compile_definitions(fake_bench PRIVATE _DEBUG)
endif()

set_target_properties(fake_bench PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${_bin}
    BUILD_RPATH ${_bin}
    INSTALL_RPATH ${_install_rpath}
)
