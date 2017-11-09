call cleanup
set THIS_DIR=%CD%
MKDIR ..\build
pushd ..\build
if "%Platform%" EQU "X86" (
cmake .. -G"NMake Makefiles"  -DCMAKE_INSTALL_PREFIX=%THIS_DIR%\install32
) else (
cmake .. -G"NMake Makefiles"  -DCMAKE_INSTALL_PREFIX=%THIS_DIR%\install64
)
nmake
nmake install
popd
call run_nsis
call cleanup
