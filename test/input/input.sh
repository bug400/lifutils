rm -rf liftest.dat
rm -f tst.rom
lifutils lifmod -v -e -f ../data/VERMROM.MOD > test.txt
python3 ../difftool.py  --binary VERM1.rom ../data/VERM1.ROM
python3 ../difftool.py test.txt ../data/mod1_lin.txt
cp ../data/hp41cass.dat liffix.dat
lifutils lifdir liffix.dat > test.txt
python3 ../difftool.py test.txt ../data/lifdir_before_fix.txt

lifutils liffix -m cass liffix.dat
lifutils lifdir liffix.dat > test.txt
python3 ../difftool.py test.txt ../data/lifdir_after_fix.txt

lifutils lifinit -m cass liftest.dat 60
lifutils liflabel liftest.dat TEST
lifutils lifdir liftest.dat > test.txt
python3 ../difftool.py test.txt ../data/lifdir_liftest_empty.txt

lifutils comp41 -x hpil -x hepax ../data/audi2.txt > temp.raw 
lifutils raw41lif AUDI2 temp.raw > temp.lif 
lifutils lifput liftest.dat temp.lif
lifutils comp41 -x hpil -x hepax -f TEST1 ../data/prog41.txt > temp.lif 
lifutils lifput liftest.dat temp.lif
lifutils textlif TXTA ../data/txta.txt | lifutils lifput liftest.dat
lifutils textlif -s 0 TXT41 ../data/txta.txt | lifutils lifput liftest.dat
lifutils textlif -s 42 TXT412 ../data/txta.txt | lifutils lifput liftest.dat
lifutils lifrename liftest.dat TXTA TXTB
lifutils textlif75 TXT75 ../data/txta.txt > temp.lif
lifutils lifput liftest.dat temp.lif
lifutils textlif75 -n TXT75L ../data/txta75.txt > temp.lif
lifutils lifput liftest.dat temp.lif
lifutils rom41hx VERMROM VERM1.rom > temp.lif
lifutils lifput liftest.dat temp.lif
lifutils rom41lif VERMROML VERM1.rom > temp.lif
lifutils lifput liftest.dat temp.lif
lifutils rom41lif VERMROME VERM1.rom > temp.lif
lifutils lifput liftest.dat temp.lif

lifutils textlif TXTA ../data/txta.txt | lifutils lifput liftest.dat
lifutils lifput liftest.dat ../data/dat1.lif
lifutils lifput liftest.dat ../data/key1.lif
lifutils lifput liftest.dat ../data/wall1.lif
lifutils lifput liftest.dat ../data/stat1.lif
lifutils lifput liftest.dat ../data/memt.lif
lifutils lifput liftest.dat ../data/wall1_long.lif 2> err_long.txt
python3 ../difftool.py err_long.txt ../data/err_long.txt
lifutils lifput liftest.dat ../data/wall1_short.lif 2> err_short.txt
python3 ../difftool.py err_short.txt ../data/err_short.txt
lifutils lifput liftest.dat ../data/phycons.lif
lifutils lifstat liftest.dat > test.txt
python3 ../difftool.py test.txt ../data/lifstat_before_pack.txt

lifutils liflabel liftest.dat
lifutils lifdir liftest.dat > test.txt
python3 ../difftool.py test.txt ../data/lifdir_liftest_filled.txt

lifutils lifpurge liftest.dat VERMROM
lifutils lifpack liftest.dat
lifutils liflabel -c liftest.dat
lifutils lifstat liftest.dat > test.txt
python3 ../difftool.py test.txt ../data/lifstat_after_pack.txt

lifutils lifstat liftest.dat 110 > test.txt
python3 ../difftool.py test.txt ../data/lifstat_memt.txt
lifutils lifstat liftest.dat 3 0 15 > test.txt
python3 ../difftool.py test.txt ../data/lifstat_memt.txt

lifutils lifget -b liftest.dat MEM temp.lif
lifutils lifpurge liftest.dat MEM
lifutils lifput liftest.dat temp.lif


lifutils lifdir liftest.dat > test.txt
python3 ../difftool.py test.txt ../data/lifdir_liftest_packed.txt
lifutils lifdir -n liftest.dat > test.txt
python3 ../difftool.py test.txt ../data/lifdir_liftest_packed_names.txt
lifutils lifdir -v 2 liftest.dat > test.txt
python3 ../difftool.py test.txt ../data/lifdir_liftest_packed_verbose.txt
lifutils lifdir -c liftest.dat > test.txt
python3 ../difftool.py test.txt ../data/lifdir_liftest_packed_csv.txt


lifutils lifget  liftest.dat TXTA temp.lif  
lifutils lifraw temp.lif > temp.raw
lifutils liftext  temp.raw > test.txt
python3 ../difftool.py  test.txt ../data/txta.txt

lifutils lifget liftest.dat TXT75 temp.lif  
lifutils lifraw temp.lif > temp.raw
lifutils liftext75  temp.raw  > test.txt
python3 ../difftool.py  test.txt ../data/txta.txt

lifutils lifget liftest.dat TXT75L temp.lif 
lifutils lifraw temp.lif > temp.raw 
lifutils liftext75 temp.raw > test.txt
python3 ../difftool.py  test.txt ../data/txta.txt

