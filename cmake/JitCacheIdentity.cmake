# A content/build fingerprint for the host/JIT ABI, generated at build time.
# Reconfigure after source edits, including dirty/uncommitted changes; a version
# number or git revision alone would silently reuse stale machine code.
get_filename_component(_fl_jit_root "${CMAKE_CURRENT_LIST_DIR}/.." REALPATH)
file(GLOB_RECURSE _fl_jit_inputs
    "${_fl_jit_root}/src/*.hh" "${_fl_jit_root}/src/*.hpp" "${_fl_jit_root}/src/*.h" "${_fl_jit_root}/src/*.cpp"
    "${_fl_jit_root}/src/*.y" "${_fl_jit_root}/src/*.l"
    "${_fl_jit_root}/include/*.h" "${_fl_jit_root}/cmake/*.cmake")
list(APPEND _fl_jit_inputs "${_fl_jit_root}/CMakeLists.txt" "${_fl_jit_root}/src/CMakeLists.txt")
list(SORT _fl_jit_inputs)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${_fl_jit_inputs})
set(_fl_jit_identity "${CMAKE_SYSTEM_NAME};${CMAKE_SYSTEM_PROCESSOR};${CMAKE_SIZEOF_VOID_P};${CMAKE_CXX_COMPILER_ID};${CMAKE_CXX_COMPILER_VERSION};${CMAKE_BUILD_TYPE}")
foreach(_input IN LISTS _fl_jit_inputs)
    file(SHA256 "${_input}" _hash)
    string(APPEND _fl_jit_identity ";${_input}:${_hash}")
endforeach()
foreach(_flag CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
    CMAKE_CXX_FLAGS_RELWITHDEBINFO CMAKE_CXX_FLAGS_MINSIZEREL
    CMAKE_CXX_COMPILER_TARGET CMAKE_CXX_COMPILER_EXTERNAL_TOOLCHAIN CMAKE_CXX_STANDARD
    CMAKE_SHARED_LINKER_FLAGS CMAKE_OSX_ARCHITECTURES CMAKE_OSX_SYSROOT
    CMAKE_OSX_DEPLOYMENT_TARGET USE_ASAN USE_COV FAKELUA_NATIVE_OBJECT_DEBUG_HASH)
    string(APPEND _fl_jit_identity ";${_flag}=${${_flag}}")
endforeach()
# Include custom configuration flags as well as the standard configurations.
get_cmake_property(_fl_jit_variables VARIABLES)
foreach(_flag IN LISTS _fl_jit_variables)
    if(_flag MATCHES "^CMAKE_(CXX_FLAGS|SHARED_LINKER_FLAGS)(_|$)")
        string(APPEND _fl_jit_identity ";${_flag}=${${_flag}}")
    endif()
endforeach()
foreach(_property COMPILE_OPTIONS COMPILE_DEFINITIONS COMPILE_FEATURES CXX_STANDARD CXX_EXTENSIONS
    POSITION_INDEPENDENT_CODE INCLUDE_DIRECTORIES LINK_OPTIONS LINK_LIBRARIES)
    get_target_property(_value fakelua ${_property})
    string(APPEND _fl_jit_identity ";${_property}=${_value}")
endforeach()
file(SHA256 "${CMAKE_CXX_COMPILER}" _fl_host_compiler_hash)
string(SHA256 _fl_jit_hash "${_fl_jit_identity};${_fl_host_compiler_hash}")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/jit_build_identity.h.tmp" "#pragma once\n#define FAKELUA_JIT_BUILD_ID \"${_fl_jit_hash}\"\n")
configure_file("${CMAKE_CURRENT_BINARY_DIR}/jit_build_identity.h.tmp" "${CMAKE_CURRENT_BINARY_DIR}/jit_build_identity.h" COPYONLY)
target_include_directories(fakelua PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
# CMAKE_BUILD_TYPE is empty for multi-config generators. A per-source definition
# records the actual configuration without a shared header that concurrent
# Debug/Release builds could overwrite.
set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/jit/gcc_cache.cpp" APPEND PROPERTY
    COMPILE_DEFINITIONS "FAKELUA_JIT_BUILD_CONFIGURATION=\"$<CONFIG>\"")
