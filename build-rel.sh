export CC=clang-9
export CXX=clang++-9

mkdir build-rel
cd build-rel
cmake -DVAJOLET_CPU_TYPE=64NEW  -DCMAKE_BUILD_TYPE=Release ..
make

