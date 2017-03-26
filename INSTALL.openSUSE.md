# Building MMEX on openSUSE

## Install Prerequisites

### openSUSE 44.2
Install required packages. You can choose which compiler you want to use:

**GCC - default**

	sudo zypper update -y && sudo zypper install -y \
	cmake gettext-tools gcc-c++ git make wxWidgets-3_0-devel
	
**CLang - optional**

    sudo zypper update -y && sudo zypper install -y \
    cmake gettext-tools git llvm-clang make wxWidgets-3_0-devel
    
If you installed CLang furthermore you need to change your default compiler via: 

    sudo ln -sf /usr/bin/clang /usr/bin/cc
    sudo ln -sf /usr/bin/clang++ /usr/bin/c++
		
## Download Source
	git clone https://github.com/moneymanagerex/moneymanagerex
	cd moneymanagerex
	git checkout cmake
	git submodule update --init
	
## Compile
	cmake -D ARG_VER_MAJOR=1 -D ARG_VER_MINOR=3 -D ARG_VER_PATCH=3 .
    make package
	
## Install
Now you can install the produced packaged via command:

    sudo rpm -i <name-of-created-package-from-previous-step>.rpm
