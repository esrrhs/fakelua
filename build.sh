#! /bin/sh

BUILD_FLAG=""

if [ $# == 1 ] && [ $1 == "release" ];then
    BUILD_FLAG=" -DREMOD=ON"
fi

if [[ "$OSTYPE" == "linux-gnu" ]]; then
    echo "os "$OSTYPE

    cd src/flexbison
    flex -o ../flex.cpp flex.l
    bison -v -t -d bison.y
    mv bison.tab.c ../bison.cpp
    mv bison.tab.h	../bison.h
    cd ../../

elif [[ "$OSTYPE" == "darwin"* ]]; then
    echo "os "$OSTYPE
else
	echo "not support "$OSTYPE
	exit 1
fi

#lib
rm -rf CMakeCache.txt
rm -rf CMakeFiles
rm -rf cmake_install.cmake
rm -rf Makefile
cmake . $BUILD_FLAG
make clean
make 

if [ $? -ne 0 ];then
	echo "build lib fail"
	exit 1
fi

echo "build ok"
