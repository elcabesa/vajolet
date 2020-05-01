export CC=clang-10
export CXX=clang++-10

mkdir build-rel
cd build-rel
cmake -DVAJOLET_CPU_TYPE=64NEW  -DCMAKE_BUILD_TYPE=Release ..
make

