Building Money Manager Ex from sources
======================================

Jump to [Windows] | [Linux] | [macOS] instructions.



Microsoft Windows with Visual Studio
------------------------------------

#### 1. Install Prerequisites

1. Install [Microsoft Visual Studio]. Free *Visual Studio Community*
   version should be enough.

2. Download and install [Git for Windows]. Once installed, you’ll be able
   to use Git from the command prompt or *PowerShell*. Use the defaults
     selected during the installation.

3. Obtain [gettext precompiled binaries] as recommended at [gettext] library
   home page.

4. Set [system environment variable] `wxwin` to `c:\projects\wxwidgets`.

5. Setup [Developer Command Prompt]:

       %comspec% /k "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat x86"

   Change `x86` parameter to `amd64` for native 64-bits build.

   Above value needs to be changed according to your *Visual Studio* version
   and installation location.
   For example VS 2017 uses:

       %comspec% /k "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars32.bat"
       %comspec% /k "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat"
       
   Or simply start it from *Start Menu* using *VS2015 x64 Native Tools
   Command Prompt* or *VS2015 x86 Native Tools Command Prompt* (names my vary
   in different VS installed versions).

   **All following commands must be run from this command prompt!**

#### 2. Build wxWidgets

To download [wxWidgets] 3.1.0 from Git repository and compile it run:

    mkdir c:\projects
    git clone --depth 1 -b v3.1.0 https://github.com/wxWidgets/wxWidgets.git %WXWIN%
    cd %WXWIN%\build\msw
    nmake /f makefile.vc TARGET_CPU=x86 BUILD=Release

Replace `x86` with `x64` for 64-bits arch.

#### 3. Download MMEX Source

Simply clone [official Git repository] with submodules using:

    git clone --recursive https://github.com/moneymanagerex/moneymanagerex c:\projects\mmex

You can select MMEX version by adding `-b v1.4.0` parameter.

#### 4. Compile

Generate build enviroment for [NMake] tool using [CMake]:

    mkdir c:\projects\mmex\build
    cd c:\projects\mmex\build
    cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release ..

If you want build the project with for debugging proposes replace cmake flag
`-DCMAKE_BUILD_TYPE=Release` with `-DCMAKE_BUILD_TYPE=Debug`.

Then start build process with:

    nmake /f Makefile TARGET_CPU=x86

Replace `x86` with `x64` for 64-bits arch.

#### 5. Build Package

For building installable package, you need to have [NSIS] installed.
Then you can create it using:

    cd c:\projects\mmex\build
    cpack .

#### Loading MMEX Project into Visual Studio GUI

If you are interested in producing *Visual Studio project file*, you can run
following command:

    cd c:\projects\mmex\build
    cmake -G "Visual Studio 14 2015" -DCMAKE_BUILD_TYPE=Release ..

or for 64-bits build:

    cd c:\projects\mmex\build
    cmake -G "Visual Studio 14 2015 Win64" -DCMAKE_BUILD_TYPE=Release ..

.vcproj file will be generated ready to be load into Visual Studio.

You can also try native [CMake support in Visual Studio 2017].


macOS with Homebrew
-------------------

#### 1. Install Prerequisites

1. First verify, if you have installed *Xcode Command Line Tools*. Open
   terminal and type `git --version` if it shows something like
   `git version 2.11.0 (Apple Git-81)`
   you are fine. If not, you will be prompted to *Install* them via your
   operating system. Alternativelly, you can install those tools via command:

       xcode-select --install

2. After that, for comfortable installing softare we use [Homebrew].
   Run the command:

       /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"

3. Install required packages. You can choose which compiler you want to use:

       brew update && brew install wxmac gettext cmake
       brew link --force gettext

#### 2. Download Sources

    git clone --recursive https://github.com/moneymanagerex/moneymanagerex

You can select MMEX version by adding `-b v1.4.0` parameter.

#### 3. Compile and Create Package

    mkdir moneymanagerex/build
    cd moneymanagerex/build
    export MAKEFLAGS=-j4
    cmake -DCMAKE_BUILD_TYPE=Release ..
    make package

