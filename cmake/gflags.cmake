include(FetchContent)

set(GFLAGS_VER 2.2.2)
set(GFLAGS_DOWNLOAD_URL https://github.com/gflags/gflags/archive/v${GFLAGS_VER}.tar.gz)

# 定义外部库的版本
FetchContent_Declare(
        com_github_gflags
        URL ${GFLAGS_DOWNLOAD_URL}
        SOURCE_DIR $ENV{FAKELUA_HOME}/cmake_third_party/gflags
)
FetchContent_MakeAvailable(com_github_gflags)

include_directories(${FETCHCONTENT_BASE_DIR}/com_github_gflags-build/include)
