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

. ../common/variables.sh

ARCHITECTURE="amd64"

PACKAGE_NAME="mmex-$MMEX_VERSION-$ARCHITECTURE"

BUILD_DIR="$HOME/build"

#Build the source
cd ../../..
MMEX_DIR=`pwd`

./bootstrap
if [ $? -gt 0 ]; then
    echo "ERROR!"
    exit 1
fi

./configure --prefix="$BUILD_DIR/$PACKAGE_NAME/usr"

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
strip "$BUILD_DIR/$PACKAGE_NAME/usr/bin/mmex"

#Make sure any needed files are in place and formatted correctly
#Changelog
mv "$BUILD_DIR/$PACKAGE_NAME/usr/share/doc/mmex/version.txt" "$BUILD_DIR/$PACKAGE_NAME/usr/share/doc/mmex/changelog"
gzip -9 -f "$BUILD_DIR/$PACKAGE_NAME/usr/share/doc/mmex/changelog"

#Copyright
cp "$BUILD_DIR/$PACKAGE_NAME/usr/share/doc/mmex/contrib.txt" "$BUILD_DIR/$PACKAGE_NAME/usr/share/doc/mmex/copyright"
sed -i "s/See the GNU General Public License for more details./A copy of the GPLv2 can be found in \"\/usr\/share\/common-licenses\/GPL-2\"/g" "$BUILD_DIR/$PACKAGE_NAME/usr/share/doc/mmex/copyright"

#Copy the manpage in to place and modify
cp "$MMEX_DIR/setup/linux/common/mmex.1" "$BUILD_DIR/$PACKAGE_NAME/usr/share/man/man1/mmex.1"
sed -i "s/MMEX_RELEASE_DATE/$MMEX_RELEASE_DATE/g" "$BUILD_DIR/$PACKAGE_NAME/usr/share/man/man1/mmex.1"
sed -i "s/MMEX_VERSION/$MMEX_VERSION/g" "$BUILD_DIR/$PACKAGE_NAME/usr/share/man/man1/mmex.1"
gzip -9 -f "$BUILD_DIR/$PACKAGE_NAME/usr/share/man/man1/mmex.1"
chmod 644 "$BUILD_DIR/$PACKAGE_NAME/usr/share/man/man1/mmex.1.gz"

#Calculate installed size
INSTALLED_SIZE=$(du -sb $BUILD_DIR/ | cut -f1)
INSTALLED_SIZE=`expr $INSTALLED_SIZE / 1024`

mkdir -p "$BUILD_DIR/$PACKAGE_NAME/DEBIAN"

#Create the control file
echo "Package: mmex
Version: $MMEX_VERSION
Section: misc
Priority: extra
Architecture: $ARCHITECTURE
Homepage: $MMEX_HOMEPAGE
Depends: $MMEX_DEPENDS
Installed-Size: $INSTALLED_SIZE
Maintainer: MoneyManagerEx <$MMEX_EMAIL>
Description: $MMEX_DESCRIPTION" > $BUILD_DIR/$PACKAGE_NAME/DEBIAN/control

#Build the package
cd $BUILD_DIR
fakeroot dpkg-deb -b $PACKAGE_NAME

#Check for any packaging problems
lintian $PACKAGE_NAME.deb

