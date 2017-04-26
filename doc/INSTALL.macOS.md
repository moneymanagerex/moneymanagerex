# Building MMEX on MacOS

## Install Prerequisites
First verify, if you have installed Xcode Command Line Tools. Open terminal and type `git --version` if it shows something like 
```git version 2.11.0 (Apple Git-81)```

you are fine. If not, you will be prompted to *Install* them via your operating system. Alternativelly, you can install those tools via command:
`xcode-select --install`

After that, for comfortable installing softare we use Homebrew https://brew.sh

Run the command:

```/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"```

### MacOS Sierra 
Install required packages. You can choose which compiler you want to use:

**CLang**

	brew update && brew install wxmac gettext cmake
	brew link --force gettext

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
