#!/bin/bash
set -e
mkdir -pv $1
cd $1
mkdir -pv backend
wget "http://apertium.projectjj.com/${WINX}/${BUILDTYPE}/hfst-ospell-latest.7z" -O hfst-ospell-latest.7z
7z x hfst-ospell-latest.7z hfst-ospell/bin/
rsync -avu hfst-ospell/bin/* backend/
wget "http://apertium.projectjj.com/${WINX}/${BUILDTYPE}/tdc-proof-latest.7z" -O tdc-proof-latest.7z
7z x tdc-proof-latest.7z tdc-proof/bin/
rsync -avu tdc-proof/bin/* backend/
wget "http://apertium.projectjj.com/win32/${BUILDTYPE}/data.php?deb=$2" -O data.deb
7z x data.deb
set +e
tar -xf data.tar ./usr/share/voikko/3/ ./usr/share/apertium/
cp -av usr/share/voikko/3/*.zhfst usr/share/apertium/*/*.zhfst backend/
