#!/usr/bin/env bash
set -e

apt update
apt install -y g++ python3 python3-pip

pip3 install conan

conan install -g conan_paths -b outdated CI/