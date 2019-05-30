Build the Windows version of the LIFUTILS with MSVC 2017 or MSVC 2019

Build requirements:

cmake, Visual Studio 2017 or 2019 build tools, nsis 3.0. The nsis
installation must be manually enhanced with the strlen 8192 special build and 
the multi user extension (https://github.com/Drizin/NsisMultiUser).

Build:
run build_lifutils.cmd compiles the LIFUTILS and creates a windows installer for
the current build tool environment. You may choose either the x86 native tools command prompt to create a 32 bit installer or the x64 native tools command prompt to create a 64bit installer.
