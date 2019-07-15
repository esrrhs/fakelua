#! /bin/sh

echo "fake loop"
time ../bin/fakebin loop.fk
echo "------------"

echo "lua loop"
time lua loop.lua
echo "------------"

echo "python loop"
time python loop.py
echo "------------"

echo "fake prime"
time ../bin/fakebin prime.fk
echo "------------"

echo "lua prime"
time lua prime.lua
echo "------------"

echo "python prime"
time python prime.py
echo "------------"
