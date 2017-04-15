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
    If you want build the project with debug create debug dir in build directory
    and replace cmake flag
    -DCMAKE_BUILD_TYPE=Release
    with
     -DCMAKE_BUILD_TYPE=Debug

    cd build
    mkdir release
    cd release
    cmake -DCMAKE_BUILD_TYPE=Release ../../
	make package
	
## Install
Now you can install the produced packaged via command:

    sudo rpm -i <name-of-created-package-from-previous-step>.rpm
