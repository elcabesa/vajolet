#!/usr/bin/env bash
pushd ..
mkdir release
popd

./clean.sh
./create-nopopcnt-build.sh
cp ../build/src/Vajolet.exe ../release/Vajolet2_3.2_old.exe
./clean.sh

./create-popcnt-build.sh
cp ../build/src/Vajolet.exe ../release/Vajolet2_3.2.exe
./clean.sh

./create-bmi2-build.sh
cp ../build/src/Vajolet.exe ../release/Vajolet2_3.2_bmi.exe
./clean.sh

