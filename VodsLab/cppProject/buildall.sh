#!/bin/bash
set -e
# Build Linux executable
mkdir -p build-linux
cd build-linux
cmake ..
make
cd ..
# Build Windows executable (cross-compile)
mkdir -p build-mingw
cd build-mingw
cmake .. -DCMAKE_TOOLCHAIN_FILE=../mingw-w64-toolchain.cmake
make
cd ..
