#!/bin/bash
pkgutil --only-files --files org.bug400.lifutils | sed 's!^!rm -i /!' 
echo "rm -d -i /usr/local/share/doc/lifutils/html"
echo "rm -d -i /usr/local/share/doc/lifutils/hardware"
echo "rm -d -i /usr/local/share/doc/lifutils"
echo "rm -d -i /usr/local/share/lifutils/xroms"
echo "rm -d -i /usr/local/share/lifutils"
echo "pkgutil --forget org.bug400.lifutils" 
