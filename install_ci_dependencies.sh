#!/usr/bin/env bash

wget https://qubusproject.org/carrot/downloads/src/carrot-0.2.0-dev-src.tar.bz2

tar xvf carrot-0.2.0-dev-src.tar.bz2

cd carrot-0.2.0-dev-src

mkdir build
cd build

cmake -DCMAKE_BUILD_TYPE=Release ..

make

sudo make install