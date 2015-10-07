#!/bin/bash
set -e
mkdir -pv $1
cd $1
mkdir -pv backend
wget 'http://apertium.projectjj.com/win32/nightly/hfst-ospell-latest.7z' -O hfst-ospell-latest.7z
7z x hfst-ospell-latest.7z hfst-ospell/bin/
mv -v hfst-ospell/bin/* backend/
wget 'http://apertium.projectjj.com/win32/nightly/data.php?deb=giella-sme' -O data.deb
7z x data.deb
tar -xf data.tar ./usr/lib/x86_64-linux-gnu/voikko/3/
mv -v usr/lib/x86_64-linux-gnu/voikko/3/* backend/
