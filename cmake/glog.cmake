include(FetchContent)

set(GLOG_VER 0.6.0)
set(GLOG_DOWNLOAD_URL https://https://github.com/google/glog/archive/refs/tags/v${GFLAGS_VER}.tar.gz)

# 定义外部库的版本
FetchContent_Declare(
        com_github_glog
        URL ${GLOG_DOWNLOAD_URL}
        SOURCE_DIR $ENV{FAKELUA_HOME}/cmake_third_party/glog
)
FetchContent_MakeAvailable(com_github_glog)

include_directories($ENV{FAKELUA_HOME}/cmake_third_party/glog/src)
