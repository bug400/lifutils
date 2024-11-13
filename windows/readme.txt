Build the Windows version of the LIFUTILS with MSVC 2019 or MSVC 2022

Build requirements:

Visual Studio 2019 or 2022 with cmake build tools, nsis 3.0.9. 
The nsis installation must be manually enhanced with the strlen 8192 special build,  then EnvVar plugin, 
the multi user extension (https://github.com/Drizin/NsisMultiUser - but without the dlls in the Plugins 
directory), StdUtils v1.14 (https://github.com/lordmulder/stdutils/releases) and the UAC plugin 
(https://nsis.sourceforge.io/UAC_plug-in)


Build:
run build_lifutils.cmd compiles the LIFUTILS and creates a windows installer for the current build tool 
environment. You may choose either the x86 native tools command prompt to create a 32 bit installer 
or the x64 native tools command prompt to create a 64bit installer.

If build_lifutils.cmd is invoked with the parameter "TEST" then the test suite is run. This requires a Python 3 interpreter installed on the system.

Note: you must set the environment variable NSISDIR which points to the root directory of the install
system.
