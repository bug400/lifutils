#!/bin/bash
cd pdf
rm -f *.pdf
rm -f *.gz
rm -f *.1
cp ../manpages/*.gz .
for i in *.gz ; do gzip -d $i ; done
mv barcode.5 barcode.1
for i in *.1 ; do man -Tps -l $i  | ps2pdf - `basename $i .1`.pdf; done
rm -f *.1
rm inp41.pdf
rm outp41.pdf
rm in71.pdf
rm out71.pdf
rm lifilper.pdf
