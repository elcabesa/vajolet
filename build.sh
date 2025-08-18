export CC=clang-9
export CXX=clang++-9

mkdir build
cd build
cmake -DVAJOLET_CPU_TYPE=64NEW  -DCMAKE_BUILD_TYPE=Debug ..
make

