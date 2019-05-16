set /p version=<..\version.txt
if "%Platform%" EQU "x86" (
   "C:\Program Files (x86)\nsis\makensis.exe" -NOCD -DINPDIR="install32" -DVERSTR=%version% ..\nsis\Setup.nsi
) else (
   "C:\Program Files (x86)\nsis\makensis.exe" -NOCD -DINPDIR64="install64" -DVERSTR=%version% ..\nsis\Setup.nsi
)
