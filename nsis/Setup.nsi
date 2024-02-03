; Command line arguments:
; makensis -DINPDIR=<input-dir> -DINPDIR64=<input-dir-64-bit> \
;   -DOUTFILE=<output-file> -DVERSTR=<version-string> Main.nsi
; Main nsis configuration file for the lifutils and asm71 package
;
; Issues:
; - does always require elevation if deinstalled from "Apps & Features"
;
Unicode true ; properly display all languages (Installer will not work on Windows 95, 98 or ME!)
;SetCompressor /SOLID lzma
!include MUI2.nsh
!include UAC.nsh
!include NsisMultiUser.nsh
!include LogicLib.nsh
!include StdUtils.nsh

;
; Installer defines -- product related
!include "..\nsis\Configuration.nsh"
;
; Installer defines -- not product related
!define MIN_WIN_VER "XP"
!define SINGLE_INSTANCE_ID "${PRODUCT_UUID}" ; do not change this between program versions!
!define COMPANY_NAME "bug400" ; company, used for registry tree hierarchy
!define CONTACT "" ; stored as the contact information in the uninstall info of the registry
!define SETUP_MUTEX "${COMPANY_NAME} ${PRODUCT_NAME} Setup Mutex" ; do not change this between program versions!
!define APP_MUTEX "${COMPANY_NAME} ${PRODUCT_NAME} App Mutex" ; do not change this between program versions!
!define SETTINGS_REG_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_UUID}"
!define LICENSE_FILE "${SRC}/doc/GPL-2" ; license file, optional
!define MULTIUSER_INSTALLMODE_UNINSTALL_REGISTRY_KEY "${PRODUCT_UUID}"
;
; additional includes
!include "..\nsis\Path.nsh"
!include "..\nsis\Utils.nsh"

;
; Set environment according to settings of INPDIR or INPDIR64
!ifdef INPDIR
        !define SRC "${INPDIR}"
        !define PLATFORM "Win32"
!endif
!ifdef INPDIR64
        !define SRC "${INPDIR64}"
        !define PLATFORM "Win64"
!endif

!ifndef OUTFILE
        !ifdef INPDIR
                !define OUTFILE "${PRODUCT_NAME}-win32-setup.exe"
        !endif
        !ifdef INPDIR64
                !define OUTFILE "${PRODUCT_NAME}-win64-setup.exe"
        !endif
!endif

;
; Macro definitions
!macro ADD_PATH directory mode
       Push ${directory}
       Push ${mode}
       Call AddToPath
!macroend

!macro REMOVE_PATH directory mode
       Push ${directory}
       Push ${mode}
       Call un.RemoveFromPath
!macroend

!macro SET_ENVIRONMENT variable value mode
        ${if} ${mode} == "AllUsers"
                WriteRegExpandStr SHCTX "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" ${variable} ${value} 
        ${else}
                WriteRegExpandStr SHCTX "Environment" ${variable} ${mode}
        ${endif}
        SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=5000
!macroend
;
; remove environment variable accroding to mode
;
!macro REMOVE_ENVIRONMENT variable mode
        ${if} ${mode} == "AllUsers"
                DeleteRegValue SHCTX "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" ${variable}
        ${else}
                DeleteRegValue SHCTX "Environment" ${variable}
        ${endif}
	ClearErrors
        SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=5000
!macroend

; NsisMultiUser optional defines
!define MULTIUSER_INSTALLMODE_ALLOW_BOTH_INSTALLATIONS 0
!define MULTIUSER_INSTALLMODE_ALLOW_ELEVATION 1
!define MULTIUSER_INSTALLMODE_ALLOW_ELEVATION_IF_SILENT 0
!define MULTIUSER_INSTALLMODE_DEFAULT_ALLUSERS 0
!define MULTIUSER_INSTALLMODE_DEFAULT_CURRENTUSER  1
!if ${PLATFORM} == "Win64"
	!define MULTIUSER_INSTALLMODE_64_BIT 1
!endif
!define MULTIUSER_INSTALLMODE_DISPLAYNAME "${PRODUCT_NAME} ${VERSION} ${PLATFORM}"

; Variables
Var StartMenuFolder
Var PathModified

; Installer Attributes
Name "${PRODUCT_NAME} v${VERSION} ${PLATFORM}"
OutFile "${OUTFILE}"
BrandingText "©2019 ${COMPANY_NAME}"

