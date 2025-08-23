set /p Version=<..\version.txt
set product=%~1
set windir=%~2
if "%Platform%" EQU "x86" (
set Staging=%windir%\install32
if exist %product%-%VERSION%-32.msi del /f %product%-%VERSION%-32.msi
wix build -pdbtype none -ext WixToolset.UI.wixext -bindvariable WixUILicenseRtf=..\msi\License.rtf -arch x86 -out %product%-%VERSION%-32.msi -culture en-US -i %CD% -loc ..\msi\package.wxl ..\msi\package.wxs components.wxs 
) else (
set Staging=%windir%\install64
if exist %product%-%VERSION%-64.msi del /f %product%-%VERSION%-64.msi
wix build -pdbtype none -ext WixToolset.UI.wixext -bindvariable WixUILicenseRtf=..\msi\License.rtf -arch x64 -out %product%-%VERSION%-64.msi -culture en-US -i %CD% -loc ..\msi\package.wxl ..\msi\package.wxs components.wxs 
)
