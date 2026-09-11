set(FAKE_SOURCES
    binary.cpp
    bindfunc.cpp
    bison.cpp
    buildinfunc.cpp
    buildinfuncfile.cpp
    buildinfuncmath.cpp
    buildinfuncos.cpp
    buildinfuncstring.cpp
    codegen.cpp
    compiler.cpp
    container.cpp
    debuging.cpp
    fake-inc.cpp
    flex.cpp
    funcmap.cpp
    interpreter.cpp
    myflexer.cpp
    optimizer.cpp
    parser.cpp
    pointerheap.cpp
    processor.cpp
    profile.cpp
    stringheap.cpp
    syntree.cpp
    types.cpp
)

add_library(fake SHARED ${FAKE_SOURCES})

target_include_directories(fake
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/deps
)

if(FAKE_RELEASE)
    target_compile_options(fake PRIVATE -O3 -Wall -flto)
    target_compile_definitions(fake PRIVATE NDEBUG)
    target_link_options(fake PRIVATE -flto)
else()
    target_compile_options(fake PRIVATE -g3 -O0 -Wall)
    target_compile_definitions(fake PRIVATE _DEBUG)
endif()

set_target_properties(fake PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin
    ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin
    BUILD_RPATH ${PROJECT_SOURCE_DIR}/bin
)
