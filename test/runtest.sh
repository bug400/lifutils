#!/bin/bash
#
# Ctest test driver for LIFUTILS
#
# Parameters:
# 1: name of test file
# 2: path to executables
# 3: path to scripts
# 4: path to xroms directory
#
if test -z "${1}" ; then
    testfile="regression"
else
    testfile=$1
fi
if test ! -z "${2}" ; then
    export PATH=$2:$PATH
fi
if test ! -z "${3}" ; then
    export PATH=$3:$PATH
fi
if test ! -z "${4}" ; then
    export LIFUTILSXROMDIR=$4
fi
    
export LIFUTILSREGRESSIONTEST=1
cd ${testfile}
rm -f ${testfile}.out
./${testfile}.sh > ${testfile}.out
diff ${testfile}.out reference/${testfile}.out
ret=$?
rm -f ${testfile}.out
exit $ret
