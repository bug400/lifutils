#!/bin/bash
if [ "`dirname $0`" != "." ] ; then
   echo "script must be called from its subdirectory"
   exit 1
fi
pushd .. > /dev/null
rm -f ../lifutils_*.deb
rm -f ../lifutils_*.changes
dpkg-buildpackage -b -tc -uc
mv ../lifutils_*.deb linux
mv ../lifutils_*.changes linux
rm -f ../lifutils-dbgsym*.deb
rm -f ../lifutils_*buldinfo
popd > /dev/null
