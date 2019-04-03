set CC=clang --target=x86_64-mingw32
set CXX=clang++ --target=x86_64-mingw32

mkdir release
mkdir build
cd build
cmake -DVAJOLET_CPU_TYPE=64BMI2 -DCMAKE_BUILD_TYPE=Release -G "MinGW Makefiles" ..
mingw32-make
copy Vajolet.exe ..\release\Vajolet2_2.7_bmi.exe
cd ..
rmdir build /s /q

mkdir build
cd build
cmake -DVAJOLET_CPU_TYPE=64NEW -DCMAKE_BUILD_TYPE=Release -G "MinGW Makefiles" ..
mingw32-make
copy Vajolet.exe ..\release\Vajolet2_2.7.exe
cd ..
rmdir build /s /q

mkdir build
cd build
cmake -DVAJOLET_CPU_TYPE=64Old -DCMAKE_BUILD_TYPE=Release -G "MinGW Makefiles" ..
mingw32-make
copy Vajolet.exe ..\release\Vajolet2_2.7_old.exe
cd ..
rmdir build /s /q



