@ECHO OFF
REM
REM Ctest test driver for LIFUTILS
REM It uses the find command supplied by the vi editor
REM
REM Parameters:
REM 1: name of test file
REM 2: path to executables
REM 3: path to scripts directory
REM 4: path to xroms directory
REM
set LIFUTILSREGRESSIONTEST=1
set PATH=%~2;%~3;%PATH%
set LIFUTILSXROMDIR=%~4
set testfile=%~1
pushd %testfile%
if exist %testfile%.out del /F %testfile%.out
call %testfile%.cmd > %testfile%.out
python ..\difftool.py %testfile%.out reference\%testfile%.out
set ret=%errorlevel%
if exist %testfile%.out del /F %testfile%.out
popd
exit /B %ret%
