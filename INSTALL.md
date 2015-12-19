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

There is an installer exe file to safely install the LIFUTILS. Download and
execute the installer file and follow the instructions. You need admin privileges.
The software can be safely uninstalled from the control panel. At the moment the
installer only provides a 32-bit version of the software.

Since the LIFUTILS are command line utilities you invoke then from a console window.
The PATH variable is modified by the installation program.


Mac OS X:

There is a zip file that contains the subdirectories "bin" and "usr". For a
system wide installation it is recommended to install these files under
/usr/local.

Obtain administrative privileges and:

     cd /usr/local
     unzip <name-of-zip-file>

There is no installation package available at the moment.


Installation from scratch
-------------------------

LINUX:

* Install the autotools, automake tool chain
* Install c development tool chain
* Download and unpack the LIFUTILS-sources

type:

     ./autogen.sh
     ./configure --prefix=(destination directory)
     ./make
     ./make install

The destination directory is the location of the bin/ and share/ directory
where the files will be installed (usually /usr/local or /usr).
Note: to install files in /usr or /usr/local you need root rights.

To invoke the lifutil tools the bin directory of LIFUTILS must be contained in the
PATH environment variable.


Windows:

LIFUTILS was cross compiled under LINUX. Building LIFUTILS on Windows should
be possible with either the Cygwin or MSYS development system.


Mac OS X:

It is recommended that you use the binaries supplied by the MacPorts Project:
https://www.macports.org. First install the Xcode Developer Tools. Open a
terminal window (as superuser) and type:

     xcode-select --install

After installation download and install the OS X Package install for your
operating system.

The software installed by the port utility requires the following environemnt variables:

     export PATH=/opt/local/bin:/opt/local/sbin:$PATH
     export MANPATH=/opt/local/share/man:$MANPATH

You should apply the setting of the environment variables to your .profile or
.bash_profile file.

To compile the LIFUTILS you must install the autotool, autoconf and automake
packages.

Compile and install the software according to the LINUX description (see above).
