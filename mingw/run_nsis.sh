#!/bin/bash
if test "${1}" == "X86" ; then
   makensis -NOCD -DINPDIR=install32 -DVERSTR="1.7.7b1" ../nsis/lifutils.nsi
else
   makensis -NOCD -DINPDIR64=install64 -DVERSTR="1.7.7b1" ../nsis/lifutils.nsi
fi
