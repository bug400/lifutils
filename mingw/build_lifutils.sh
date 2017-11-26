#!/bin/bash
#
# mingw 32 bit cross compile
#
if [ "`dirname $0`" != "." ] ; then
   echo "script must be called from its subdirectory"
   exit 1
fi
this_dir=`pwd`
pushd .. > /dev/null
rm -rf build
mkdir build
pushd build > /dev/null
cmake .. -DCMAKE_TOOLCHAIN_FILE=${this_dir}/mingw32.cmake -DCMAKE_INSTALL_PREFIX=${this_dir}/install32 -DUSER_ROOT_PATH=`pwd`
make
make install
popd > /dev/null
popd > /dev/null
./run_nsis.sh X86
rm -rf install32
#
# mingw 64 bit cross compile
#
this_dir=`pwd`
pushd .. > /dev/null
rm -rf build
mkdir build
pushd build > /dev/null
cmake .. -DCMAKE_TOOLCHAIN_FILE=${this_dir}/mingw64.cmake -DCMAKE_INSTALL_PREFIX=${this_dir}/install64 -DUSER_ROOT_PATH=`pwd`
make
make install
popd > /dev/null
rm -rf build
popd > /dev/null
./run_nsis.sh i686
rm -rf install64
