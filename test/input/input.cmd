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

lifutils comp41 -x hpil -x hepax ..\data\audi2.txt | lifutils raw41lif AUDI2 | lifutils lifput liftest.dat
lifutils comp41 -x hpil -x hepax ..\data\prog41.txt | lifutils raw41lif TEST1 | lifutils lifput liftest.dat
lifutils textlif TXTA < ..\data\txta.txt | lifutils lifput liftest.dat
lifutils lifrename liftest.dat TXTA TXTB
lifutils textlif75 TXT75 < ..\data\txta.txt | lifutils lifput liftest.dat
type VERM1.rom | lifutils rom41hx VERMROM | lifutils lifput liftest.dat
type ..\data\txta.txt | lifutils textlif TXTA | lifutils lifput liftest.dat
lifutils lifput liftest.dat ..\data\dat1.lif
lifutils lifput liftest.dat ..\data\key1.lif
lifutils lifput liftest.dat ..\data\wall1.lif
lifutils lifput liftest.dat ..\data\stat1.lif
lifutils lifput liftest.dat ..\data\memt.lif
lifutils lifput liftest.dat ..\data\phycons.lif
lifutils lifstat liftest.dat > test.txt
python ..\difftool.py test.txt ..\data\lifstat_before_pack.txt

lifutils liflabel liftest.dat
lifutils lifdir liftest.dat > test.txt
python ..\difftool.py test.txt ..\data\lifdir_liftest_filled.txt

lifutils lifpurge liftest.dat VERMROM
lifutils lifpack liftest.dat
lifutils lifstat liftest.dat > test.txt
python ..\difftool.py test.txt ..\data\lifstat_after_pack.txt

lifutils lifdir liftest.dat > test.txt
python ..\difftool.py test.txt ..\data\lifdir_liftest_packed.txt

lifutils lifget  liftest.dat TXTA temp.lif  
lifutils liftext -r temp.lif > test.txt
python ..\difftool.py  test.txt ..\data\txta.txt

lifutils lifget liftest.dat TXT75 temp.lif  
lifutils liftext75 -r temp.lif  > test.txt
python ..\difftool.py  test.txt ..\data\txta.txt

lifutils lifget liftest.dat TEST1 temp.lif
lifutils decomp41 -x hpil -x hepax -r temp.lif > test.txt
python ..\difftool.py  test.txt ..\data\prog41.txt

lifutils lifget  liftest.dat DAT1 temp.lif
lifutils sdata -r temp.lif > test.txt
python ..\difftool.py  test.txt ..\data\dat1.txt

lifutils lifget liftest.dat DAT1 temp.lif
lifutils regs41 -r temp.lif > test.txt
python ..\difftool.py  test.txt ..\data\regs1.txt

lifutils lifget liftest.dat KEY1 temp.lif
lifutils key41 -r temp.lif > test.txt
python ..\difftool.py  test.txt ..\data\key1.txt

lifutils lifget liftest.dat STAT1 temp.lif
lifutils stat41 -r temp.lif > test.txt
python ..\difftool.py  test.txt ..\data\stat1.txt

lifutils lifget  liftest.dat WALL1 temp.lif
lifutils wcat41 -r temp.lif > test.txt
python ..\difftool.py  test.txt ..\data\wcat1.txt

lifutils rom41cat -x VERM1.rom > test.txt
python ..\difftool.py  test.txt ..\data\rom41cat1.txt

lifutils lifget -r liftest.dat DAT1 | lifutils sdatabar | lifutils barps > test.ps
python ..\difftool.py  test.ps ..\data\sdata.ps

lifutils lifget -r liftest.dat TEST1 | lifutils prog41bar | lifutils barps > test.ps
python ..\difftool.py  test.ps ..\data\prog.ps

lifutils lifget -r liftest.dat DAT1 | lifutils sdatabar | lifutils barprt > test.txt
python ..\difftool.py  test.txt ..\data\barprt.txt

lifutils lexcat71 -r ..\data\memt.lif > test.txt
python ..\difftool.py  test.txt ..\data\lexcat71.txt

lifutils lexcat75 -r ..\data\phycons.lif > test.txt
python ..\difftool.py  test.txt ..\data\lexcat75.txt

lifutils lifheader ..\data\memt.lif > test.txt
python ..\difftool.py  test.txt ..\data\lifheader.txt

lifutils rom41hx VERMROM < VERM1.rom | lifutils lifraw | lifutils hx41rom > tst.rom
python ..\difftool.py  --binary VERM1.rom tst.rom

lifutils rom41er VERMROM < VERM1.rom | lifutils lifraw | lifutils er41rom > tst.rom
python ..\difftool.py  --binary VERM1.rom tst.rom

lifutils lifget -r liftest.dat AUDI2 | lifutils outp41 | lifutils inp41 | lifutils decomp41 -x hpil -x hepax > test.txt
python ..\difftool.py  test.txt ..\data\audi2.txt 

lifutils wall41 -r -k -i ..\data\wall1.lif | key41 > test.txt
python ..\difftool.py  test.txt ..\data\key1.txt
lifutils wall41 -r -g -i ..\data\wall1.lif | sdata > test.txt
python ..\difftool.py  test.txt ..\data\dat1.txt
lifutils wall41 -r -g -i ..\data\wall1.lif | regs41 > test.txt
python ..\difftool.py  test.txt ..\data\regs1.txt
lifutils wall41 -r -s -i  ..\data\wall1.lif | stat41 > test.txt
python ..\difftool.py  test.txt ..\data\stat1w.txt
lifutils wall41 -r -p prog -i ..\data\wall1.lif 
lifutils decomp41 -x hpil -x hepax  prog.001 > test.txt
python ..\difftool.py  test.txt ..\data\audi2.txt

if exist temp.lif del /F temp.lif
if exist liftest.dat del /F liftest.dat
if exist tst.rom del /F tst.rom
if exist tst.ps del /F tst.ps
if exist liffix.dat del /F liffix.dat
if exist prog.ps del /F prog.ps
if exist VERM1.rom del /f VERM1.rom
if exist tst.txt del /F tst.txt
if exist prog.000 del /F prog.000
if exist prog.001 del /F prog.001
if exist prog.002 del /F prog.002


