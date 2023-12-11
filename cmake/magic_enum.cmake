include(FetchContent)

set(MAGIC_ENUM_VER 0.9.3)
set(MAGIC_ENUM_DOWNLOAD_URL https://github.com/Neargye/magic_enum/archive/refs/tags/v${MAGIC_ENUM_VER}.tar.gz)

# 定义外部库的版本
FetchContent_Declare(
        com_github_magic_enum
        URL ${MAGIC_ENUM_DOWNLOAD_URL}
        SOURCE_DIR ${CMAKE_BINARY_DIR}/cmake_third_party/magic_enum
)
FetchContent_MakeAvailable(com_github_magic_enum)

include_directories(${CMAKE_BINARY_DIR}/cmake_third_party/magic_enum/include)
