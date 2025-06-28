LIFUTILS (Utilities to handle LIF files and media images)
=========================================================

Index
-----

* [Description](#description)
* [Compatibility](#compatibility)
* [Installation](#installation)
* [Issues](#issues)
* [License](#license)
* [Acknowledgments](#acknowledgments)


Description
-----------

LIFUTILS are command line utilities to handle various LIF
(Hewlett Packard Logical Interchange Format) files and media images.

Main features are:
* Import or export files from a LIF image file to the local file system
* Pack, label, initialize a LIF image file
* Purge, rename files from a LIF image file
* List directory if a LIF image file
* Convert ASCII files to LIF text files and vice versa
* Compile/decompile HP-41 program files
* add LIF headers to certain raw file types (compiled HP-41 files, HP-41 ROM files)
* output HP-41 SDATA-, KEY-, status- WALL-files as text


See the doc/readme.html file for more information about features and usage.

Note: there is experimental support for the access of floppy devices to
read and write HP9114 floppy disks under LINUX. Floppy disk support will 
n o t work with USB floppy devices.


Compatibility
-------------

The LIFUTILS were tested on LINUX, Windows 10 and mac OS.


INSTALLATION
------------

The [release](https://github.com/bug400/lifutils/releases) section provides precompiled binaries for LINUX (32 and 64bit), Windows (32 bit and 64 bit) and mac OS.

See the [Installation Instructions](https://github.com/bug400/lifutils/blob/master/INSTALL.md) how to install.

Issues
------

Known issues of version 2.0 are:
* _inp41_ and _outp41_ calculate a wrong checksum. See [this post](https://www.hpmuseum.org/forum/thread-23742.html) for details and many thanks to the author.


License
-------

The LIFUTILS are published under the GNU General Public License v2.0 License 
(see LICENSE file).


Acknowledgments
----------------

The LIF Utilities were originally developed by Tony Duell and enhanced by 
Joachim Siebold. Code was taken from hp41uc (Leo Duran), modfile (Warren Furlow)  and liftool (Christophe Gottheimer).

