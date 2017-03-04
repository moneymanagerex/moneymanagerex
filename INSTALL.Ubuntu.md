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
	sudo apt-get update && sudo apt-get install -y build-essential cmake git libgtk-3-dev libwebkitgtk-dev libwxgtk3.0-dev libwxgtk-webview3.0-dev
		
## Download Source
	git clone https://github.com/moneymanagerex/moneymanagerex
	cd moneymanagerex
	git checkout cmake
	git submodule update --init
	
## Compile & Install
	cmake . && make package
