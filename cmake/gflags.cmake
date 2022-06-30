include(FetchContent)

set(GFLAGS_VER 2.2.2)
set(GFLAGS_DOWNLOAD_URL https://mirrors.tencent.com/github.com/gflags/gflags/archive/v${GFLAGS_VER}.tar.gz)

# 定义外部库的版本
FetchContent_Declare(
        gflags
        URL ${GFLAGS_DOWNLOAD_URL}
        SOURCE_DIR $ENV{FAKELUA_HOME}/cmake_third_party/gflags
)
FetchContent_MakeAvailable(gflags)

include_directories($ENV{FAKELUA_HOME}/cmake_third_party/gflags/src)
