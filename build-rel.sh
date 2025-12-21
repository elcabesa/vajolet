export CC=gcc-13
export CXX=g++-13

mkdir build-rel
cd build-rel
cmake -DVAJOLET_CPU_TYPE=64BMI2  -DCMAKE_BUILD_TYPE=Release ..
make

