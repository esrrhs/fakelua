#! /bin/sh

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

