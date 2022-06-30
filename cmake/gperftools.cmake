include(FetchContent)

set(GPERFTOOLS_VER 2.10)
set(GPERFTOOLS_URL https://mirrors.tencent.com/github.com/gperftools/gperftools/releases/download/gperftools-${GPERFTOOLS_VER}/gperftools-${GPERFTOOLS_VER}.tar.gz)

set(GPERFTOOLS_BUILD_CPU_PROFILER ON CACHE BOOL "" FORCE)
set(GPERFTOOLS_BUILD_HEAP_PROFILER OFF CACHE BOOL "" FORCE)
set(GPERFTOOLS_BUILD_HEAP_CHECKER OFF CACHE BOOL "" FORCE)
set(GPERFTOOLS_BUILD_DEBUGALLOC OFF CACHE BOOL "" FORCE)
set(gperftools_build_benchmark OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
        com_github_gperftools
        URL ${GPERFTOOLS_URL}
        SOURCE_DIR $ENV{FAKELUA_HOME}/cmake_third_party/gperftools
)
FetchContent_MakeAvailable(com_github_gperftools)

include_directories($ENV{FAKELUA_HOME}/cmake_third_party/gperftools/src)
