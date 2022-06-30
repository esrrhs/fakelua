include(FetchContent)

set(GOOGLETEST_VER 1.10.0)
set(GOOGLETEST_URL https://mirrors.tencent.com/github.com/google/googletest/archive/refs/tags/release-${GOOGLETEST_VER}.tar.gz)

FetchContent_Declare(
        com_github_googletest
        URL ${GOOGLETEST_URL}
        SOURCE_DIR $ENV{FAKELUA_HOME}/cmake_third_party/googletest
)
FetchContent_MakeAvailable(com_github_googletest)

include_directories($ENV{FAKELUA_HOME}/cmake_third_party/googletest/include)
