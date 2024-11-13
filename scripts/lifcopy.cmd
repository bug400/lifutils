@echo off
rem copy files from a LIF image file to another new LIF image file
rem check number of input parameters
set argC=0
for %%x in (%*) do set /A argC+=1
if not %argC% == 4 (
   echo Usage: lifcopy lif-image-filename1 lif-image-filename2 medium-type dir-entries
   exit /B 1
)
rem check if input file exists
if not exist %1 (
   echo Input LIF image file does not exist
   exit /B 1
)
rem check if output file does not exist
if exist %2 (
   echo Output LIF image file already exists
   exit /B 1
)
rem initialize new image file
lifutils lifinit -m %3 %2 %4
if errorlevel 1 (
   exit /B 1
)
rem copy content
for /F %%f in ('lifutils lifdir -n %1') do (
   lifutils lifget %1 %%f | lifutils lifput %2
   if errorlevel 1 (
      exit /B 1
   )
)
