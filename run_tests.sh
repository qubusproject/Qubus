#!/bin/bash

mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DKUBUS_BUILD_TESTS=ON -DHPX_IGNORE_COMPILER_COMPATIBILITY=ON ../
make
ctest
