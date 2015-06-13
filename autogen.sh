#! /bin/sh

ls -lh configure.ac

# Generate the aclocal.m4 file for automake, based on configure.in
aclocal

# Regenerate the files autoconf / automake
if [ "`(uname -s) 2>/dev/null`" = "Darwin" ]
then
    glibtoolize --force
else
    libtoolize --force --automake
fi

# Remove old cache files
rm -f config.cache
rm -f config.log

# Generate the configure script based on configure.in
autoconf

# Generate the Makefile.in
automake -a --foreign

