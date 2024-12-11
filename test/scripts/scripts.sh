rm -rf liftest.dat
rm -f tst.rom
lifmod -v -e -f ../data/VERMROM.MOD > test.txt
python3 ../difftool.py  --binary VERM1.rom ../data/VERM1.ROM
python3 ../difftool.py test.txt ../data/mod1_lin.txt
cp ../data/hp41cass.dat liffix.dat
lifdir liffix.dat > test.txt
python3 ../difftool.py test.txt ../data/lifdir_before_fix.txt

liffix -m cass liffix.dat
lifdir liffix.dat > test.txt
python3 ../difftool.py test.txt ../data/lifdir_after_fix.txt

lifinit -m cass liftest.dat 60
liflabel liftest.dat TEST
lifdir liftest.dat > test.txt
python3 ../difftool.py test.txt ../data/lifdir_liftest_empty.txt

comp41 -x hpil -x hepax < ../data/audi2.txt | raw41lif AUDI2 | lifput liftest.dat
comp41 -x hpil -x hepax -f TEST1 < ../data/prog41.txt | lifput liftest.dat
textlif TXTA < ../data/txta.txt | lifput liftest.dat
textlif -s 0 TXT41 < ../data/txta.txt | lifput liftest.dat
textlif -s 42 TXT412 < ../data/txta.txt | lifput liftest.dat
lifrename liftest.dat TXTA TXTB
textlif75 TXT75 < ../data/txta.txt | lifput liftest.dat
textlif75 -n TXT75L < ../data/txta75.txt | lifput liftest.dat
cat VERM1.rom | rom41hx VERMROM | lifput liftest.dat
cat VERM1.rom | lifutils rom41lif VERMROML | lifutils lifput liftest.dat
cat VERM1.rom | lifutils rom41lif VERMROME | lifutils lifput liftest.dat
cat ../data/txta.txt | textlif TXTA | lifput liftest.dat
lifput liftest.dat ../data/dat1.lif
lifput liftest.dat ../data/key1.lif
lifput liftest.dat ../data/wall1.lif
lifput liftest.dat ../data/stat1.lif
lifput liftest.dat ../data/memt.lif
lifput liftest.dat ../data/wall1_long.lif 2> err_long.txt
python3 ../difftool.py err_long.txt ../data/err_long.txt
lifput liftest.dat ../data/wall1_short.lif 2> err_short.txt
python3 ../difftool.py err_short.txt ../data/err_short.txt

lifput liftest.dat ../data/phycons.lif
lifstat liftest.dat > test.txt
python3 ../difftool.py test.txt ../data/lifstat_before_pack.txt

liflabel liftest.dat
lifdir liftest.dat > test.txt
python3 ../difftool.py test.txt ../data/lifdir_liftest_filled.txt

lifpurge liftest.dat VERMROM
lifpack liftest.dat
liflabel -c liftest.dat
lifstat liftest.dat > test.txt
python3 ../difftool.py test.txt ../data/lifstat_after_pack.txt

lifstat liftest.dat 110 > test.txt
python3 ../difftool.py test.txt ../data/lifstat_memt.txt
lifstat liftest.dat 3 0 15 > test.txt
python3 ../difftool.py test.txt ../data/lifstat_memt.txt

lifget -b liftest.dat MEM > temp.lif
lifpurge liftest.dat MEM
lifput liftest.dat temp.lif

lifdir liftest.dat > test.txt
python3 ../difftool.py test.txt ../data/lifdir_liftest_packed.txt
lifdir -n liftest.dat > test.txt
python3 ../difftool.py test.txt ../data/lifdir_liftest_packed_names.txt
lifdir -v 2 liftest.dat > test.txt
python3 ../difftool.py test.txt ../data/lifdir_liftest_packed_verbose.txt
lifdir -c liftest.dat > test.txt
python3 ../difftool.py test.txt ../data/lifdir_liftest_packed_csv.txt

lifget liftest.dat TXTA  | lifraw | liftext > test.txt
python3 ../difftool.py  test.txt ../data/txta.txt

lifget liftest.dat TXT75  | lifraw | liftext75  > test.txt
python3 ../difftool.py  test.txt ../data/txta.txt

