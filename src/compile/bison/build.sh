#! /bin/sh
set -x

bison parser.y --html --graph --header=parser.h -o parser.cpp
export LC_ALL=en_US.UTF-8
bison parser.y --html --graph --header=parser.h -o parser.cpp
