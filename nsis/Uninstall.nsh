Var SemiSilentMode ; installer started uninstaller in semi-silent mode using /SS parameter
Var RunningFromInstaller ; installer started uninstaller using /uninstall parameter

; Installer Attributes
ShowUninstDetails show 

; Pages
!define MUI_UNABORTWARNING ; Show a confirmation when cancelling the installation

!define MULTIUSER_INSTALLMODE_CHANGE_MODE_UNFUNCTION un.PageInstallModeChangeMode
!insertmacro MULTIUSER_UNPAGE_INSTALLMODE

!define MUI_PAGE_CUSTOMFUNCTION_PRE un.PageComponentsPre
!define MUI_PAGE_CUSTOMFUNCTION_SHOW un.PageComponentsShow

!insertmacro MUI_UNPAGE_INSTFILES

Section "un.Program Files" SectionUninstallProgram
	SectionIn RO

	; Try to delete the EXE as the first step - if it's in use, don't remove anything else
	!insertmacro un.DeleteRetryAbort "$INSTDIR\barprt.exe"
	!insertmacro un.DeleteRetryAbort "$INSTDIR\barps.exe"
	!insertmacro un.DeleteRetryAbort "$INSTDIR\comp41.exe"
	!insertmacro un.DeleteRetryAbort "$INSTDIR\decomp41.exe"
	!insertmacro un.DeleteRetryAbort "$INSTDIR\emu7470.exe"
	!insertmacro un.DeleteRetryAbort "$INSTDIR\hx41rom.exe"
	!insertmacro un.DeleteRetryAbort "$INSTDIR\er41rom.exe"
	!insertmacro un.DeleteRetryAbort "$INSTDIR\key41.exe"
	!insertmacro un.DeleteRetryAbort "$INSTDIR\lexcat71.exe"
	!insertmacro un.DeleteRetryAbort "$INSTDIR\lifdir.exe"
	!insertmacro un.DeleteRetryAbort "$INSTDIR\liffix.exe"
	!insertmacro un.DeleteRetryAbort "$INSTDIR\lifget.exe"
	!insertmacro un.DeleteRetryAbort "$INSTDIR\lifheader.exe"
	!insertmacro un.DeleteRetryAbort "$INSTDIR\lifinit.exe"
	!insertmacro un.DeleteRetryAbort "$INSTDIR\liflabel.exe"
	!insertmacro un.DeleteRetryAbort "$INSTDIR\lifmod.exe"
	!insertmacro un.DeleteRetryAbort "$INSTDIR\lifpack.exe"
	!insertmacro un.DeleteRetryAbort "$INSTDIR\lifpurge.exe"
	!insertmacro un.DeleteRetryAbort "$INSTDIR\lifput.exe"
	!insertmacro un.DeleteRetryAbort "$INSTDIR\lifraw.exe"
	!insertmacro un.DeleteRetryAbort "$INSTDIR\lifrename.exe"
	!insertmacro un.DeleteRetryAbort "$INSTDIR\lifstat.exe"
	!insertmacro un.DeleteRetryAbort "$INSTDIR\liftext.exe"
	!insertmacro un.DeleteRetryAbort "$INSTDIR\lifversion.exe"
	!insertmacro un.DeleteRetryAbort "$INSTDIR\prog41bar.exe"
	!insertmacro un.DeleteRetryAbort "$INSTDIR\raw41lif.exe"
	!insertmacro un.DeleteRetryAbort "$INSTDIR\regs41.exe"
	!insertmacro un.DeleteRetryAbort "$INSTDIR\rom41cat.exe"
	!insertmacro un.DeleteRetryAbort "$INSTDIR\rom41er.exe"
	!insertmacro un.DeleteRetryAbort "$INSTDIR\rom41hx.exe"
	!insertmacro un.DeleteRetryAbort "$INSTDIR\sdatabar.exe"
	!insertmacro un.DeleteRetryAbort "$INSTDIR\sdata.exe"
	!insertmacro un.DeleteRetryAbort "$INSTDIR\stat41.exe"
	!insertmacro un.DeleteRetryAbort "$INSTDIR\text75.exe"
	!insertmacro un.DeleteRetryAbort "$INSTDIR\textlif.exe"
	!insertmacro un.DeleteRetryAbort "$INSTDIR\wall41.exe"
	!insertmacro un.DeleteRetryAbort "$INSTDIR\wcat41.exe"
	!insertmacro un.DeleteRetryAbort "$INSTDIR\activate_lifutils.bat"

        !insertmacro un.DeleteRetryAbort "$INSTDIR\xroms\advantage.xrom"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\xroms\alpha.xrom"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\xroms\cardrdr.xrom"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\xroms\devil.xrom"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\xroms\hepax.xrom"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\xroms\hpil.xrom"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\xroms\plotter.xrom"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\xroms\printer.xrom"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\xroms\timecx.xrom"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\xroms\time.xrom"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\xroms\wand.xrom"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\xroms\xfncx.xrom"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\xroms\xfn.xrom"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\xroms\xio.xrom"
	RMdir "$INSTDIR\xroms"

        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\html\comp41.html"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\html\decomp41.html"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\html\hx41rom.html"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\html\er41rom.html"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\html\key41.html"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\html\lexcat71.html"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\html\lifdir.html"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\html\liffix.html"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\html\lifget.html"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\html\lifheader.html"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\html\lifinit.html"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\html\liflabel.html"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\html\lifmod.html"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\html\lifpack.html"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\html\lifpurge.html"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\html\lifput.html"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\html\lifraw.html"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\html\lifrename.html"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\html\lifstat.html"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\html\liftext.html"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\html\lifversion.html"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\html\prog41bar.html"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\html\raw41lif.html"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\html\regs41.html"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\html\rom41cat.html"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\html\rom41er.html"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\html\rom41hx.html"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\html\sdatabar.html"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\html\sdata.html"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\html\stat41.html"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\html\text75.html"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\html\textlif.html"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\html\wall41.html"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\html\wcat41.html"
	RMdir "$INSTDIR\doc\html"

        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\hardware\barcode.asm"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\hardware\barcode.hex"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\hardware\barcode.lst"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\hardware\barcode-schematic"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\hardware\barcode-test.c"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\hardware\barcode-wand-mods"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\hardware\Centronics-to-GPIO"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\hardware\HP41-PC-barcode-download"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\hardware\HP41-PC-parallel-download"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\hardware\HP41-PC-serial-transfer"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\hardware\HP71-PC-parallel-download"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\hardware\HP71-PC-serial-transfer"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\hardware\setrs_hp71.txt"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\hardware\setrs.p41"
	RMdir "$INSTDIR\doc\hardware"

        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\copying.html"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\GPL-2"
        !insertmacro un.DeleteRetryAbort "$INSTDIR\doc\readme.html"
	RMdir "$INSTDIR\doc"
	
  ; Clean up "Program Group entries" we check that we created Start menu folder
	${if} "$StartMenuFolder" != ""
                !insertmacro un.DeleteRetryAbort "$SMPROGRAMS\$StartMenuFolder\Lifutils Prompt.lnk"
                !insertmacro un.DeleteRetryAbort "$SMPROGRAMS\$StartMenuFolder\Documentation.lnk"
		RMDir "$SMPROGRAMS\$StartMenuFolder"
	${endif}	

  ; Remove path
	!insertmacro LIFUTILS_REMOVE_PATH $INSTDIR $MultiUser.InstallMode

  ; Remove environment varialbe LIFUTILSXEMOMDIR
 	!insertmacro LIFUTILS_REMOVE_ENVIRONMENT "LIFUTILSXROMDIR" $MultiUser.InstallMode
		
