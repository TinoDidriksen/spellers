#!/bin/bash
set -e
mkdir -pv $1
cd $1
mkdir -pv backend
wget 'http://apertium.projectjj.com/win32/nightly/hfst-ospell-latest.7z' -O hfst-ospell-latest.7z
7z x hfst-ospell-latest.7z hfst-ospell/bin/
rsync -avu hfst-ospell/bin/* backend/
wget 'http://apertium.projectjj.com/win32/nightly/tdc-proof-latest.7z' -O tdc-proof-latest.7z
7z x tdc-proof-latest.7z tdc-proof/bin/
rsync -avu tdc-proof/bin/* backend/
wget 'http://apertium.projectjj.com/win32/nightly/data.php?deb=giella-kal' -O data.deb
7z x data.deb
tar -xf data.tar ./usr/share/voikko/3/
mv -v usr/share/voikko/3/* backend/
