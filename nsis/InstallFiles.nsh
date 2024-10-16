;
;	Install files
;
	SetOutPath "$INSTDIR"
	File "${SRC}\barprt.exe"
	File "${SRC}\barps.exe"
	File "${SRC}\comp41.exe"
	File "${SRC}\decomp41.exe"
	File "${SRC}\emu7470.exe"
	File "${SRC}\hx41rom.exe"
	File "${SRC}\key41.exe"
	File "${SRC}\lexcat71.exe"
	File "${SRC}\lexcat75.exe"
	File "${SRC}\lifdir.exe"
	File "${SRC}\liffix.exe"
	File "${SRC}\lifget.exe"
	File "${SRC}\lifheader.exe"
	File "${SRC}\lifinit.exe"
	File "${SRC}\liflabel.exe"
	File "${SRC}\lifmod.exe"
	File "${SRC}\lifpack.exe"
	File "${SRC}\lifpurge.exe"
	File "${SRC}\lifput.exe"
	File "${SRC}\lifraw.exe"
	File "${SRC}\lifrename.exe"
	File "${SRC}\lifstat.exe"
	File "${SRC}\liftext.exe"
	File "${SRC}\lifversion.exe"
	File "${SRC}\prog41bar.exe"
	File "${SRC}\raw41lif.exe"
	File "${SRC}\regs41.exe"
	File "${SRC}\rom41cat.exe"
	File "${SRC}\er41rom.exe"
	File "${SRC}\rom41er.exe"
	File "${SRC}\rom41lif.exe"
	File "${SRC}\rom41hx.exe"
	File "${SRC}\sdatabar.exe"
	File "${SRC}\sdata.exe"
	File "${SRC}\stat41.exe"
	File "${SRC}\liftext75.exe"
	File "${SRC}\textlif75.exe"
	File "${SRC}\textlif.exe"
	File "${SRC}\wall41.exe"
	File "${SRC}\wcat41.exe"
	File "${SRC}\in71.exe"
	File "${SRC}\out71.exe"
	File "${SRC}\inp41.exe"
	File "${SRC}\outp41.exe"
	createDirectory "$INSTDIR\xroms"
	SetOutPath "$INSTDIR\xroms"
        FILE "${SRC}\xroms\advantage.xrom"
        FILE "${SRC}\xroms\alpha.xrom"
        FILE "${SRC}\xroms\aviation.xrom"
        FILE "${SRC}\xroms\cardrdr.xrom"
        FILE "${SRC}\xroms\ccd.xrom"
        FILE "${SRC}\xroms\circuits.xrom"
        FILE "${SRC}\xroms\clinical.xrom"
        FILE "${SRC}\xroms\dataacq.xrom"
        FILE "${SRC}\xroms\devil.xrom"
        FILE "${SRC}\xroms\finance.xrom"
        FILE "${SRC}\xroms\games.xrom"
        FILE "${SRC}\xroms\hepax.xrom"
        FILE "${SRC}\xroms\homemgmt.xrom"
        FILE "${SRC}\xroms\hpdevices.xrom"
        FILE "${SRC}\xroms\hpil.xrom"
        FILE "${SRC}\xroms\machine.xrom"
        FILE "${SRC}\xroms\math.xrom"
        FILE "${SRC}\xroms\melrom.xrom"
        FILE "${SRC}\xroms\navigation.xrom"
        FILE "${SRC}\xroms\petroleum.xrom"
        FILE "${SRC}\xroms\plotter.xrom"
        FILE "${SRC}\xroms\ppc.xrom"
        FILE "${SRC}\xroms\printer.xrom"
        FILE "${SRC}\xroms\realestate.xrom"
        FILE "${SRC}\xroms\standard.xrom"
        FILE "${SRC}\xroms\statistics.xrom"
        FILE "${SRC}\xroms\struct.xrom"
        FILE "${SRC}\xroms\stress.xrom"
        FILE "${SRC}\xroms\surveying.xrom"
        FILE "${SRC}\xroms\thermal.xrom"
        FILE "${SRC}\xroms\timecx.xrom"
        FILE "${SRC}\xroms\time.xrom"
        FILE "${SRC}\xroms\wand.xrom"
        FILE "${SRC}\xroms\xfncx.xrom"
        FILE "${SRC}\xroms\xfn.xrom"
        FILE "${SRC}\xroms\xio.xrom"

	createDirectory "$INSTDIR\doc"
	SetOutPath "$INSTDIR\doc"
        FILE "${SRC}\doc\copying.html"
        FILE "${SRC}\doc\GPL-2"
        FILE "${SRC}\doc\readme.html"
        FILE "${SRC}\doc\tutorial.html"

	createDirectory "$INSTDIR\doc\html"
	SetOutPath "$INSTDIR\doc\html"
        FILE "${SRC}\doc\html\barprt.html"
        FILE "${SRC}\doc\html\barps.html"
        FILE "${SRC}\doc\html\comp41.html"
        FILE "${SRC}\doc\html\decomp41.html"
        FILE "${SRC}\doc\html\hx41rom.html"
        FILE "${SRC}\doc\html\key41.html"
        FILE "${SRC}\doc\html\lexcat71.html"
        FILE "${SRC}\doc\html\lexcat75.html"
        FILE "${SRC}\doc\html\lifdir.html"
        FILE "${SRC}\doc\html\liffix.html"
        FILE "${SRC}\doc\html\lifget.html"
        FILE "${SRC}\doc\html\lifheader.html"
        FILE "${SRC}\doc\html\lifinit.html"
        FILE "${SRC}\doc\html\liflabel.html"
        FILE "${SRC}\doc\html\lifmod.html"
        FILE "${SRC}\doc\html\lifpack.html"
        FILE "${SRC}\doc\html\lifpurge.html"
        FILE "${SRC}\doc\html\lifput.html"
        FILE "${SRC}\doc\html\lifraw.html"
        FILE "${SRC}\doc\html\lifrename.html"
        FILE "${SRC}\doc\html\lifstat.html"
        FILE "${SRC}\doc\html\liftext.html"
        FILE "${SRC}\doc\html\lifversion.html"
        FILE "${SRC}\doc\html\prog41bar.html"
        FILE "${SRC}\doc\html\raw41lif.html"
        FILE "${SRC}\doc\html\regs41.html"
        FILE "${SRC}\doc\html\rom41cat.html"
        FILE "${SRC}\doc\html\rom41er.html"
        FILE "${SRC}\doc\html\rom41lif.html"
        FILE "${SRC}\doc\html\er41rom.html"
        FILE "${SRC}\doc\html\rom41hx.html"
        FILE "${SRC}\doc\html\sdatabar.html"
        FILE "${SRC}\doc\html\sdata.html"
        FILE "${SRC}\doc\html\stat41.html"
        FILE "${SRC}\doc\html\liftext75.html"
        FILE "${SRC}\doc\html\textlif75.html"
        FILE "${SRC}\doc\html\textlif.html"
        FILE "${SRC}\doc\html\wall41.html"
        FILE "${SRC}\doc\html\wcat41.html"
        FILE "${SRC}\doc\html\in71.html"
        FILE "${SRC}\doc\html\out71.html"
        FILE "${SRC}\doc\html\inp41.html"
        FILE "${SRC}\doc\html\outp41.html"

	createDirectory "$INSTDIR\doc\hardware"
	SetOutPath "$INSTDIR\doc\hardware"
        FILE "${SRC}\doc\hardware\barcode.asm"
        FILE "${SRC}\doc\hardware\barcode.hex"
        FILE "${SRC}\doc\hardware\barcode.lst"
        FILE "${SRC}\doc\hardware\barcode-schematic"
        FILE "${SRC}\doc\hardware\barcode-test.c"
        FILE "${SRC}\doc\hardware\barcode-wand-mods"
        FILE "${SRC}\doc\hardware\Centronics-to-GPIO"
        FILE "${SRC}\doc\hardware\HP41-PC-barcode-download"
        FILE "${SRC}\doc\hardware\HP41-PC-parallel-download"
        FILE "${SRC}\doc\hardware\HP41-PC-serial-transfer"
        FILE "${SRC}\doc\hardware\HP71-PC-parallel-download"
        FILE "${SRC}\doc\hardware\HP71-PC-serial-transfer"
        FILE "${SRC}\doc\hardware\setrs_hp71.txt"
        FILE "${SRC}\doc\hardware\setrs.p41"
;
; write file activate.bat
;
        FileOpen $4 "$INSTDIR\activate_${PRODUCT_NAME}.bat" w 
        FileWrite $4 "@PATH=$INSTDIR;%PATH%$\r$\n"
        FileWrite $4 "@SET LIFUTILSXROMDIR=$INSTDIR\xroms$\r$\n"
        FileClose $4
