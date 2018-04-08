## LIFUTILS (Utilities to handle LIF files and media images)

Index
-----

* [Description](#description)
* [Compatibility](#compatibility)
* [Installation](#installation)
* [License](#license)
* [Acknowledgements](#acknowledgements)


Description
===========

LIFUTILS are command line utilities to handle various LIF
(Hewlett Packard Logical Interchange Format) files and media images.

Main features are:
* Import or export files from a LIF image file to the local file system
* Pack, label, initialize a LIF image file
* Purge, rename files from a LIF image file
* List directory if a LIF image file
* Convert ASCII files to LIF text files and vice versa
* Compile/decompile HP-41 program files
* add LIF headers to certain raw file types (compiled HP-41 files, HP-41 rom files)
* output HP-41 SDATA-, KEY-, status- WALL-files as text


See the [doc/readme.html](https://rawgit.com/bug400/lifutils/master/doc/readme.html)
for more information about features and usage.

Note: there is experimental support for the access of floppy devices to
read and write HP9114 floppy disks under LINUX. Floppy disk support will 
n o t work with USB floppy devices.


Compatibility
=============

The LIFUTILS were tested on LINUX, Windows 10 and mac OS.


INSTALLATION
============

The [release](https://github.com/bug400/lifutils/releases) section provides precompiled binaries for LINUX (32 and 64bit), Windows (32 bit and 64 bit) and mac OS.

See the [Installation Instructions](https://github.com/bug400/lifutils/blob/master/INSTALL.md) how to install.


License
=======

The LIFUTILS are published under th GNU General Public License v2.0 License 
(see LICENSE file).


Acknowledgements
================

The LIF Utilities were originally developed by Tony Duell and enhanced by 
Joachim Siebold. Code was taken from hp41uc (Leo Duran), modfile (Warren Furlow)  and liftool (Christophe Gottheimer).

