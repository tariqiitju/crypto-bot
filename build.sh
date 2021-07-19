#!/bin/bash

mkdir -p build
cd build
# conan install ..
conan install .. --build=missing
# cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
# cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE=Release
cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE=Debug
cmake --build .

### TODO
## introduce a makefile
## clean -> delete build directoroty
## resolve -> conan install .. --build=missing