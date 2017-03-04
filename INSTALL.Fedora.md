# Building MMEX on Fedora

## Install Prerequisites

### Fedora 25
	yum -y update && yum -y install cmake gcc-c++ gettext git gtk3-devel make rpm-build webkitgtk3-devel wxGTK3-devel

## Download Source
	git clone https://github.com/moneymanagerex/moneymanagerex
	cd moneymanagerex
	git checkout cmake
	git submodule update --init
	
## Compile & Install
	cmake . && make package
