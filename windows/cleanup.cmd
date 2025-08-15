if exist ..\build RD /S /Q ..\build
if "%Platform%" EQU "x86" (
   if exist .\install32 RD /S /Q .\install32
) else (
   if exist .\install64 RD /S /Q .\install64
)