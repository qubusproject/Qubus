#!/usr/bin/env bash
set -e

apt update
apt install -y g++ python3 python3-pip cmake ninja-build libicu-dev libgmp-dev libncurses-dev libgtest-dev

pip3 install conan

conan remote add ci-cache https://api.bintray.com/conan/qubusproject/ci-cache
conan remote add conan-community https://api.bintray.com/conan/conan-community/conan
conan remote add qubusproject https://api.bintray.com/conan/qubusproject/conan