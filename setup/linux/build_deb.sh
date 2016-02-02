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

. variables.sh

# Specify the build version of mmex
RELEASE_DIR="$HOME/release"

PACKAGE_NAME="mmex-$MMEX_VERSION-$ARCHITECTURE"

#Build the source
cd ../..
MMEX_DIR=`pwd`

rm -rf "$RELEASE_DIR/$PACKAGE_NAME"
cmake -DCMAKE_INSTALL_PREFIX=/usr

if [ $? -gt 0 ]; then
    echo "ERROR!"
    exit 1
fi
make DESTDIR="$RELEASE_DIR/$PACKAGE_NAME" install
if [ $? -gt 0 ]; then
    echo "ERROR!"
    exit 1
fi

cd "$RELEASE_DIR/$PACKAGE_NAME"

#Strip the binary before calculating the installed size
strip --remove-section=.comment "usr/bin/mmex"

#Make sure any needed files are in place and formatted correctly
#Changelog
mv "usr/share/doc/mmex/version.txt" "usr/share/doc/mmex/changelog"
gzip -9 -f -n "usr/share/doc/mmex/changelog"

#Copyright
cp "usr/share/doc/mmex/contrib.txt" "usr/share/doc/mmex/copyright"
sed -i "s/See the GNU General Public License for more details./A copy of the GPLv2 can be found in \"\/usr\/share\/common-licenses\/GPL-2\"/g" "usr/share/doc/mmex/copyright"
sed -i 's/\r//g' "usr/share/doc/mmex/copyright"

#Calculate installed size
INSTALLED_SIZE=$(du -sb $RELEASE_DIR/ | cut -f1)
INSTALLED_SIZE=`expr $INSTALLED_SIZE / 1024`

mkdir -p "DEBIAN"

#Create the control file
echo "Package: mmex
Version: $MMEX_VERSION
Section: misc
Priority: extra
Architecture: $ARCHITECTURE
Homepage: $MMEX_HOMEPAGE
Depends: $MMEX_DEB_DEPENDS
Installed-Size: $INSTALLED_SIZE
Maintainer: MoneyManagerEx <$MMEX_EMAIL>
Description: $MMEX_DESCRIPTION" > "DEBIAN/control"

#Generate md5sums
md5sum `find . -type f | grep -v '^[.]/DEBIAN/'` > DEBIAN/md5sums
chmod 0644 DEBIAN/md5sums

#Build the package
cd $RELEASE_DIR
fakeroot dpkg-deb -b $PACKAGE_NAME

#Check for any packaging problems
lintian -EviIL +pedantic $PACKAGE_NAME.deb

