cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

project(fakelua CXX)

set(CMAKE_CXX_STANDARD 23)

# add dependencies
include(cmake/CPM.cmake)

# place binaries and libraries according to GNU standards
include(GNUInstallDirs)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})

set(ENV{FAKELUA_HOME} "${CMAKE_CURRENT_SOURCE_DIR}/")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unused-local-typedefs -Wno-unused-function -DNDEBUG -g3 -ggdb3 -fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -DNDEBUG -g3 -ggdb3 -fPIC")

option(USE_ASAN "use asan" OFF)
if (USE_ASAN)
    message("build with asan")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
endif ()

option(USE_COV "use cov" OFF)
if (USE_COV)
    message("build with cov")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage -fprofile-update=atomic")
endif ()

IF (WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
    # 用 select reactor，不用 IOCP。asio 的 IOCP 后端在关停时会丢唤醒包：stop() 只投
    # 一个完成包，还被一次性闩锁挡着，丢一次 ~io_context 里 join 内部线程就永远不返回
    # （asio#431、asio#1393，至今 open，复现形状正是循环创建销毁 io_context）。POSIX
    # 那条路径用的是持锁 signal_all 广播，结构上丢不掉，关掉 IOCP 就走这条。
    #
    # 代价是每次 poll() 要重建并扫描全部描述符，O(连接数) 而不是 O(就绪数)。没有连接数
    # 上限：asio 的 win_fd_set_adapter 是动态扩容的，Winsock 的 select 读 fd_count
    # 而不是 FD_SETSIZE。
    #
    # 必须是全局定义：只给部分 target 定义会让各 TU 看到不同的 asio 布局，构成 ODR 违规。
    add_compile_definitions(BOOST_ASIO_DISABLE_IOCP)
    # IOCP 关掉之后 ConnectEx 本来就走不到，留着是为了防止将来有人重新打开 IOCP。
    add_compile_definitions(BOOST_ASIO_DISABLE_CONNECTEX)
ELSEIF (APPLE)
    if (USE_COV)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -Wno-non-pod-varargs")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
    else ()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wno-non-pod-varargs")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
    endif ()
ELSE ()
    if (USE_COV)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
    else ()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
    endif ()
ENDIF ()

find_program(CCACHE_PROGRAM ccache)
if (CCACHE_PROGRAM)
    message("build with ccache")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif ()

add_subdirectory(src)
add_subdirectory(cmd)

enable_testing()
add_subdirectory(test)
add_subdirectory(benchmark)

option(BUILD_FUZZ "Build fuzz targets (requires clang)" OFF)
if (BUILD_FUZZ)
    add_subdirectory(fuzz)
endif ()
