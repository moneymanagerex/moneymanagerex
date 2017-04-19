# Building MMEX on Windows

## Install Prerequisites
Set [system environment](http://www.computerhope.com/issues/ch000549.htm) `wxwin: c:\projects\wxwidgets`

Obtain **GETTEXT** library via official page
https://www.gnu.org/software/gettext/
where for windows version is recomendation to download it from
https://mlocati.github.io/articles/gettext-iconv-windows.html

## Download wxWidgets

	git clone https://github.com/wxWidgets/wxWidgets.git %WXWIN%
	cd %WXWIN%
	git checkout tags/v3.1.0
	cd build/msw

Choose your architecture and via visual studio developer console run command
from *Native Tools Command Prompt for VS 2017* or *Developer Command Prompt for VS 2017* for 32 bit

	nmake /f makefile.vc TARGET_CPU=Win32 BUILD=release
or 64 bit

	nmake /f makefile.vc TARGET_CPU=x64 BUILD=release

## Download Source
	git clone https://github.com/moneymanagerex/moneymanagerex
	cd moneymanagerex
	git checkout cmake
	git submodule update --init
	
## Compile
If you want build the project with debug create debug dir in build directory
and replace cmake flag
`-DCMAKE_BUILD_TYPE=Release`
with
`-DCMAKE_BUILD_TYPE=Debug`

    cd build
    mkdir release
    cd release
    cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release ../../

Choose your architecture and via visual studio developer console run command for 32 bit

	nmake /f Makefile TARGET_CPU=Win32
or 64 bit

	nmake /f Makefile TARGET_CPU=x64 BUILD=release

If you are interested in producing *Visual Studio project file*, you can run this command instead of previous one
`cmake -G "Visual Studio 14 2015" -DCMAKE_BUILD_TYPE=Release ../../`

## Build package
For building installable package, you need to have NSIS installed http://nsis.sourceforge.net/Download

	cpack .

