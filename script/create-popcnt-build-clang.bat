set CC=clang --target=x86_64-mingw32
set CXX=clang++ --target=x86_64-mingw32
pushd .. & mkdir build & pushd build & cmake -DVAJOLET_CPU_TYPE=64NEW -DCMAKE_BUILD_TYPE=Release -G "MinGW Makefiles" .. & mingw32-make & popd & popd

