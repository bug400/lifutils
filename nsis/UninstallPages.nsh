; Installer Attributes
ShowUninstDetails show 

; Interface settings
!define MUI_UNABORTWARNING ; Show a confirmation when cancelling the installation

; Pages
!define MULTIUSER_INSTALLMODE_CHANGE_MODE_FUNCTION un.PageInstallModeChangeMode
!insertmacro MULTIUSER_UNPAGE_INSTALLMODE
!insertmacro MUI_UNPAGE_INSTFILES