If you want build the project with for debugging proposes replace cmake flag
`-DCMAKE_BUILD_TYPE=Release` with `-DCMAKE_BUILD_TYPE=Debug`.

You could tune `-j4` option to different number to use all processor cores
during build phase.


Linux
-----

#### 1. Install Prerequisites

All required packages should be installed from offical distribution
using native package manager:

| Distribution | Update packages list | Install package             |
|--------------|----------------------|-----------------------------|
| Ubuntu       | `sudo apt-get update`| `sudo apt-get install <pkg>`|
| openSUSE     | `sudo zypper update` | `sudo zypper install <pkg>` |
| Fedora       | `sudo yum update`    | `sudo yum install <pkg>`    |
| Arch         | `sudo pacman -Syy`   | `sudo pacman -Syu <pkg>`    |

It is good practise to update packages list before installing new packages.
This allows install or update required dependencies to the latest versions.

List of required packages for each distribution is avaiable in *dockerfiles*
in [dockers] subdirectory. Look for file
`Dockerfile.<distribution>.<codename>` - ie. [Dockerfile.ubuntu.zesty].

Sometimes additional steps / workarounds are needed for specific distribution
or version - they are inclued in above mentioned dockerfiles. Please run them
before next steps.

#### 2. Select C++ Compiler

You can choose which compiler you want to use: GCC (default in most Linux
distributions) or CLang (optional). If you want use CLang you should install
additional package and select it as default compiler:

| Distribution | CLang pkg  | Change compiler to CLang |
|--------------|------------|--------------------------|
| Ubuntu       | clang      | `sudo update-alternatives --set cc /usr/bin/clang`<br>`sudo update-alternatives --set c++ /usr/bin/clang++`|
| openSUSE     | llvm-clang | `sudo ln -sf /usr/bin/clang /usr/bin/cc`<br>`sudo ln -sf /usr/bin/clang++ /usr/bin/c++`|
| Fedora       | clang      | `sudo ln -sf /usr/bin/clang /usr/bin/cc`<br>`sudo ln -sf /usr/bin/clang++ /usr/bin/c++`|
| Arch         | clang      | `export CC=clang CXX=clang++`|

#### 3. Download Sources

Same as for [macOS](#2-download-sources)
	
#### 4. Compile and Create Package

Same as for [macOS](#3-compile-and-create-package)

#### 5. Install MMEX Package

| Distribution | Install package from local file             |
|--------------|---------------------------------------------|
| Ubuntu       | `sudo apt-get install ./mmex-<version>.deb` |
| openSUSE     | `sudo rpm -i ./mmex-<version>.rpm`          |
| Fedora       | `sudo rpm -i ./mmex-<version>.rpm`          |
| Arch         | `sudo pacman -U ./mmex-<version>.pkg.tar.xz`|

<!-- links -->
[Windows]: #microsoft-windows-with-visual-studio
[Linux]: #linux
[macOS]: #macos-with-homebrew
[dockers]: dockers/
[Dockerfile.ubuntu.zesty]: dockers/Dockerfile.ubuntu.zesty
[Microsoft Visual Studio]:
    https://www.visualstudio.com/downloads/
[Git for Windows]:
    https://git-scm.com/download/win
[system environment variable]:
    http://www.computerhope.com/issues/ch000549.htm
[gettext]:
    https://www.gnu.org/software/gettext/#downloading
[gettext precompiled binaries]:
    https://mlocati.github.io/articles/gettext-iconv-windows.html
[Developer Command Prompt]:
    https://docs.microsoft.com/en-us/dotnet/framework/tools/developer-command-prompt-for-vs
[official Git repository]:
    https://github.com/moneymanagerex/moneymanagerex
[NMake]:
    https://docs.microsoft.com/cpp/build/nmake-reference
[CMake]:
    https://cmake.org/
[wxWidgets]:
    https://wxwidgets.org/
[NSIS]:
    http://nsis.sourceforge.net/Download
[CMake support in Visual Studio 2017]:
    https://blogs.msdn.microsoft.com/vcblog/2016/10/05/cmake-support-in-visual-studio/
[Homebrew]:
    https://brew.sh