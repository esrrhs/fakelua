include(FetchContent)

set(GLOG_VER 0.6.0)
set(GLOG_DOWNLOAD_URL https://github.com/google/glog/archive/refs/tags/v${GLOG_VER}.tar.gz)

IF (WIN32)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
ELSE ()
    set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
ENDIF ()
set(WITH_GFLAGS OFF CACHE BOOL "" FORCE)
set(WITH_GTEST OFF CACHE BOOL "" FORCE)
set(WITH_PKGCONFIG OFF CACHE BOOL "" FORCE)
set(WITH_SYMBOLIZE OFF CACHE BOOL "" FORCE)
set(WITH_THREADS ON CACHE BOOL "" FORCE)
set(WITH_TLS OFF CACHE BOOL "" FORCE)
set(WITH_UNWIND OFF CACHE BOOL "" FORCE)

add_compile_definitions(HAVE_LIB_GFLAGS)

# 定义外部库的版本
FetchContent_Declare(
        com_github_glog
        URL ${GLOG_DOWNLOAD_URL}
        SOURCE_DIR ${CMAKE_BINARY_DIR}/cmake_third_party/glog
)
FetchContent_MakeAvailable(com_github_glog)

include_directories(${CMAKE_BINARY_DIR}/cmake_third_party/glog/src)
include_directories(${FETCHCONTENT_BASE_DIR}/com_github_glog-build)
