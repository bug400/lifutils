set /p version=<..\version.txt
if "%Platform%" EQU "x86" (
   "%NSISDIR%\makensis.exe" -NOCD -DINPDIR="install32" -DVERSTR=%version% ..\nsis\Setup.nsi
) else (
   "%NSISDIR%\makensis.exe" -NOCD -DINPDIR64="install64" -DVERSTR=%version% ..\nsis\Setup.nsi
)
