LIFUTILS Installation instructions
==================================

Index
-----

* [Installation of the precompiled binaries](#installation-of-the-precompiled-binaries)
* [Build from scratch](#build-from-scratch)


Installation of the precompiled binaries
----------------------------------------

LINUX:

There are DEBIAN packages available for download (32-bit and 64-bit).
Install the package as root.

     dpkg -i <package-file-name>


Windows:

There are installer exe file to safely install the LIFUTILS. Download 
the appropriate 32 or 64 bit version of the installer file follow the
instructions below.

* Uninstall any previous installed version of the lifutils before installing
a new version.
* A 32 bit version of the lifutils will not install on a 64 bit windows
* you may choose a system-wide installation or an installation only for
the current user. A system-wide installation requires administrator privileges.
* You may choose to modify the PATH variable and set the
LIFUTILSXROMDIR environment variable. This is not needed if you call the
lifutils from the "lifutils prompt" in the start menu.
* It is recommended to uninstall ASM71 from the legacy "programs and features" control panel. Uninstalling from the "apps and features " control panel (Windows 10) requires to enter administrator credentials even for a local installation.



Mac OS:

There is a package that installs the package under /usr/local. There is no
need to set the LIFUTILSXROMDIR environment variable as long as the
install location is not changed.

Note: you get a message that the package cannot be opened because it is from
an unidentified developer. Click on the "?" button in the message window to
learn how to proceed.

To uninstall the LIFUTILS run the script create_lifutils_removescript.sh to 
create an uninstall script:

     bash /usr/local/share/lifutils/create_lifutils_removescript.sh > uninstall.sh

The run the script uninstall.sh as administrator:

     sudo bash ./uninstall.sh

Due to security reasons each delete operation requires confirmation.



Build from scratch
------------------

The LIFUTILS were migrated to the CMake build system. 

Download the lifutils source package and follow the instruction in the
build directories:

* linux: Linux (Debian package tools)
* macos: mac OS (XCode command line tools)
* mingw (mingw cross compile under Linux and nsis 3.0 package builder)
* windows (MSVC 2017 or MSVC 2019 and nsis 3.0 package builder)
