set /p Version=<..\version.txt
if "%Platform%" EQU "x86" (
set Staging=%CD%\install32
if exist lifutils-%VERSION%-32.msi del /f lifutils-%VERSION%-32.msi
wix build -pdbtype none -ext WixToolset.UI.wixext -bindvariable WixUILicenseRtf=License.rtf -arch x86 -out lifutils-%VERSION%-32.msi lifutils.wxs WixUI_InstallDirMod.wxs SelectScopeDlg.wxs
) else (
set Staging=%CD%\install64
if exist lifutils-%VERSION%-64.msi del /f lifutils-%VERSION%-64.msi
wix build -pdbtype none -ext WixToolset.UI.wixext -bindvariable WixUILicenseRtf=License.rtf -arch x64 -out lifutils-%VERSION%-64.msi lifutils.wxs WixUI_InstallDirMod.wxs SelectScopeDlg.wxs
)
