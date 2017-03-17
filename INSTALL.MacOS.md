# Building MMEX on MacOS

## Install Prerequisites

### MacOS Sierra 
Install required packages. You can choose which compiler you want to use:

**CLang - optional**

	brew update && brew install wxmac gettext
	brew link --force gettext

## Download Source
	git clone https://github.com/moneymanagerex/moneymanagerex
	cd moneymanagerex
	git checkout cmake_v1.3.x
	git submodule update --init
	
## Compile 
	cmake . && make package
