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
		
## Download Source

	cd ~/Development
	git clone https://github.com/moneymanagerex/moneymanagerex
	cd moneymanagerex
	git submodule update --init
	
## Compile & Install
	cmake .
	make && sudo make install
