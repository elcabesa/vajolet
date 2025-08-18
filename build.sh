export CC=gcc
export CXX=g++

mkdir build
cd build
cmake -DVAJOLET_CPU_TYPE=64NEW  -DCMAKE_BUILD_TYPE=Debug ..
make

