LIFUTILS Installation instructions
==================================

Index
-----

* [Installation of the precompiled binaries](#installation-of-the-precompiled-binaries)
* [Build from scratch](#build-from-scratch)


Installation of the precompiled binaries
----------------------------------------

LINUX:

The releases section provides installers (.deb files) for Debian Linux and compatible distributions. See 
the release comments, which Debian version is currently supported. To install the .deb file, issue the following command as root in the directory containing the installer file:

     apt install ./lifutils_X.Y.Z_<architecture>.deb 

**Note:** This command installs the software package from a downloaded file, not from a repository. To obtain a new version of the software, you must download the corresponding deb package and install it using the command above. The previous program version will then be overwritten.

Windows:

There is an MSI installer file to safely install the LIFUTILS on Windows 10/11. There is no 32bit version any more. The installer only allows an installation for the current user and modifies/sets the PATH and LIFUTILSXROMDIR environment variables. The package can be removed in the "Apps and Features" section in the Windows Settings. The changes to the environment variables are then reset.

Since there is no longer a Start Menu entry for lifutils, it is recommended that you create a link to the file


     C:\Users\<your username>\AppData\Local\Programs\lifutils\doc\readme.html

on the desktop in order to access the documentation.

**WARNING**

Version 2.0.1 had to switch to the Microsoft msiexec installer to ensure that lifutils can continue to be installed securely. This means that an automatic upgrade from version 2.0.0 and earlier is not possible. You must therefore manually uninstall all existing installations (both system-wide and local) before version 2.0.1 can be installed.



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
* windows (MSVC 2022 and wix 6 package builder)
