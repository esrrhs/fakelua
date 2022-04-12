#! /bin/sh

if [[ "$OSTYPE" == "linux-gnu" ]]; then
  echo "os "$OSTYPE

  flex -o ../lua.flex.cpp --header-file=../lua.flex.h lua.l

elif [[ "$OSTYPE" == "darwin"* ]]; then
  echo "os "$OSTYPE
else
  echo "not support "$OSTYPE
  exit 1
fi
