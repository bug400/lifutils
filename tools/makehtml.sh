#!/bin/bash
cd doc/html
rm -f *
cp ../../man/* .
mv barcode.5 barcode.1
for i in *.1 ; do cat $i | groff -Thtml -mman > `basename $i .1`.html; done
rm -f *.1