AllowSkipFiles off
SetOverwrite on ; (default setting) set to on except for where it is manually switched off
ShowInstDetails show


; Interface Settings
!define MUI_ABORTWARNING ; Show a confirmation when cancelling the installation
!define MUI_LANGDLL_ALLLANGUAGES ; Show all languages, despite user's codepage

; Remember the installer language
!define MUI_LANGDLL_REGISTRY_ROOT SHCTX
!define MUI_LANGDLL_REGISTRY_KEY "${SETTINGS_REG_KEY}"
!define MUI_LANGDLL_REGISTRY_VALUENAME "Language"

; Pages
;
; Welcome page
!define MUI_PAGE_CUSTOMFUNCTION_PRE PageWelcomeLicensePre
!insertmacro MUI_PAGE_WELCOME

;
; License page, if LICENSE_FILE defined
!ifdef LICENSE_FILE
	!define MUI_PAGE_CUSTOMFUNCTION_PRE PageWelcomeLicensePre
	!insertmacro MUI_PAGE_LICENSE "${LICENSE_FILE}"
!endif

;
; Installation mode (all users, single user)
!define MULTIUSER_INSTALLMODE_CHANGE_MODE_FUNCTION PageInstallModeChangeMode
!insertmacro MULTIUSER_PAGE_INSTALLMODE

;
; Components page
!define MUI_COMPONENTSPAGE_SMALLDESC
!insertmacro MUI_PAGE_COMPONENTS
;
; Directory page
!define MUI_PAGE_CUSTOMFUNCTION_PRE PageDirectoryPre
!define MUI_PAGE_CUSTOMFUNCTION_SHOW PageDirectoryShow
!insertmacro MUI_PAGE_DIRECTORY
;
; Start menu page
!define MUI_STARTMENUPAGE_NODISABLE ; Do not display the checkbox to disable the creation of Start Menu shortcuts
!define MUI_STARTMENUPAGE_DEFAULTFOLDER "${PRODUCT_NAME}"
!define MUI_STARTMENUPAGE_REGISTRY_ROOT SHCTX ; writing to $StartMenuFolder happens in MUI_STARTMENU_WRITE_END, so it's safe to use SHCTX here
!define MUI_STARTMENUPAGE_REGISTRY_KEY "${SETTINGS_REG_KEY}"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "StartMenuFolder"
!define MUI_PAGE_CUSTOMFUNCTION_PRE PageStartMenuPre
!insertmacro MUI_PAGE_STARTMENU "" "$StartMenuFolder"
;
; Install files page
!define MUI_PAGE_CUSTOMFUNCTION_LEAVE PageInstFilesLeave
!insertmacro MUI_PAGE_INSTFILES


!define MUI_STARTMENUPAGE_DEFAULTFOLDER "${PRODUCT_NAME}" ; the MUI_PAGE_STARTMENU macro undefines MUI_STARTMENUPAGE_DEFAULTFOLDER, but we need it

; remove next line if you're using signing after the uninstaller is extracted from the initially compiled setup
!include "..\nsis\UninstallPages.nsh"

; Languages (first is default language) - must be inserted after all pages
!insertmacro MUI_LANGUAGE "English"
;!insertmacro MUI_LANGUAGE "German"
!insertmacro MULTIUSER_LANGUAGE_INIT

; Reserve files
!insertmacro MUI_RESERVEFILE_LANGDLL

; Functions
Function CheckInstallation 
	; if there's an installed version, uninstall it first (I chose not to start the uninstaller silently, so that user sees what failed)
	; if both per-user and per-machine versions are installed, unistall the one that matches $MultiUser.InstallMode
	StrCpy $0 ""
	${if} $HasCurrentModeInstallation = 1
		StrCpy $0 "$MultiUser.InstallMode"
	${else}
		!if ${MULTIUSER_INSTALLMODE_ALLOW_BOTH_INSTALLATIONS} = 0
			${if} $HasPerMachineInstallation = 1
				StrCpy $0 "AllUsers" ; if there's no per-user installation, but there's per-machine installation, uninstall it
			${elseif} $HasPerUserInstallation = 1
				StrCpy $0 "CurrentUser" ; if there's no per-machine installation, but there's per-user installation, uninstall it
			${endif}
		!endif
	${endif}

	${if} "$0" != ""
		${if} $0 == "AllUsers"
			StrCpy $1 "$PerMachineUninstallString"
			StrCpy $3 "$PerMachineInstallationFolder"
		${else}
			StrCpy $1 "$PerUserUninstallString"
			StrCpy $3 "$PerUserInstallationFolder"
		${endif}
		${if} ${silent}
			StrCpy $2 "/S"
		${else}
			StrCpy $2 ""
		${endif}
	${endif}
