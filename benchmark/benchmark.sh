#! /bin/sh

TEST="loop prime string"
for i in $TEST; do
    echo "lua $i"
    time lua $i.lua
    echo "------------"

    echo "python $i"
    time python $i.py
    echo "------------"

    echo "fake $i"
    time ../bin/fakebin $i.fk
    echo "------------"

    echo "fake JIT $i"
    time ../bin/fakebin -j $i.fk
    echo "------------"
done
