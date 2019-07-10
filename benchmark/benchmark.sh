#! /bin/sh

echo "fake loop"
time ../bin/fakebin loop.fk
echo "------------"

echo "lua loop"
time lua loop.lua
echo "------------"

echo "fake prime"
time ../bin/fakebin prime.fk
echo "------------"

echo "lua prime"
time lua prime.lua
echo "------------"
