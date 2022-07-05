#! /bin/sh
set -x
bison parser.y --html --graph --header=parser.h -o parser.cpp
