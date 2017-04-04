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

**CLang - optional**

	brew update && brew install wxmac gettext cmake
	brew link --force gettext

## Download Source
	git clone https://github.com/moneymanagerex/moneymanagerex
	cd moneymanagerex
	git checkout cmake
	git submodule update --init
	
## Compile 
	cmake -D ARG_VER_MAJOR=1 -D ARG_VER_MINOR=3 -D ARG_VER_PATCH=3 .
    make package
