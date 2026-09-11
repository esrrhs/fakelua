#! /bin/sh

set -e

BUILD_FLAG=""

if [ "$#" = 1 ] && [ "$1" = "release" ]; then
    BUILD_FLAG="-DFAKE_RELEASE=ON"
fi

rm -rf CMakeCache.txt CMakeFiles cmake_install.cmake Makefile \
    src/CMakeFiles src/Makefile src/cmake_install.cmake \
    test/CMakeFiles test/Makefile test/cmake_install.cmake \
    CTestTestfile.cmake src/CTestTestfile.cmake test/CTestTestfile.cmake \
    Testing DartConfiguration.tcl

cmake . $BUILD_FLAG
cmake --build . --parallel

echo "build ok"
