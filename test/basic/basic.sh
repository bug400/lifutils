rm -rf liftest.dat
rm -f tst.rom
rm -f sdata.ps
rm -f prog.ps
lifinit -m cass liftest.dat 60
liflabel liftest.dat TEST
comp41 -x hpil -x hepax < audi2.txt | raw41lif AUDI2 | lifput liftest.dat
comp41 -x hpil -x hepax < prog41.txt | raw41lif TEST1 | lifput liftest.dat
textlif TXTA < txta.txt | lifput liftest.dat
lifrename liftest.dat TXTA TXTB
textlif75 TXT75 < txta.txt | lifput liftest.dat
cat verm1.rom | rom41hx VERMROM | lifput liftest.dat
cat txta.txt | textlif TXTA | lifput liftest.dat
lifput liftest.dat data1.lif
lifput liftest.dat key1.lif
lifput liftest.dat wall1.lif
lifput liftest.dat stat1.lif
lifstat liftest.dat
liflabel liftest.dat
lifdir liftest.dat
lifpurge liftest.dat VERMROM
lifpack liftest.dat
lifstat liftest.dat
lifdir liftest.dat
echo "output of liftext..."
lifget -r liftest.dat TXTA  | liftext
echo "output of liftext75..."
lifget -r liftest.dat TXT75  | liftext75 -n
echo "output of decomp41..."
lifget -r liftest.dat TEST1 |  decomp41 -x hpil -x hepax
echo "Output of sdata..."
lifget -r liftest.dat DATA1 | sdata
echo "Output of regs41..."
lifget -r liftest.dat DATA1 | regs41
echo "Output of key41..."
lifget -r liftest.dat KEY1 | key41
echo "Output of wcat41..."
lifget -r liftest.dat WALL1 | wcat41
echo "Output of stat41..."
lifget -r liftest.dat STAT1 | stat41
echo "Output of ROM dump"
cat verm1.rom | rom41cat -x
lifget -r liftest.dat DATA1 | sdatabar | barps > sdata.ps
diff sdata.ps reference/sdata.ps
lifget -r liftest.dat TEST1 | prog41bar | barps > prog.ps 
diff prog.ps reference/prog.ps
lifget -r liftest.dat DATA1 | sdatabar | barprt
lifput liftest.dat memt.lex
lifput liftest.dat phycons.lif
echo "output of lex71"
lifget -r liftest.dat MEM | lexcat71
echo "output of lex75"
lifget -r liftest.dat PHYCONS | lexcat75
lifheader memt.lex
rom41hx VERMROM < verm1.rom | lifraw | hx41rom > tst.rom
diff verm1.rom tst.rom
rom41er VERMROM < verm1.rom | lifraw | er41rom > tst.rom
diff verm1.rom tst.rom
rm -rf liftest.dat
rm -f tst.rom
rm -f sdata.ps
rm -f prog.ps
