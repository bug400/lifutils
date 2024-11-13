#!/bin/bash
# copy files from a LIF image file to another new LIF image file
# check number of input parameters
if test $# -ne 4 
then
   echo "Usage: lifcopy lif-image-filename1 lif-image-filename2 medium-type dir-entries"
   exit 1
fi
# check if input file exists
if  test ! -r "${1}"
then
   echo "Input LIF image file does not exist"
   exit 1
fi
# check if output file does not exist
if test  -f "${2}"
then
   echo "Output LIF image file already exists"
   exit 1
fi
# initialize new image file
lifutils lifinit -m ${3} ${2} ${4}
if test $? -ne 0; then
   exit 1
fi
# copy content
for i in `lifutils lifdir -n ${1}`; do 
   lifutils lifget ${1} ${i} | lifutils lifput ${2} 
   if test $? -ne 0; then
      exit 1
   fi
done
exit 0
