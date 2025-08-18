pushd ..
mkdir release
popd

CALL clean.bat
CALL create-nopopcnt-build-clang.bat
copy ..\build\src\Vajolet.exe ..\release\Vajolet2_2.9_old.exe
CALL clean.bat

CALL create-popcnt-build-clang.bat
copy ..\build\src\Vajolet.exe ..\release\Vajolet2_2.9.exe
CALL clean.bat

CALL create-bmi2-build-clang.bat
copy ..\build\src\Vajolet.exe ..\release\Vajolet2_2.9_bmi.exe
CALL clean.bat
