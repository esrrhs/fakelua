#! /bin/sh

BUILD_FLAG=""

if [ $# == 1 ] && [ $1 == "release" ];then
    BUILD_FLAG=" -DREMOD=ON"
fi

cd flexbison
flex -o ../src/flex.cpp flex.l
bison -v -t -d bison.y
mv bison.tab.c ../src/bison.cpp
mv bison.tab.h	../src/bison.h
cd ..

#lib
rm CMakeCache.txt -rf
rm CMakeFiles -rf
rm cmake_install.cmake -rf
rm Makefile -rf
cmake . $BUILD_FLAG
make clean
make -j5

#test
cd test
rm CMakeCache.txt -rf
rm CMakeFiles -rf
rm cmake_install.cmake -rf
rm Makefile -rf
cmake . $BUILD_FLAG
make clean
make -j5
cd ..

#fake
cd fake
rm CMakeCache.txt -rf
rm CMakeFiles -rf
rm cmake_install.cmake -rf
rm Makefile -rf
cmake . $BUILD_FLAG
make clean
make -j5
cd ..

#testjit
cd testjit
rm CMakeCache.txt -rf
rm CMakeFiles -rf
rm cmake_install.cmake -rf
rm Makefile -rf
cmake . $BUILD_FLAG
make clean
make -j5
cd ..

#luatest
cd luatest
rm CMakeCache.txt -rf
rm CMakeFiles -rf
rm cmake_install.cmake -rf
rm Makefile -rf
cmake . $BUILD_FLAG
make clean
make -j5

echo "build ok"
