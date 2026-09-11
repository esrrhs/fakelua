#! /bin/sh
set -e
cd "$(dirname "$0")/.."
exec ctest --output-on-failure "$@"
