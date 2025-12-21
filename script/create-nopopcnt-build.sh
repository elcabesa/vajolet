#!/usr/bin/env bash

pushd .. 
mkdir build 
pushd build 
cmake -DVAJOLET_CPU_TYPE=64OLD -DCMAKE_BUILD_TYPE=Release ..
make Vajolet
popd
popd

