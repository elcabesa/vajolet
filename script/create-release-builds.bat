pushd ..
mkdir release
popd

CALL create-nopopcnt-build-clang.bat
copy ..\build\src\Vajolet.exe ..\release\Vajolet2_2.8_old.exe
CALL clean.bat

CALL create-popcnt-build-clang.bat
copy ..\build\src\Vajolet.exe ..\release\Vajolet2_2.8.exe
CALL clean.bat

CALL create-bmi-build-clang.bat
copy ..\build\src\Vajolet.exe ..\release\Vajolet2_2.8_bmi.exe
CALL clean.bat