SectionEnd

Section "-Uninstall" ; hidden section, must always be the last one!
	; Remove the uninstaller from registry as the very last step - if sth. goes wrong, let the user run it again
	!insertmacro MULTIUSER_RegistryRemoveInstallInfo ; Remove registry keys
		
  Delete "$INSTDIR\${UNINSTALL_FILENAME}"	
  ; remove the directory only if it is empty - the user might have saved some files in it		
	RMDir "$INSTDIR"  		
SectionEnd

; Callbacks
Function un.onInit	
	${GetParameters} $R0
		
	${GetOptions} $R0 "/uninstall" $R1
	${ifnot} ${errors}	
		StrCpy $RunningFromInstaller 1		
	${else}
		StrCpy $RunningFromInstaller 0
	${endif}
	
	${GetOptions} $R0 "/SS" $R1
	${ifnot} ${errors}		
		StrCpy $SemiSilentMode 1
		SetAutoClose true ; auto close (if no errors) if we are called from the installer; if there are errors, will be automatically set to false
	${else}
		StrCpy $SemiSilentMode 0
	${endif}		
	
	${ifnot} ${UAC_IsInnerInstance}
		${andif} $RunningFromInstaller$SemiSilentMode == "00"
		!insertmacro CheckSingleInstance "${SINGLE_INSTANCE_ID}"
	${endif}		
		
	!insertmacro MULTIUSER_UNINIT		
FunctionEnd

Function un.PageInstallModeChangeMode
	!insertmacro MUI_STARTMENU_GETFOLDER "" $StartMenuFolder
FunctionEnd

Function un.PageComponentsPre
	${if} $SemiSilentMode == 1
		Abort ; if user is installing, no use to remove program settings anyway (should be compatible with all versions)
	${endif}
FunctionEnd

Function un.PageComponentsShow
	; Show/hide the Back button 
	GetDlgItem $0 $HWNDPARENT 3 
	ShowWindow $0 $UninstallShowBackButton
FunctionEnd

Function un.onUninstFailed
	${if} $SemiSilentMode == 0
		MessageBox MB_ICONSTOP "${PRODUCT_NAME} ${VERSION} could not be fully uninstalled.$\r$\nPlease, restart Windows and run the uninstaller again." /SD IDOK	
	${else}
		MessageBox MB_ICONSTOP "${PRODUCT_NAME} could not be fully installed.$\r$\nPlease, restart Windows and run the setup program again." /SD IDOK	
	${endif}
FunctionEnd
