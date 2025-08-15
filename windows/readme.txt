Build the Windows version of the LIFUTILS with MSVC 2022

Build requirements:
Visual Studio 2022 with cmake build tools, wix.
To instal wix: install .NET. Devel without any options in VS, call wix install from the VS developer command prompt
  dotnet tool install --global wix
  wix extension add -g WixToolset.UI.wixext

Build:
run build_lifutils.cmd compiles the LIFUTILS and creates a windows installer for the current build tool 
environment. You may choose either the x86 native tools command prompt to create a 32 bit installer 
or the x64 native tools command prompt to create a 64bit installer.

If build_lifutils.cmd is invoked with the parameter "TEST" then the test suite is run. This requires a Python 3 interpreter installed on the system.
