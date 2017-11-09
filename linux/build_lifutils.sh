#!/bin/bash
pushd ..
rm -f ../lifutils_*.deb
rm -f ../lifutils_*.changes
dpkg-buildpackage -b -tc
mv ../lifutils_*.deb linux
mv ../lifutils_*.changes linux
popd
