include(FetchContent)

set(GOOGLETEST_VER 1.12.1)
set(GOOGLETEST_URL https://github.com/google/googletest/archive/refs/tags/release-${GOOGLETEST_VER}.tar.gz)

FetchContent_Declare(
        com_github_googletest
        URL ${GOOGLETEST_URL}
        SOURCE_DIR ${CMAKE_BINARY_DIR}/cmake_third_party/googletest
)
FetchContent_MakeAvailable(com_github_googletest)

include_directories(${CMAKE_BINARY_DIR}/cmake_third_party/googletest/include)
