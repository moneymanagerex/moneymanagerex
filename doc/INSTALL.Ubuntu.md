# Building MMEX on Ubuntu

## Install Prerequisites

### Ubuntu 14.04
	sudo apt-key adv --fetch-keys http://repos.codelite.org/CodeLite.asc
	sudo apt-add-repository 'deb http://repos.codelite.org/wx3.1.0/ubuntu/ trusty universe' -y
	sudo apt-get update && sudo apt-get install -y \
	build-essential cmake git libwxbase3.1-dev libwxgtk3.1-dev libwxgtk-webview3.1-dev
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
	git submodule update --init
	
## Compile
If you want build the project with debug create debug dir in build directory
and replace cmake flag
`-DCMAKE_BUILD_TYPE=Release`
with
`-DCMAKE_BUILD_TYPE=Debug`

    cd build
    mkdir release
    cd release
    cmake -DCMAKE_BUILD_TYPE=Release ../../
	make package
	
## Install
Now you can install the produced packaged via command:

    sudo dpkg -i <name-of-created-package-from-previous-step>.deb
