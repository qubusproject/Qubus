#!/bin/bash

mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DHPX_IGNORE_COMPILER_COMPATIBILITY=ON ../
make doc