# Building MMEX on Windows

## Install Prerequisites
# Set system environment wxwin: c:\projects\wxwidgets

# Latter GETTEXT library can be obtained normaly via official
# https://www.gnu.org/software/gettext/
# where is mentioned that for windows it can be downloaded from
# https://mlocati.github.io/articles/gettext-iconv-windows.html

# But for appveyor we need to install cygwin that contains
# embeded GETTEXT library
#path: C:\cygwin64\bin;C:\cygwin\bin;%path%

	git clone https://github.com/wxWidgets/wxWidgets.git %WXWIN%
	cd %WXWIN%
	git checkout tags/v3.1.0
	cd build/msw

	Choose your architecture and via visual studio developer console run command

	#nmake /f makefile.vc TARGET_CPU=Win32
	nmake /f makefile.vc TARGET_CPU=x64

## Download Source
	git clone https://github.com/moneymanagerex/moneymanagerex
	cd moneymanagerex
	git checkout v1.3.x
	git submodule update --init
	
## Compile
	cmake -G "NMake Makefiles"

	Choose your architecture and via visual studio developer console run command

	#nmake /f Makefile TARGET_CPU=Win32
	nmake /f Makefile TARGET_CPU=x64
