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
ARCHITECTURE="i386"

# Specify the build version of mmex
MMEX_VERSION="1.0.0.0"

EMAIL="vomikan@mail.ru"

cd ../../..
PWD=`pwd`
rsync -rCh $PWD/ ~/build/mmex-$MMEX_VERSION/

cd ~/build
tar czf mmex_$MMEX_VERSION.orig.tar.gz  mmex-$MMEX_VERSION
cd  mmex-$MMEX_VERSION
cp setup/linux/debian/remove_tests.diff patch.diff
patch -p0 < patch.diff
rm patch.diff

./bootstrap

# -s, --single              set package class to single
# -e, --email <address>     use <address> as the maintainer e-mail address
# -c, --copyright <type>    use <type> of license in copyright file
# -n, --native              the program is Debian native, don't generate .orig
dh_make -s -e $EMAIL -c gpl3 -n


cd ~/build/mmex-$MMEX_VERSION

sed -e 's/<insert up to 60 chars description>/Simple to use financial management software/g' debian/control > debian/control.tmp
mv debian/control.tmp debian/control 
sed -e 's/<insert long description, indented with spaces>/Money Manager Ex (MMEX) is a free, open-source,\
 cross-platform, easy-to-use personal finance software.\
 It primarily helps organize one`s finances and keeps\
 track of where, when and how the money goes. \
  MMEX includes all the basic features that 90% of users\
 would want to see in a personal finance application. \
  The design goals are to concentrate on simplicity and\
 user friendliness - something one can use everyday./g' debian/control > debian/control.tmp
mv debian/control.tmp debian/control 
sed -e 's%<insert the upstream URL, if relevant>%https://sourceforge.net/projects/moneymanagerex%g' debian/control > debian/control.tmp
mv debian/control.tmp debian/control 
#
sed -e 's%<url://example.com>%https://sourceforge.net/projects/moneymanagerex%g' debian/copyright > debian/copyright.tmp
mv debian/copyright.tmp debian/copyright

sed -e 's%<put author.s name and email here>%Madhan Kanagavel%g' debian/copyright > debian/copyright.tmp
mv debian/copyright.tmp debian/copyright 
sed -e 's%<Copyright (C) YYYY Firstname Lastname>%Copyright (C) 2005-2009 Madhan Kanagavel%g' debian/copyright > debian/copyright.tmp
mv debian/copyright.tmp debian/copyright 
#Copyright (C) 2009-2010 VaDiM, Nikolay, Wesley Ellis
sed -e 's%<likewise for another author>%Copyright (C) 2009-2010 VaDiM, Nikolay, Wesley Ellis\
    Copyright (C) 2010-2012 Stefano, Nikolay%g' debian/copyright > debian/copyright.tmp
mv debian/copyright.tmp debian/copyright 


echo "Now edit control, changelog files in ~/build/mmex-$MMEX_VERSION/debian directory"
echo "Then goto to ~/build/mmex-$MMEX_VERSION directory and start"
echo " dpkg-buildpackage -rfakeroot"

exit

dpkg-buildpackage -rfakeroot

lintian ../mmex-$MMEX_VERSION.deb
# check errorrs against other lintian deb 

# install the package (Have you backed up your databases?)

#sudo dpkg -i mmex.deb
