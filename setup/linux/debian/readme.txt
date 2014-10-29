# How to build and install mmex

# Install required packages
sudo apt-get install build-essential
sudo apt-get install g++-multilib
sudo apt-get install automake
sudo apt-get install python-dev
sudo apt-get install libgtk-3-dev
sudo apt-get install webkitgtk-3.0
sudo apt-get install lintian
sudo apt-get install subversion

# Download, build and install wxWidgets
mkdir ~/Development
cd ~/Development
svn checkout http://svn.wxwidgets.org/svn/wx/wxWidgets/tags/WX_3_0_0 wxWidgets-3.0.0
cd wxWidgets-3.0.0
./configure --enable-webview --enable-webview-webkit && make
sudo make install
sudo ldconfig

# Download, build and install bakefile
cd ~/Development
wget http://sourceforge.net/projects/bakefile/files/bakefile/0.2.9/bakefile-0.2.9.tar.gz
tar xfz bakefile-0.2.9.tar.gz
cd bakefile-0.2.9
./configure && make 
sudo make install

# Download mmex
cd ~/Development
svn checkout http://svn.code.sf.net/p/moneymanagerex/code/trunk mmex

# Configure mmex
#     Modify 3rd/cgitemplate/html_template.h
#         Add line "#include <cstddef>"
#     Modify "setup/linux/debian/build.sh"
#         Specify version of mmex ("1.2.0.0")
#         Specify system Architecture  ("i386" or "amd64")

# Build mmex
cd mmex/setup/linux/debian
./build.sh

# Install the package (Have you backed up your databases?)
cd ~/build
dpkg -i mmex-xXX.deb

# Then run it
mmex&


