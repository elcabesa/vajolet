pushd ..
mkdir release
popd

create-nopopcnt-build-clang.bat
copy ..\build\src\Vajolet.exe ..\release\Vajolet2_2.7_old.exe
clean.bat

create-popcnt-build-clang.bat
copy ..\build\src\Vajolet.exe ..\release\Vajolet2_2.7.exe
clean.bat

create-bmi-build-clang.bat
copy ..\build\src\Vajolet.exe ..\release\Vajolet2_2.7_bmi.exe
clean.bat





