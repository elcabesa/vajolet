pushd .. & mkdir build & pushd build & cmake -DVAJOLET_CPU_TYPE=64NEW -DCMAKE_BUILD_TYPE=Release -G "MinGW Makefiles" .. & mingw32-make & popd & popd

