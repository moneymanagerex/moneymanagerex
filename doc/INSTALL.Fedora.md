# Building MMEX on Fedora

## Install Prerequisites

### Fedora 25
Install required packages. You can choose which compiler you want to use:

**GCC - default**

	sudo yum -y update && sudo yum -y install \
	cmake gcc-c++ gettext git gtk3-devel make rpm-build webkitgtk3-devel wxGTK3-devel

**CLang - optional**

    sudo yum -y update && sum yum -y install \
    cmake clang gettext git gtk3-devel make rpm-build webkitgtk3-devel wxGTK3-devel

If you installed CLang furthermore you need to change your default compiler via: 

    sudo ln -sf /usr/bin/clang /usr/bin/cc
    sudo ln -sf /usr/bin/clang++ /usr/bin/c++

## Download Source
	git clone --recursive https://github.com/moneymanagerex/moneymanagerex
	
## Compile
If you want build the project with debug create debug dir in build directory
and replace cmake flag
`-DCMAKE_BUILD_TYPE=Release`
with
`-DCMAKE_BUILD_TYPE=Debug`

    cd moneymanagerex/build
    mkdir release
    cd release
    cmake -DCMAKE_BUILD_TYPE=Release ../../
	make package
	
## Install
Now you can install the produced packaged via command:

    sudo rpm -i <name-of-created-package-from-previous-step>.rpm
