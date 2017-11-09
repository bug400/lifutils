if "%Platform%" EQU "X86" (
   "C:\Program Files (x86)\nsis\makensis.exe" -NOCD -DINPDIR="install32" -DVERSTR="1.7.7b1" ..\nsis\lifutils.nsi
) else (
   "C:\Program Files (x86)\nsis\makensis.exe" -NOCD -DINPDIR64="install64" -DVERSTR="1.7.7b1" ..\nsis\lifutils.nsi
)