lifget liftest.dat TXT75L | lifraw | liftext75 > test.txt
python3 ../difftool.py  test.txt ../data/txta.txt
lifget liftest.dat TXT75L | lifraw | liftext75 -n > test.txt
python3 ../difftool.py  test.txt ../data/txta75.txt


lifget -r liftest.dat TEST1 |  decomp41 -x hpil -x hepax > test.txt
python3 ../difftool.py  test.txt ../data/prog41.txt

lifget -r liftest.dat DAT1 | sdata > test.txt
python3 ../difftool.py  test.txt ../data/dat1.txt
lifget -r liftest.dat DAT1 | sdata -h > test.txt
python3 ../difftool.py  test.txt ../data/dat1_hex.txt
lifget -r liftest.dat DAT1 | sdata -n -b -l > test.txt
python3 ../difftool.py  test.txt ../data/dat1_extended.txt

lifget -r liftest.dat DAT1 | regs41 > test.txt
python3 ../difftool.py  test.txt ../data/regs1.txt

lifget -r liftest.dat KEY1 | key41 -x hpdevices > test.txt
python3 ../difftool.py  test.txt ../data/key1.txt
lifutils lifget -r liftest.dat KEY1 | lifutils key41 -h > test.txt
python3 ../difftool.py  test.txt ../data/key1_hex.txt

lifget -r liftest.dat STAT1 | stat41 > test.txt
python3 ../difftool.py  test.txt ../data/stat1.txt
lifget -r liftest.dat STAT1 | stat41 -b -f -v > test.txt
python3 ../difftool.py  test.txt ../data/stat1_extended.txt


lifget -r liftest.dat WALL1 | wcat41 > test.txt
python3 ../difftool.py  test.txt ../data/wcat1.txt

cat VERM1.rom | rom41cat -x > test.txt
python3 ../difftool.py  test.txt ../data/rom41cat1.txt

lifget -r liftest.dat DAT1 | sdatabar | barps > test.ps
python3 ../difftool.py  test.ps ../data/sdata.ps

lifget -r liftest.dat TEST1 | prog41bar | barps > test.ps
python3 ../difftool.py  test.ps ../data/prog.ps

lifget -r liftest.dat DAT1 | sdatabar | barprt > test.txt
python3 ../difftool.py  test.txt ../data/barprt.txt

lifget -r liftest.dat MEM | lexcat71 > test.txt
python3 ../difftool.py  test.txt ../data/lexcat71.txt

lifget -r liftest.dat PHYCONS | lexcat75 > test.txt
python3 ../difftool.py  test.txt ../data/lexcat75.txt

lifheader ../data/memt.lif > test.txt
python3 ../difftool.py  test.txt ../data/lifheader.txt

rom41hx VERMROM < VERM1.rom | lifraw | hx41rom > tst.rom
python3 ../difftool.py  --binary VERM1.rom tst.rom

rom41er VERMROM < VERM1.rom | lifraw | er41rom > tst.rom
python3 ../difftool.py  --binary VERM1.rom tst.rom

lifget -r liftest.dat AUDI2 | outp41 | inp41 | decomp41 -x hpil -x hepax > test.txt
python3 ../difftool.py  test.txt ../data/audi2.txt 

wall41 -r -k < ../data/wall1.lif | key41 -x hpdevices > test.txt
python3 ../difftool.py  test.txt ../data/key1.txt
wall41 -r -g < ../data/wall1.lif | sdata > test.txt
python3 ../difftool.py  test.txt ../data/dat1.txt
wall41 -r -g < ../data/wall1.lif | regs41 > test.txt
python3 ../difftool.py  test.txt ../data/regs1.txt
wall41 -r -s < ../data/wall1.lif | stat41 > test.txt
python3 ../difftool.py  test.txt ../data/stat1w.txt
wall41 -r -p prog < ../data/wall1.lif 
decomp41 -x hpil -x hepax  < prog.001 > test.txt
python3 ../difftool.py  test.txt ../data/audi2.txt
rm -f err_long.txt 
rm -f err_short.txt
rm -rf liftest.dat
rm -f tst.rom
rm -f test.ps
rm -f temp.lif
rm -f VERM1.rom
rm -f liffix.dat
rm -f test.txt
rm -f prog.000
rm -f prog.001
rm -f prog.002
