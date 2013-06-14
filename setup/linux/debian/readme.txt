#How to prepare system to build mmex

sudo apt-get install subversion

mkdir ~/Development
cd ~/Development
svn checkout http://svn.code.sf.net/p/moneymanagerex/code/trunk/mmex mmex

#Install wx libs
sudo apt-get install libwxgtk2.9-dev libwxgtk2.9

#Installing Bakefile On Ubuntu 11.04
sudo apt-get install python-dev

#Download archive from link below then unpack it and build
http://sourceforge.net/projects/bakefile/files/bakefile/0.2.9/bakefile-0.2.9.tar.gz/download
./configure && make 
sudo make install

sudo apt-get install automake

# How to build (and install) mmex.deb package
#
# Note: Build support for 32 bit and 64 bit.
#
# Difference in Control file: Line 13 (In file: build.sh)
#       32bit:= Architecture: i386
#       64bit:= Architecture: amd64
#
# To build the amd64 (64 bit version), set variable in build.sh file to amd64
#

# Specify system Architecture  ("i386" or "amd64")
ARCHITECTURE="i386"

# The corresponding version of mmex must be specified:
MMEX_VERSION="0.9.9.0"

#Then start ./build.sh file

#Check ~/build/mmex-xXX.deb file
lintian mmex-xXX.deb
# check errors against other lintian deb 

# install the package (Have you backed up your databases?)
#dpkg -i mmex.deb

#Then run it
#mmex&
