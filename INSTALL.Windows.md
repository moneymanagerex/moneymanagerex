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
	with "Native Tools Command Prompt for VS 2017" or "Developer Command Prompt for VS 2017"

	#nmake /f makefile.vc TARGET_CPU=Win32 BUILD=release
	nmake /f makefile.vc TARGET_CPU=x64 BUILD=release

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
	cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release ../../

	Choose your architecture and via visual studio developer console run command

	#nmake /f Makefile TARGET_CPU=Win32
	nmake /f Makefile TARGET_CPU=x64 BUILD=release

    # If you are interested in producing Visual Studio project file, you can run
    # this command instead of previous one
    #cmake -G "Visual Studio 14 2015" -DCMAKE_BUILD_TYPE=Release ../../
## Build package
    For building installable package, you need to have NSIS installed http://nsis.sourceforge.net/Download

	cpack .
