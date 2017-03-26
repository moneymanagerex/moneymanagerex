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
	git checkout cmake
	git submodule update --init
	
## Compile 
	cmake -D ARG_VER_MAJOR=1 -D ARG_VER_MINOR=3 -D ARG_VER_PATCH=3 .
    make package