lifutils lifget liftest.dat TXT75L temp.lif 
lifutils lifraw temp.lif > temp.raw 
lifutils liftext75 -n  temp.raw > test.txt
python3 ../difftool.py  test.txt ../data/txta75.txt

lifutils lifget liftest.dat TEST1 temp.lif
lifutils decomp41 -x hpil -x hepax -r temp.lif > test.txt
python3 ../difftool.py  test.txt ../data/prog41.txt

lifutils lifget  liftest.dat DAT1 temp.lif
lifutils sdata -r temp.lif > test.txt
python3 ../difftool.py  test.txt ../data/dat1.txt
lifutils lifget liftest.dat DAT1 temp.lif
lifutils sdata -r -h temp.lif > test.txt
python3 ../difftool.py  test.txt ../data/dat1_hex.txt
lifutils lifget liftest.dat DAT1 temp.lif
lifutils sdata -r -n -b -l temp.lif > test.txt
python3 ../difftool.py  test.txt ../data/dat1_extended.txt

lifutils lifget liftest.dat DAT1 temp.lif
lifutils regs41 -r temp.lif > test.txt
python3 ../difftool.py  test.txt ../data/regs1.txt

lifutils lifget liftest.dat KEY1 temp.lif
lifutils key41 -r -x hpdevices temp.lif > test.txt
python3 ../difftool.py  test.txt ../data/key1.txt

lifutils lifget liftest.dat KEY1 temp.lif
lifutils key41 -r -h  temp.lif > test.txt
python3 ../difftool.py  test.txt ../data/key1_hex.txt

lifutils lifget liftest.dat STAT1 temp.lif
lifutils stat41 -r temp.lif > test.txt
python3 ../difftool.py  test.txt ../data/stat1.txt
lifutils lifget liftest.dat STAT1 temp.lif
lifutils stat41 -r -b -f -v temp.lif > test.txt
python3 ../difftool.py  test.txt ../data/stat1_extended.txt

lifutils lifget  liftest.dat WALL1 temp.lif
lifutils wcat41 -r temp.lif > test.txt
python3 ../difftool.py  test.txt ../data/wcat1.txt

lifutils rom41cat -x VERM1.rom > test.txt
python3 ../difftool.py  test.txt ../data/rom41cat1.txt

lifutils lifget  liftest.dat DAT1 temp.lif 
lifutils sdatabar -r temp.lif | lifutils barps > test.ps
python3 ../difftool.py  test.ps ../data/sdata.ps

lifutils lifget  liftest.dat TEST1 temp.lif
lifutils prog41bar -r temp.lif | lifutils barps > test.ps
python3 ../difftool.py  test.ps ../data/prog.ps

lifutils lifget -r liftest.dat DAT1 | lifutils sdatabar | lifutils barprt > test.txt
python3 ../difftool.py  test.txt ../data/barprt.txt

lifutils lexcat71 -r ../data/memt.lif > test.txt
python3 ../difftool.py  test.txt ../data/lexcat71.txt

lifutils lexcat75 -r ../data/phycons.lif > test.txt
python3 ../difftool.py  test.txt ../data/lexcat75.txt

lifutils lifheader ../data/memt.lif > test.txt
python3 ../difftool.py  test.txt ../data/lifheader.txt

lifutils rom41hx VERMROM VERM1.rom > temp.lif
lifutils hx41rom -r temp.lif > tst.rom
python3 ../difftool.py  --binary VERM1.rom tst.rom

lifutils rom41er VERMROM VERM1.rom > temp.lif 
lifutils er41rom -r temp.lif > tst.rom
python3 ../difftool.py  --binary VERM1.rom tst.rom

lifutils lifget -r liftest.dat AUDI2 | lifutils outp41 | lifutils inp41 | lifutils decomp41 -x hpil -x hepax > test.txt
python3 ../difftool.py  test.txt ../data/audi2.txt 

lifutils wall41 -r -k -i ../data/wall1.lif | key41 -x hpdevices > test.txt
python3 ../difftool.py  test.txt ../data/key1.txt
lifutils wall41 -r -g -i ../data/wall1.lif | sdata > test.txt
python3 ../difftool.py  test.txt ../data/dat1.txt
lifutils wall41 -r -g -i ../data/wall1.lif | regs41 > test.txt
python3 ../difftool.py  test.txt ../data/regs1.txt
lifutils wall41 -r -s -i  ../data/wall1.lif | stat41 > test.txt
python3 ../difftool.py  test.txt ../data/stat1w.txt
lifutils wall41 -r -p prog -i ../data/wall1.lif 
lifutils decomp41 -x hpil -x hepax  prog.001 > test.txt
python3 ../difftool.py  test.txt ../data/audi2.txt
rm -f err_long.txt 
rm -f err_short.txt
rm -f temp.lif
rm -f temp.raw
rm -f liftest.dat
rm -f tst.rom
rm -f test.ps
rm -f VERM1.rom
rm -f liffix.dat
rm -f test.txt
rm -f prog.000
rm -f prog.001
rm -f prog.002
