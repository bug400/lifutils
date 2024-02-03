Build the Windows version of the LIFUTILS with mingw on Linux

Build requirements:

cmake, c and c++ cross compiler toolchaein (32 and 64 bit), nsis 3.0.9. The nsis
installation must be manually enhanced with the strlen 8192 special build,  then EnvVar plugin, 
the multi user extension (https://github.com/Drizin/NsisMultiUser - but without the dlls in the Plugins 
directory), StdUtils v1.14 (https://github.com/lordmulder/stdutils/releases) and the UAC plugin 
(https://nsis.sourceforge.io/UAC_plug-in)

Build:
Set the environment variable NSISDIR to point to the root directory of the NSIS installer
run build_lifutils.sh compiles the LIFUTILS and creates a windows installer for
the 32 and the 64 bit version.
