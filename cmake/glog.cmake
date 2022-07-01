include(FetchContent)

set(GLOG_VER 0.6.0)
set(GLOG_DOWNLOAD_URL https://github.com/google/glog/archive/refs/tags/v${GLOG_VER}.tar.gz)

# 定义外部库的版本
FetchContent_Declare(
        com_github_glog
        URL ${GLOG_DOWNLOAD_URL}
        SOURCE_DIR ${CMAKE_BINARY_DIR}/cmake_third_party/glog
)
FetchContent_MakeAvailable(com_github_glog)

include_directories(${CMAKE_BINARY_DIR}/cmake_third_party/glog/src)
