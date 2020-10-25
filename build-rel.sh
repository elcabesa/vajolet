export CC=clang-11
export CXX=clang++-11

mkdir build-rel
cd build-rel
cmake -DVAJOLET_CPU_TYPE=64BMI2  -DCMAKE_BUILD_TYPE=Release ..
make

