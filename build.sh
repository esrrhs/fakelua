#! /bin/sh
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
cmake .
make clean
make -j

#test
cd test
rm CMakeCache.txt -rf
rm CMakeFiles -rf
rm cmake_install.cmake -rf
rm Makefile -rf
cmake .
make clean
make -j
cd ..

#luatest
cd luatest
rm CMakeCache.txt -rf
rm CMakeFiles -rf
rm cmake_install.cmake -rf
rm Makefile -rf
cmake .
make clean
make -j

echo "build ok"
