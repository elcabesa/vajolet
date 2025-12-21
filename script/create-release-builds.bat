pushd ..
mkdir release
popd

CALL clean.bat
CALL create-nopopcnt-build-clang.bat
copy ..\build\src\Vajolet.exe ..\release\Vajolet2_3.0_old.exe
CALL clean.bat

CALL create-popcnt-build-clang.bat
copy ..\build\src\Vajolet.exe ..\release\Vajolet2_3.0.exe
CALL clean.bat

CALL create-bmi2-build-clang.bat
copy ..\build\src\Vajolet.exe ..\release\Vajolet2_3.0_bmi.exe
CALL clean.bat
