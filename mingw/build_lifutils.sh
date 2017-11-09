#!/bin/bash
#
# mingw 32 bit cross compile
#
this_dir=`pwd`
pushd ..
rm -rf build
mkdir build
pushd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=${this_dir}/mingw32.cmake -DCMAKE_INSTALL_PREFIX=${this_dir}/install32 -DUSER_ROOT_PATH=`pwd`
make
make install
popd
popd
./run_nsis.sh X86
rm -rf install32
#
# mingw 64 bit cross compile
#
this_dir=`pwd`
pushd ..
rm -rf build
mkdir build
pushd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=${this_dir}/mingw64.cmake -DCMAKE_INSTALL_PREFIX=${this_dir}/install64 -DUSER_ROOT_PATH=`pwd`
make
make install
popd
rm -rf build
popd
./run_nsis.sh i686
rm -rf install64
