@echo off
if exist liftest.dat del /F liftest.dat
if exist tst.rom del /F tst.rom

lifutils lifmod -v -e -f ..\data\VERMROM.MOD > test.txt
python ..\difftool.py  --binary VERM1.rom ..\data\VERM1.ROM
python ..\difftool.py test.txt ..\data\mod1_win.txt
copy ..\data\hp41cass.dat liffix.dat > nul
lifutils lifdir liffix.dat > test.txt
python ..\difftool.py test.txt ..\data\lifdir_before_fix.txt

lifutils liffix -m cass liffix.dat
lifutils lifdir liffix.dat > test.txt
python ..\difftool.py test.txt ..\data\lifdir_after_fix.txt

lifutils lifinit -m cass liftest.dat 60
lifutils liflabel liftest.dat TEST
lifutils lifdir liftest.dat > test.txt
python ..\difftool.py test.txt ..\data\lifdir_liftest_empty.txt

lifutils comp41 -x hpil -x hepax < ..\data\audi2.txt | lifutils raw41lif AUDI2 | lifutils lifput liftest.dat
lifutils comp41 -x hpil -f TEST1 -x hepax < ..\data\prog41.txt | lifutils lifput liftest.dat
lifutils textlif TXTA < ..\data\txta.txt | lifutils lifput liftest.dat
lifutils textlif -s 0 TXT41 < ..\data\txta.txt | lifutils lifput liftest.dat
lifutils textlif -s 42 TXT412 < ..\data\txta.txt | lifutils lifput liftest.dat
lifutils lifrename liftest.dat TXTA TXTB
lifutils textlif75 TXT75 < ..\data\txta.txt | lifutils lifput liftest.dat
lifutils textlif75 TXT75 < ..\data\txta.txt | lifutils lifput liftest.dat
lifutils textlif75 -n TXT75L < ..\data\txta75.txt | lifutils lifput liftest.dat
type VERM1.rom | lifutils rom41hx VERMROM | lifutils lifput liftest.dat
type VERM1.rom | lifutils rom41lif VERMROML | lifutils lifput liftest.dat
type VERM1.rom | lifutils rom41er VERMROME | lifutils lifput liftest.dat
type ..\data\txta.txt | lifutils textlif TXTA | lifutils lifput liftest.dat
lifutils lifput liftest.dat ..\data\dat1.lif
lifutils lifput liftest.dat ..\data\key1.lif
lifutils lifput liftest.dat ..\data\wall1.lif
lifutils lifput liftest.dat ..\data\stat1.lif
lifutils lifput liftest.dat ..\data\memt.lif
lifutils lifput liftest.dat ..\data\wall1_long.lif 2> err_long.txt
python ..\difftool.py err_long.txt ..\data\err_long.txt
lifutils lifput liftest.dat ..\data\wall1_short.lif 2> err_short.txt
python ..\difftool.py err_short.txt ..\data\err_short.txt
lifutils lifput liftest.dat ..\data\phycons.lif
lifutils lifstat liftest.dat > test.txt
python ..\difftool.py test.txt ..\data\lifstat_before_pack.txt

lifutils liflabel liftest.dat
lifutils lifdir liftest.dat > test.txt
python ..\difftool.py test.txt ..\data\lifdir_liftest_filled.txt

lifutils lifpurge liftest.dat VERMROM
lifutils lifpack liftest.dat
lifutils liflabel -c liftest.dat
lifutils lifstat liftest.dat > test.txt
python ..\difftool.py test.txt ..\data\lifstat_after_pack.txt

lifutils lifstat liftest.dat 97 > test.txt
python ..\difftool.py test.txt ..\data\lifstat_memt.txt
lifutils lifstat liftest.dat 3 0 2 > test.txt
python ..\difftool.py test.txt ..\data\lifstat_memt.txt

lifutils lifget -b liftest.dat MEM > temp.lif
lifutils lifpurge liftest.dat MEM
lifutils lifput liftest.dat temp.lif

lifutils lifdir liftest.dat > test.txt
python ..\difftool.py test.txt ..\data\lifdir_liftest_packed.txt
lifutils lifdir -n liftest.dat > test.txt
python ..\difftool.py test.txt ..\data\lifdir_liftest_packed_names.txt
lifutils lifdir -v 2 liftest.dat > test.txt
python ..\difftool.py test.txt ..\data\lifdir_liftest_packed_verbose.txt
lifutils lifdir -c liftest.dat > test.txt
python ..\difftool.py test.txt ..\data\lifdir_liftest_packed_csv.txt

lifutils lifget  liftest.dat TXTA  | lifutils lifraw | lifutils liftext > test.txt
python ..\difftool.py  test.txt ..\data\txta.txt

lifutils lifget  liftest.dat TXT75  | lifutils lifraw | lifutils liftext75  > test.txt
python ..\difftool.py  test.txt ..\data\txta.txt

