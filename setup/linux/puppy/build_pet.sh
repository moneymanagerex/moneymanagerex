#!/bin/bash
# This script will build .pet package
#
#

# The corresponding version of mmex must be specified below
MMEX_VERSION="0.9.9.0"

PET_UTILS=`pwd`
cd ../../../
./configure --prefix=$HOME/build/mmex-$MMEX_VERSION/usr
make install

mkdir -p ~/build/mmex-$MMEX_VERSION/usr/share/applications
cp resources/mmex.desktop ~/build/mmex-$MMEX_VERSION/usr/share/applications/

mkdir -p ~/build/mmex-$MMEX_VERSION/usr/share/icons/hicolor/scalable/apps
cp graphics/mmex.svg ~/build/mmex-$MMEX_VERSION/usr/share/icons/hicolor/scalable/apps/
mkdir -p ~/build/mmex-$MMEX_VERSION/usr/local/lib/X11/mini-icons
cp resources/mmex.xpm ~/build/mmex-$MMEX_VERSION/usr/local/lib/X11/mini-icons/

mkdir -p ~/build/mmex-$MMEX_VERSION/usr/share/man/man1
cp doc/mmex.1.gz ~/build/mmex-$MMEX_VERSION/usr/share/man/man1/

rm ~/build/mmex-$MMEX_VERSION/usr/share/mmex/po/*.po

strip ~/build/mmex-$MMEX_VERSION/usr/bin/mmex

#cd setup/linux/puppy/
cd ~/build/
fakeroot $PET_UTILS/dir2pet mmex-$MMEX_VERSION $PET_UTILS


