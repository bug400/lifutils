#!/bin/bash
export version=`cat ../version.txt`
if [ -z "${version}" ] ; then
   echo "missing or empty version.txt file"
   exit 1
fi
if test "${1}" == "X86" ; then
   makensis -NOCD -DINPDIR=install32 -DVERSTR="${version}" ../nsis/lifutils.nsi
else
   makensis -NOCD -DINPDIR64=install64 -DVERSTR="${version}" ../nsis/lifutils.nsi
fi
