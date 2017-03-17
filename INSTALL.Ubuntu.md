# Building MMEX on Ubuntu

## Install Prerequisites

### Ubuntu 14.04
	sudo apt-add-repository ppa:costamagnagianfranco/locutusofborg-ppa
	sudo apt-get update
	sudo apt-get install git
	sudo apt-get install build-essential automake
	sudo apt-get install libwxbase3.0-dev libwxgtk3.0-dev libwxgtk-webview3.0-dev
	sudo apt-get install python-dev

### Ubuntu 15.04 & 15.10
	sudo apt-get install git
	sudo apt-get install build-essential automake
	sudo apt-get install libwxbase3.0-dev libwxgtk3.0-dev libwxgtk-webview3.0-dev
	sudo apt-get install python-dev

### Ubuntu 16.04
Install required packages. You can choose which compiler you want to use:

**GCC - default**

	sudo apt-get update && sudo apt-get install -y \
	build-essential cmake git libgtk-3-dev libwebkitgtk-dev libwxgtk3.0-dev libwxgtk-webview3.0-dev

**CLang - optional**

    sudo apt-get update && sudo apt-get install -y \
    clang cmake git libgtk-3-dev libwebkitgtk-dev libwxgtk3.0-dev libwxgtk-webview3.0-dev make

If you installed CLang furthermore you need to change your default compiler via: 

    sudo update-alternatives --set cc /usr/bin/clang
    sudo update-alternatives --set c++ /usr/bin/clang++

## Download Source
	git clone https://github.com/moneymanagerex/moneymanagerex
	cd moneymanagerex
	git checkout cmake_v1.3.x
	git submodule update --init
	
## Compile
	cmake . && make package
	
## Install
Now you can install the produced packaged via command:

    sudo dpkg -i <name-of-created-package-from-previous-step>.deb
