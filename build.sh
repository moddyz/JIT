#!/bin/bash

set -euxo pipefail

mkdir -p build && cd build

# Only build if installation path not specified.
if [ $# -eq 0 ]
then
    cmake ..
    cmake --build .
else
    cmake -DCMAKE_INSTALL_PREFIX=$1 ..
    cmake --build . --target install
fi
