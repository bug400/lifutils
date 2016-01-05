#!/bin/bash
cd doc/html
rm -f *.htnl
cp ../../man/*.1 .
cp ../../man/barcode.5 .
mv barcode.5 barcode.1
for i in *.1 ; do cat $i | groff -Thtml -mman > `basename $i .1`.html; done
rm -f *.1
