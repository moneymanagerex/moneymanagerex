#!/bin/bash
#
# Note: Build support for 32 bit and 64 bit.
#
# Difference in Control file: Line 13
#       32bit:= Architecture: i386
#       64bit:= Architecture: amd64
#
# To build the amd64 (64 bit version), set variable below to amd64
#

# Specify system Architecture  ("i386" or "amd64")
ARCHITECTURE="amd64"

# Specify the build version of mmex
MMEX_VERSION="1.2.0"

EMAIL="moneymanagerex@moneymanagerex.org"

HOMEPAGE="http://www.moneymanagerex.org"

PACKAGE_NAME="mmex-$MMEX_VERSION-$ARCHITECTURE"

BUILD_DIR="$HOME/build"

cd ../../..

./bootstrap
if [ $? -gt 0 ]; then
    echo "ERROR!"
    exit 1
fi

./configure --prefix=$BUILD_DIR/$PACKAGE_NAME/usr

if [ $? -gt 0 ]; then
    echo "ERROR!"
    exit 1
fi
make && make install
if [ $? -gt 0 ]; then
    echo "ERROR!"
    exit 1
fi

#Strip the binary before calculating the installed size
strip $BUILD_DIR/$PACKAGE_NAME/usr/bin/mmex

mkdir -p $BUILD_DIR/$PACKAGE_NAME/DEBIAN

#Calculate installed size
INSTALLED_SIZE=$(du -sb $BUILD_DIR/ | cut -f1)
INSTALLED_SIZE=`expr $INSTALLED_SIZE / 1024`

echo "Package: mmex
Version: $MMEX_VERSION
Section: misc
Priority: extra
Architecture: $ARCHITECTURE
Homepage: $HOMEPAGE
Depends: libc6 (>= 2.3.5-1), libwxgtk3.0-0 (>= 3.0.0)
Installed-Size: $INSTALLED_SIZE
Maintainer: MoneyManagerEx <$EMAIL>
Description: Simple to use financial management software
 Money  Manager Ex (MMEX) is a free, open-source,
 cross-platform, easy-to-use personal finance software.
 It primarily helps organize one's finances and keeps
 track of where, when and how the money goes.
 MMEX includes all the basic features that 90% of users
 would want to see in a personal finance application.
 The design goals are to concentrate  on  simplicity
 and  user friendliness - something one can use everyday." > $BUILD_DIR/$PACKAGE_NAME/DEBIAN/control

mv $BUILD_DIR/$PACKAGE_NAME/usr/share/doc/mmex/version.txt $BUILD_DIR/$PACKAGE_NAME/usr/share/doc/mmex/changelog
gzip -9 -f $BUILD_DIR/$PACKAGE_NAME/usr/share/doc/mmex/changelog

mv $BUILD_DIR/$PACKAGE_NAME/usr/share/doc/mmex/contrib.txt $BUILD_DIR/$PACKAGE_NAME/usr/share/doc/mmex/copyright

cd $BUILD_DIR
fakeroot dpkg-deb -b $PACKAGE_NAME

lintian $PACKAGE_NAME.deb
# check errors against other lintian deb 

# install the package (Have you backed up your databases?)

#sudo dpkg -i mmex.deb
#mmex&