FunctionEnd

Function RunUninstaller
	StrCpy $0 0
	ExecWait '$1 /SS $2 _?=$3' $0 ; $1 is quoted in registry; the _? param stops the uninstaller from copying itself to the temporary directory, which is the only way for ExecWait to work
FunctionEnd

; Sections
InstType "Typical"
InstType "Minimal"
InstType "Full"

Section "Core Files (required)" SectionCoreFiles
	SectionIn 1 2 3 RO

	!insertmacro UAC_AsUser_Call Function CheckInstallation ${UAC_SYNCREGISTERS}
	${if} $0 != ""
		HideWindow
		ClearErrors
		${if} $0 == "AllUsers"
			Call RunUninstaller
  		${else}
			!insertmacro UAC_AsUser_Call Function RunUninstaller ${UAC_SYNCREGISTERS}
  		${endif}
		${if} ${errors} ; stay in installer
			SetErrorLevel 2 ; Installation aborted by script
			BringToFront
			Abort "Error executing uninstaller."
		${else}
			${Switch} $0
				${Case} 0 ; uninstaller completed successfully - continue with installation
					BringToFront
					Sleep 1000 ; wait for cmd.exe (called by the uninstaller) to finish
					${Break}
				${Case} 1 ; Installation aborted by user (cancel button)
				${Case} 2 ; Installation aborted by script
					SetErrorLevel $0
					Quit ; uninstaller was started, but completed with errors - Quit installer
				${Default} ; all other error codes - uninstaller could not start, elevate, etc. - Abort installer
					SetErrorLevel $0
					BringToFront
					Abort "Error executing uninstaller."
			${EndSwitch}
		${endif}

		; Just a failsafe - should've been taken care of by cmd.exe
		!insertmacro DeleteRetryAbort "$3\${UNINSTALL_FILENAME}" ; the uninstaller doesn't delete itself when not copied to the temp directory
		RMDir "$3"
	${endif}

	SetOutPath $INSTDIR
	; Write uninstaller and registry uninstall info as the first step,
	; so that the user has the option to run the uninstaller if sth. goes wrong
	WriteUninstaller "${UNINSTALL_FILENAME}"
	; or this if you're using signing:
	; File "${UNINSTALL_FILENAME}"
	!insertmacro MULTIUSER_RegistryAddInstallInfo ; add registry keys
	${if} ${silent} ; MUI doesn't write language in silent mode
	    WriteRegStr "${MUI_LANGDLL_REGISTRY_ROOT}" "${MUI_LANGDLL_REGISTRY_KEY}" "${MUI_LANGDLL_REGISTRY_VALUENAME}" $LANGUAGE
	${endif}

;
; add files to install 
!include "..\nsis\InstallFiles.nsh"

SectionEnd

SectionGroup /e "Integration" SectionGroupIntegration
Section "Program Group" SectionProgramGroup
	SectionIn 1 3

	!insertmacro MUI_STARTMENU_WRITE_BEGIN ""

		CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
		CreateShortCut "$SMPROGRAMS\$StartMenuFolder\${PRODUCT_NAME} Prompt.lnk" "%windir%\system32\cmd.exe" "/K $\"$INSTDIR\activate_${PRODUCT_NAME}.bat$\"" "%windir%\system32\cmd.exe" 0 SW_SHOWNORMAL ALT|CONTROL|SHIFT|F5 "${PRODUCT_NAME} command prompt"

		CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Documentation.lnk" "$INSTDIR\doc\${PROGHTML}"

		${if} $MultiUser.InstallMode == "AllUsers"
			CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\${UNINSTALL_FILENAME}" "/allusers"
		${else}
			CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\${UNINSTALL_FILENAME}" "/currentuser"
		${endif}

	!insertmacro MUI_STARTMENU_WRITE_END
SectionEnd

!include "..\nsis\InstallComponentSections.nsh"

SectionGroupEnd

