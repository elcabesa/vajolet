export CC=gcc
export CXX=g++

mkdir build-rel
cd build-rel
cmake -DVAJOLET_CPU_TYPE=64BMI2  -DCMAKE_BUILD_TYPE=Release ..
make