lifutils lifget liftest.dat TXT75L | lifutils lifraw | lifutils liftext75 > test.txt
python ..\difftool.py  test.txt ..\data\txta.txt
lifutils lifget liftest.dat TXT75L | lifutils lifraw | lifutils liftext75 -n > test.txt
python ..\difftool.py  test.txt ..\data\txta75.txt


lifutils lifget -r liftest.dat TEST1 |  lifutils decomp41 -x hpil -x hepax > test.txt
python ..\difftool.py  test.txt ..\data\prog41.txt

lifutils lifget -r liftest.dat DAT1 | lifutils sdata > test.txt
python ..\difftool.py  test.txt ..\data\dat1.txt
lifutils lifget -r liftest.dat DAT1 | lifutils sdata -h > test.txt
python ..\difftool.py  test.txt ..\data\dat1_hex.txt
lifutils lifget -r liftest.dat DAT1 | lifutils sdata -n -b -l > test.txt
python ..\difftool.py  test.txt ..\data\dat1_extended.txt

lifutils lifget -r liftest.dat DAT1 | lifutils regs41 > test.txt
python ..\difftool.py  test.txt ..\data\regs1.txt

lifutils lifget -r liftest.dat KEY1 | lifutils key41 -x hpdevices > test.txt
python ..\difftool.py  test.txt ..\data\key1.txt
lifutils lifget -r liftest.dat KEY1 | lifutils key41 -h > test.txt
python ..\difftool.py  test.txt ..\data\key1_hex.txt

lifutils lifget -r liftest.dat STAT1 | lifutils stat41 > test.txt
python ..\difftool.py  test.txt ..\data\stat1.txt
lifutils lifget -r liftest.dat STAT1 | lifutils stat41 -b -f -v > test.txt
python ..\difftool.py  test.txt ..\data\stat1_extended.txt


lifutils lifget -r liftest.dat WALL1 | lifutils wcat41 > test.txt
python ..\difftool.py  test.txt ..\data\wcat1.txt

type VERM1.rom | lifutils rom41cat -x > test.txt
python ..\difftool.py  test.txt ..\data\rom41cat1.txt

lifutils lifget -r liftest.dat DAT1 | lifutils sdatabar | lifutils barps > test.ps
python ..\difftool.py  test.ps ..\data\sdata.ps

lifutils lifget -r liftest.dat TEST1 | lifutils prog41bar | lifutils barps > test.ps
python ..\difftool.py  test.ps ..\data\prog.ps

lifutils lifget -r liftest.dat DAT1 | lifutils sdatabar | lifutils barprt > test.txt
python ..\difftool.py  test.txt ..\data\barprt.txt

lifutils lifget -r liftest.dat MEM | lifutils lexcat71 > test.txt
python ..\difftool.py  test.txt ..\data\lexcat71.txt

lifutils lifget -r liftest.dat PHYCONS | lifutils lexcat75 > test.txt
python ..\difftool.py  test.txt ..\data\lexcat75.txt

lifutils lifheader ..\data\memt.lif > test.txt
python ..\difftool.py  test.txt ..\data\lifheader.txt

lifutils rom41hx VERMROM < VERM1.rom | lifutils lifraw | lifutils hx41rom > tst.rom
python ..\difftool.py  --binary VERM1.rom tst.rom

lifutils rom41er VERMROM < VERM1.rom | lifutils lifraw | lifutils er41rom > tst.rom
python ..\difftool.py  --binary VERM1.rom tst.rom

lifutils lifget -r liftest.dat AUDI2 | lifutils outp41 | lifutils inp41 | lifutils decomp41 -x hpil -x hepax > test.txt
python ..\difftool.py  test.txt ..\data\audi2.txt 

lifutils wall41 -r -k < ..\data\wall1.lif | key41 -x hpdevices > test.txt
python ..\difftool.py  test.txt ..\data\key1.txt
lifutils wall41 -r -g < ..\data\wall1.lif | sdata > test.txt
python ..\difftool.py  test.txt ..\data\dat1.txt
lifutils wall41 -r -g < ..\data\wall1.lif | regs41 > test.txt
python ..\difftool.py  test.txt ..\data\regs1.txt
lifutils wall41 -r -s < ..\data\wall1.lif | stat41 > test.txt
python ..\difftool.py  test.txt ..\data\stat1w.txt
lifutils wall41 -r -p prog < ..\data\wall1.lif 
lifutils decomp41 -x hpil -x hepax  < prog.001 > test.txt
python ..\difftool.py  test.txt ..\data\audi2.txt

if exist err_long.txt del /F err_long.txt
if exist err_short.txt del /F err_short.txt
if exist test.ps del /F test.ps
if exist test.txt del /F test.txt
if exist liftest.dat del /F liftest.dat
if exist tst.rom del /F tst.rom
if exist tst.ps del /F tst.ps
if exist temp.lif del /F temp.lif
if exist liffix.dat del /F liffix.dat
if exist prog.ps del /F prog.ps
if exist VERM1.rom del /f VERM1.rom
if exist tst.txt del /F tst.txt
if exist prog.000 del /F prog.000
if exist prog.001 del /F prog.001
if exist prog.002 del /F prog.002
