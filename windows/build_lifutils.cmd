@echo off
call cleanup
set THIS_DIR=%CD%
MKDIR ..\build
pushd ..\build
if "%Platform%" EQU "x86" (
mkdir %THIS_DIR%\install32
cmake .. -G"NMake Makefiles"  -DCMAKE_INSTALL_PREFIX=%THIS_DIR%\install32
) else (
mkdir %THIS_DIR\install64
cmake .. -G"NMake Makefiles"  -DCMAKE_INSTALL_PREFIX=%THIS_DIR%\install64
)
nmake
if [%1] NEQ [] nmake test
nmake install
popd
call ..\msi\build_msi lifutils %CD%
call cleanup
