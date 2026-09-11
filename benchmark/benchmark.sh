#! /bin/sh
set -e
cd "$(dirname "$0")"
if [ ! -x ../bin/fake_bench ]; then
    echo "build fake_bench first: cmake --build .. --target fake_bench" >&2
    exit 1
fi
exec ../bin/fake_bench "$@"
