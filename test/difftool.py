#!/usr/bin/python3
# -*- coding: utf-8 -*-
# difftool.py
# A diff utility as a helper program to replace the UNIX diff under Windows
# To be compatible with the behaviour of UNIX diff this program was needed because
# the Windows tools are not silent if the files are identical.
#
# The program compares text and binary files (--binary option needed).
#
# (C) Joachim Siebold 2024
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#

import argparse
import difflib
import filecmp
import sys
from pathlib import Path

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("old_file")
    parser.add_argument("new_file")
    parser.add_argument("--binary",action="store_true")
    args = parser.parse_args()

    if(args.binary):
       ret=filecmp.cmp(args.old_file,args.new_file)
       if not ret:
          print("Files $old_file $new_file differ");
          sys.exit(1)
       else:
          sys.exit(0)
    else:
       old_file = Path(args.old_file)
       new_file = Path(args.new_file)

       fd_old_file=open(old_file,encoding="utf-8")
       fd_new_file=open(new_file,encoding="utf-8")
       try:
          file_1 = fd_old_file.readlines()
       except UnicodeDecodeError:
          print(args.old_file," old file charmap decode error")
          sys.exit(1)
       try:
          file_2 = fd_new_file.readlines()
       except UnicodeDecodeError:
          print(args.new_file, "old file charmap decode error")
          sys.exit(1)

       delta = difflib.unified_diff(file_1, file_2, old_file.name, new_file.name)
       fd_old_file.close()
       fd_new_file.close()
       hasDiffs= False
       for s in delta:
          hasDiffs= True 
          sys.stdout.write(s)
       if hasDiffs:
          sys.exit(1)
       else:
          sys.exit(0)


if __name__ == "__main__":
    main()

