# Building MMEX on Ubuntu

## MMEX version 1.3.4

1. Install Prerequisites

   If you prefer CLang over default GCC compiler please replace all
   `build-essential` with `clang make` and run following commands after
   installing packages:

       sudo update-alternatives --set cc /usr/bin/clang
       sudo update-alternatives --set c++ /usr/bin/clang++

   **Ubuntu 14.04 Trusty**

       sudo apt-key adv --fetch-keys http://repos.codelite.org/CodeLite.asc
       sudo apt-add-repository 'deb http://repos.codelite.org/wx3.1.0/ubuntu/ trusty universe'
       sudo apt-get update
       sudo apt-get install git build-essential cmake3 gettext libwxgtk-webview3.1-dev wx3.1-i18n

   **Ubuntu 16.04 Xenial**

       sudo apt update
       sudo apt install git build-essential cmake gettext libwxgtk-webview3.0-dev wx3.0-i18n

   **Ubuntu 18.04 Bionic**

       sudo apt update
       sudo apt install git build-essential cmake gettext libwxgtk-webview3.0-gtk3-dev wx3.0-i18n

2. Download Source

       git clone -b v1.3.x --recurse-submodules https://github.com/moneymanagerex/moneymanagerex
       cd moneymanagerex

3. Compile

       mkdir -p compile
       cd compile
       cmake .. && & cmake --build . --target package

4. Install

       sudo dpkg -i ./<name-of-created-package-from-previous-step>.deb

## MMEX version 1.3.3

1. Install System Prerequisites

   **Ubuntu 16.04 Xenial**

       sudo apt update
       sudo apt install git build-essential gettext automake libwxgtk-webview3.0-dev python-dev wx3.0-i18n

   **Ubuntu 18.04 Bionic**

       sudo apt update
       sudo apt install git build-essential gettext automake libwxgtk-webview3.0-gtk3-dev python-dev wx3.0-i18n

2. Install Bakefile from Sources

       mkdir -p ~/Development
       cd ~/Development
       wget https://github.com/vslavik/bakefile/releases/download/v0.2.9/bakefile-0.2.9.tar.gz
       tar xf bakefile-0.2.9.tar.gz
       cd bakefile-0.2.9
       ./configure && make
       sudo make install

3. Download Source

       cd ~/Development
       git clone -b v1.3.3 --recurse-submodules https://github.com/moneymanagerex/moneymanagerex
       cd moneymanagerex

4. Compile

       cd build/bakefiles
       bakefile_gen
       cd ../../auxd
       bakefilize -c -v
       cd ..
       aclocal -I m4 -I /share/aclocal
       autoconf
       mkdir -p compile
       cd compile
       ../configure && make

5. Install

       sudo make install
