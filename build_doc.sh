#!/bin/bash

cd build
make doc
cp -r kubus/doc/* /home/gitlab/doc/latest