Section "-Write Install Size" ; hidden section, write install size as the final step
	!insertmacro MULTIUSER_RegistryAddInstallSizeInfo
SectionEnd

; Modern install component descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
	!insertmacro MUI_DESCRIPTION_TEXT ${SectionCoreFiles} "Core files requred to run ${PRODUCT_NAME}."

	!insertmacro MUI_DESCRIPTION_TEXT ${SectionGroupIntegration} "Select how to integrate the program in Windows."
	!insertmacro MUI_DESCRIPTION_TEXT ${SectionProgramGroup} "Create a ${PRODUCT_NAME} program group under Start Menu > Programs."
!include "..\nsis\InstallComponentFunctionDescription.nsh"

!insertmacro MUI_FUNCTION_DESCRIPTION_END

; Callbacks
Function .onInit
	strcpy $PathModified "0"
	!insertmacro CheckPlatform ${PLATFORM}
	!insertmacro CheckMinWinVer ${MIN_WIN_VER}
	${ifnot} ${UAC_IsInnerInstance}
		!insertmacro CheckSingleInstance "Setup" "Global" "${SETUP_MUTEX}"
		!insertmacro CheckSingleInstance "Application" "Local" "${APP_MUTEX}"
	${endif}

	!insertmacro MULTIUSER_INIT

	${if} $IsInnerInstance = 0
		!insertmacro MUI_LANGDLL_DISPLAY
	${endif}
FunctionEnd

Function PageWelcomeLicensePre
	${if} $InstallShowPagesBeforeComponents = 0
		Abort ; don't display the Welcome and License pages
	${endif}
FunctionEnd

Function PageInstallModeChangeMode
	!insertmacro MUI_STARTMENU_GETFOLDER "" $StartMenuFolder

	${if} "$StartMenuFolder" == "${MUI_STARTMENUPAGE_DEFAULTFOLDER}"
		!insertmacro MULTIUSER_GetCurrentUserString $0
		StrCpy $StartMenuFolder "$StartMenuFolder$0"
	${endif}
FunctionEnd

Function PageDirectoryPre
	GetDlgItem $1 $HWNDPARENT 1
	${if} ${SectionIsSelected} ${SectionProgramGroup}
		SendMessage $1 ${WM_SETTEXT} 0 "STR:$(^NextBtn)" ; this is not the last page before installing
		SendMessage $1 ${BCM_SETSHIELD} 0 0 ; hide SHIELD (Windows Vista and above)
	${else}
		SendMessage $1 ${WM_SETTEXT} 0 "STR:$(^InstallBtn)" ; this is the last page before installing
		Call MultiUser.CheckPageElevationRequired
		${if} $0 = 2
			SendMessage $1 ${BCM_SETSHIELD} 0 1 ; display SHIELD (Windows Vista and above)
		${endif}
	${endif}
FunctionEnd

Function PageDirectoryShow
	${if} $CmdLineDir != ""
		FindWindow $R1 "#32770" "" $HWNDPARENT

		GetDlgItem $0 $R1 1019 ; Directory edit
		SendMessage $0 ${EM_SETREADONLY} 1 0 ; read-only is better than disabled, as user can copy contents

		GetDlgItem $0 $R1 1001 ; Browse button
		EnableWindow $0 0
	${endif}
FunctionEnd

Function PageStartMenuPre
	${ifnot} ${SectionIsSelected} ${SectionProgramGroup}
		Abort ; don't display this dialog if SectionProgramGroup is not selected
	${else}
		GetDlgItem $1 $HWNDPARENT 1
		Call MultiUser.CheckPageElevationRequired
		${if} $0 = 2
			SendMessage $1 ${BCM_SETSHIELD} 0 1 ; display SHIELD (Windows Vista and above)
		${endif}
	${endif}
FunctionEnd

Function PageInstFilesLeave
  	WriteRegStr "${MUI_LANGDLL_REGISTRY_ROOT}" "${MUI_LANGDLL_REGISTRY_KEY}" "PathModified" $PathModified
FunctionEnd

Function .onInstFailed
	MessageBox MB_ICONSTOP "${PRODUCT_NAME} ${VERSION} could not be fully installed.$\r$\nPlease, restart Windows and run the setup program again." /SD IDOK
FunctionEnd

; remove next line if you're using signing after the uninstaller is extracted from the initially compiled setup
!include "..\nsis\Uninstall.nsh"
