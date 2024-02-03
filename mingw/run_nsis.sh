#!/bin/bash
export version=`cat ../version.txt`
if [ -z "${version}" ] ; then
   echo "missing or empty version.txt file"
   exit 1
fi
if test "${1}" == "X86" ; then
   $NSISDIR/makensis -V4 -NOCD -INPUTCHARSET CP1252 -DINPDIR=install32 -DVERSTR="${version}" ../nsis/Setup.nsi
else
   $NSISDIR/makensis -V4 -NOCD -INPUTCHARSET CP1252 -DINPDIR64=install64 -DVERSTR="${version}" ../nsis/Setup.nsi
fi
