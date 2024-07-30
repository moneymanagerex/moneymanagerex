Building Money Manager Ex from sources
======================================

Jump to [Windows] | [Linux] | [macOS] instructions.


Microsoft Windows
-----------------

### Prerequisites

1. Install free [Microsoft Visual Studio] Community 2022 (64-bit). Select following
   components during install:
   * Windows: Desktop development with C++
     * Visual Studio Core Editor
     * Desktop development with C++ options
       * MSVC v 143 - VS 2022 C++ x64/x86 build tools
       * Windows 10 SDK (10.0.x.x) for Desktop C++ x86 and x64
       * C++ CMake tools for Windows

   Older versions of Visual Studio should work but some settings must be
   adjusted. Please remember different VS versions uses different toolsets.

   | Name               | Version | Toolset |
   |--------------------|:-------:|:-------:|
   | Visual Studio 2008 |   9.0   |    90   |
   | Visual Studio 2010 |  10.0   |   100   |
   | Visual Studio 2012 |  11.0   |   110   |
   | Visual Studio 2013 |  12.0   |   120   |
   | Visual Studio 2015 |  14.0   |   140   |
   | Visual Studio 2017 |  15.0   |   141   |
   | Visual Studio 2019 |  16.0   |   142   |
   | Visual Studio 2022 |  17.0   |   143   |

2. Download and install [gettext pre-compiled binaries] with default options.

3. Download [wxWidgets 3.x binaries]:
   - `wxWidgets-3.*.*_Headers.7z`
   - one of `wxMSW-3.*.*-vc141_Dev.7z` or `wxMSW-3.*.*-vc141_x64_Dev.7z`
   - one of `wxMSW-3.*.*-vc141_ReleaseDLL.7z`
     or `wxMSW-3.*.*-vc141_x64_ReleaseDLL.7z`
   
   Unpack archives to `c:\wxWidgets\` or `c:\Program Files\wxWidgets\`.

   You may select different directory but then `wxwin` environment variable
   must be set:

       setx wxwin c:\path\to\unpacked\wxwidgets\files

4. Tools - Command Line - Developer Command Prompt

   Start it from *Main Menu*

   Or start the following command from start menu:

       %comspec% /k "%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" amd64

   Following command can be used with older VS versions (change 14.0 for correct version number):

       %comspec% /k "%ProgramFiles(x86)%\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x86

   Change `x86` parameter to `amd64` for native 64-bit build.

**IMPORTANT**  
__All following commands must be run from this command prompt!__

5. Clone [MMEX official Git repository] with submodules using command-line:

       git clone --recursive https://github.com/moneymanagerex/moneymanagerex c:\projects\mmex

   If git command is not recognized and you want to use git installed with VS
   you should find `git.exe` file and add its directory to the `PATH` variable

       dir /n/b/s c:\%ProgramFiles(x86)%\git.exe
       set "PATH=%PATH%;c:\path\to\git\dir"

   Or use build-in Visual Studio [IDE Team Services] to clone:
   - Open Team Explorer using `Team`->`Manage Connections...`
   - Select `Clone` under `Local Git Repositories`
   - Put `https://github.com/moneymanagerex/moneymanagerex.git` into URL field
   - Select `c:\projects\mmex` directory in field below URL
   - Select `Recursively Clone Submodule` check-box
   - Click `Clone` button below

6. Apply patches from `util` directory to CMake modules

       cd "%DevEnvDir%CommonExtensions\Microsoft\CMake\CMake\share\cmake-3.8\Modules"
       for %p in (c:\projects\mmex\util\*.cmake-*.patch) do git apply --ignore-space-change --ignore-whitespace --whitespace=nowarn %p

   See previous step for instructions if git command is not recognized.

7. [Download sources of curl], unpack them to `c:\` and build [libcurl]
   library with following commands:

       mkdir c:\curl-<version>\build
       cd c:\curl-<version>\build
       set "PATH=%PATH%;%DevEnvDir%CommonExtensions\Microsoft\CMake\CMake\bin"
       cmake -G "Visual Studio 17 2022" -A x64 -DBUILD_CURL_EXE=OFF -DHTTP_ONLY=ON ^
         -DENABLE_MANUAL=OFF -DBUILD_TESTING=OFF -DCURL_STATICLIB=ON ^
         -DCURL_USE_SCHANNEL=ON -DCMAKE_INSTALL_PREFIX=c:\libcurl ..
       set "CL=/MP"
       cmake --build . --target install --config Release --clean-first ^
         -- /maxcpucount /verbosity:minimal /nologo /p:PreferredToolArchitecture=x64

   Replace `-A x64` with `-A Win32` to remove 64-bit support.

8. Then you should follow one of  
   [Visual Studio project] | [Visual Studio CLI] | [Visual Studio CMake]

### Visual Studio GUI with project file

This should work with different versions of Visual Studio and uses native
tools to manage projects in VS IDE.

1. Generate build environment using [CMake]

       mkdir c:\projects\mmex\build
       cd c:\projects\mmex\build
       set "PATH=%PATH%;%DevEnvDir%CommonExtensions\Microsoft\CMake\CMake\bin"
       cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH=c:\libcurl ..

   This produce `c:\projects\mmex\build\MMEX.sln` file ready to be loaded into
   Visual Studio GUI.

2. Open above solution file with `File`->`Open`->`Project/Solution...` menu
   command (or `Ctrl+Shift+O`).

3. Run `Build`->`Build Solution` menu command (or `Ctrl+Shift+B`). This will
   compile MMEX and propagate support files into right directories.

4. Now you can run MMEX with `Debug`->`Start Without Debugging` menu command
   (`Ctrl+F5`) or start debugging session with `Debug`->`Start Debugging`
   (`F5`).

5. To create binary package (you need to have [NSIS] installed for this) build
   `PACKAGE` project.

### Visual Studio command-line

Use this method if you don't want to use Visual Studio GUI at all. It's very
similar to Unix compile methods and can be useful for batch builds, CI etc.

Look at [.appveyor.yml] file for more complicated command line builds
scenarios examples (debug builds, using different toolset version, Windows
XP/2003 compatible builds, compile wxWidgets from sources, shared DLL
or static linking).

1. Execute step 1 from [Visual Studio project] above

2. Compile with

       set "CL=/MP"
       cmake --build . --target install --config Release -- /maxcpucount /verbosity:minimal /nologo /p:PreferredToolArchitecture=x64

   Now you can run MMEX starting `c:\projects\mmex\build\install\bin\mmex.exe`
   file. `c:\projects\mmex\build\install` directory contains portable version.

3. Build binary package (you need to have [NSIS] installed for this)

       cpack .

   Windows installer and zip archive with portable package should be produced.


### Visual Studio 2022 GUI with native CMake support

This will work in Visual Studio 2017 or newer with _Visual C++ tools for
CMake_ option installed.

1. Open `c:\projects\mmex` with `File`->`Open`->`Folder...` menu command
   (or `Ctrl+Shift+Alt+O`).

2. Select target like `x64-Debug` in project settings drop-down.

3. Set path to libcurl library using `CMake`->`Change CMake Settings` then adding following variables into _CMakeSettings.json_:

       "variables": [
         {
           "name": "CMAKE_PREFIX_PATH",
           "value": "c:\libcurl"
         }
        
   See detailed instructions for [configuring CMake projects] from Microsoft _Visual C++ Team Blog_.

4. Run `CMake`->`Install`->`Project MMEX` menu command before debugging
   session start with `CMake`->`Debug`->`src\mmex.exe`.

5. Select `src\mmex.exe` in `Select Startup Item` drop-down to unlock commands
   in Debug menu.


macOS with Homebrew
-------------------

#### 1. Install Prerequisites

1. First verify, if you have installed *Xcode Command Line Tools*. Open
   terminal and type `git --version` if it shows something like
   `git version 2.11.0 (Apple Git-81)`
   you are fine. If not, you will be prompted to *Install* them via your
   operating system. Alternatively, you can install those tools via command:

       xcode-select --install

2. After that, for comfortable installing software we use [Homebrew].
   Run the command:

       /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

3. Install required packages.

       brew update && brew install ccache gettext cmake
       brew link --force gettext

#### 2. Build wxWidgets

Current stable version that has been tested with MMEX is v3.2.5

1. Download Sources
        
        /bin/bash -c "$(curl -fsSL -O https://github.com/wxWidgets/wxWidgets/releases/download/v3.2.5/wxWidgets-3.2.5.tar.bz2)"
        tar xzf wxWidgets-*.tar.bz2

2. Build from source

        cd wxWidgets-3.2.5
        mkdir build-cocoa
        cd build-cocoa
        export MAKEFLAGS=-j4
        ../configure --disable-shared --enable-cxx11 --with-cxx=11 \
        --with-macosx-version-min=10.14 \
        --without-libtiff \
        --enable-universal-binary=arm64,x86_64
        make

    If you want to enable debug then include `--enable-debug`

    If you want to just build for the current architecture and don't require a universal build then you can omit `--enable-universal-binary=arm64,x86_64`

    You could tune `-j4` option to a different number to use all processor cores during build phase.


#### 3. Download latest MMEX sources

    git clone --recursive https://github.com/moneymanagerex/moneymanagerex

#### 4. Compile and Create Package

    mkdir moneymanagerex/build
    cd moneymanagerex/build
    export MAKEFLAGS=-j4
    cmake -DCMAKE_CXX_FLAGS="-w" \
    -DwxWidgets_CONFIG_EXECUTABLE={PATH-TO-wxWidgets}/wxWidgets-3.2.5/build-cocoa/wx-config \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=10.10 ..
    cmake --build . --target package

Replace `{PATH-TO-wxWidgets}` with the path to the directory in which you extracted the wxWidgets source in step 2.

If you want build the project for debugging purposes replace CMake flag
`-DCMAKE_BUILD_TYPE=Release` with `-DCMAKE_BUILD_TYPE=Debug`.

If you want to just build for the current architecture and don't require a universal build then you can omit `-DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"`

You could tune `-j4` option to a different number to use all processor cores
during build phase.

Linux
-----

#### 1. Install Prerequisites

All required packages should be installed from official distribution
using native package manager:

| Distribution         | Update packages list  | Install package              |
|----------------------|-----------------------|------------------------------|
| Debian, Ubuntu, Mint | `sudo apt update`     | `sudo apt install <pkg>`     |
| openSUSE             | `sudo zypper refresh` | `sudo zypper install <pkg>`  |
| Fedora               | `sudo dnf --refresh repolist` | `sudo dnf install <pkg>` |
| CentOS               | `sudo yum clean expire-cache` | `sudo yum install <pkg>` |
| Arch                 | `sudo pacman -Syy`    | `sudo pacman -Syu <pkg>`     |
| Slackware            | `sudo slackpkg update`| `sudo slackpkg install <pkg>`|

It is good practise to update packages list before installing new packages.
This allows install or update required dependencies to the latest versions.

List of required packages for each distribution is available in *dockerfiles*
in [dockers] subdirectory. Look for file
`dockers/<distribution>.<codename>/Dockerfile` - i.e. [Dockerfile for Ubuntu
zesty].

Sometimes additional steps / workarounds are needed for specific distribution
or version - they are included in above mentioned dockerfiles. Please run them
before next steps.

To use wxWidget 3.1.5 see:
https://docs.codelite.org/wxWidgets/repo315/#ubuntu-and-debian
or to build it from sources:
https://docs.codelite.org/build/build_wx_widgets/

#### 2. Select C++ Compiler

You can choose which compiler you want to use: GCC (default in most Linux
distributions) or CLang (optional). If you want use CLang you should install
additional package and select it as default compiler:

| Distribution         | CLang pkg  | Change compiler to CLang |
|----------------------|------------|--------------------------|
| Debian, Ubuntu, Mint | clang      | `sudo update-alternatives --set cc /usr/bin/clang`<br>`sudo update-alternatives --set c++ /usr/bin/clang++`|
| openSUSE             | llvm-clang | `sudo ln -sf /usr/bin/clang /usr/bin/cc`<br>`sudo ln -sf /usr/bin/clang++ /usr/bin/c++`|
| Fedora, CentOS       | clang      | `sudo ln -sf /usr/bin/clang /usr/bin/cc`<br>`sudo ln -sf /usr/bin/clang++ /usr/bin/c++`|
| Arch                 | clang      | `export CC=clang CXX=clang++` |
| Slackware            | llvm       | `export CC=clang CXX=clang++` |

#### 3. Download Sources

You need git to download the sources, see prerequisites above.

If you want the stable 1.7.0 version of mmex:

    git clone --recursive https://github.com/moneymanagerex/moneymanagerex  --branch v1.7.0

If you want the latest (possibly unstable) version of the trunk:

    git clone --recursive https://github.com/moneymanagerex/moneymanagerex
	
#### 4. Compile and Create Package

Got to the directory, where you downloaded the sources from github.

    cd moneymanagerex/
    mkdir build
    cd build/
    cmake ..

E.g. for the trunk version the output should be something like:

    -- MMEX configuration summary
    -- ==========================
    -- Version        : 1.7.1-Beta.1
    -- Commit         : 1edc0dfe2 (2024-01-16)
    -- Branch         : master
    -- Host system    : Linux x86_64
    -- Target system  : Linux x86_64
    -- Build types    : 
    -- Generator      : Unix Makefiles
    -- Install prefix : /usr/local
    -- DB encryption  : ON
    -- 
    -- Versions
    -- --========--
    -- Linux 6.1.38
    -- Debian.12.bookworm
    -- Debian.GNU/Linux.12.(bookworm)
    -- CMake 3.25.1
    -- GNU Make 4.3
    -- ccache 4.7.5
    -- GNU 12.2.0
    -- wxWidgets 3.2.4
    -- wxSQLite3 4.9.1
    -- Lua 5.3.6
    -- curl 7.88.1
    -- gettext 0.21
    -- ------------------ 8< -----------------
    -- Configuring done
    -- Generating done
    -- Build files have been written to: <your $HOME >/<directory where you downloaded mmex>/moneymanagerex/build

Now build

    export MAKEFLAGS=-j4  (to speed up the build)
    cmake --build . --target package

If the build was ok the last message should be something like:

    [...]
    [100%] Built target mmex
    Run CPack packaging tool...
    CPack: Create package using DEB
    CPack: Install projects
    CPack: - Run preinstall target for: MMEX
    CPack: - Install project: MMEX
    CPack: Create package
    CPackDeb: - Generating dependency list
    CPack: - package: <directory where you saved the download from git>/moneymanagerex/mmex_1.7.1-Beta.1-Debian.12.bookworm_amd64.deb generated.

    For testing without installing you can run
    ../_CPack_Packages/Linux/DEB/mmex-1.7.1-Beta.1-Linux/usr/bin/mmex
    
#### 5. Install MMEX Package

| Distribution         | Install package from local file              |
|----------------------|----------------------------------------------|
| Debian, Ubuntu, Mint | `sudo apt install ./mmex-<version>.deb`      |
| openSUSE             | `sudo zypper install ./mmex-<version>.rpm`   |
| Fedora               | `sudo dnf install ./mmex-<version>.rpm`      |
| CentOS               | `sudo yum install ./mmex-<version>.rpm`      |
| Arch                 | `sudo pacman -U ./mmex-<version>.pkg.tar.xz` |
| Slackware            | `sudo installpkg ./mmex-<version>.txz`       |

[Windows]: #microsoft-windows
[Visual Studio CLI]: #visual-studio-command-line
[Visual Studio project]: #visual-studio-gui-with-project-file
[Visual Studio CMake]: #visual-studio-2017-gui-with-native-cmake-support
[Linux]: #linux
[macOS]: #macos-with-homebrew
[dockers]: dockers/
[Dockerfile for Ubuntu zesty]: dockers/ubuntu.zesty/Dockerfile
[Microsoft Visual Studio]:
    https://www.visualstudio.com/downloads/
[Git for Windows]:
    https://git-scm.com/download/win
[gettext pre-compiled binaries]:
    https://mlocati.github.io/articles/gettext-iconv-windows.html
[wxWidgets 3.x binaries]:
    https://github.com/wxWidgets/wxWidgets/releases/
[Developer Command Prompt]:
    https://docs.microsoft.com/en-us/dotnet/framework/tools/developer-command-prompt-for-vs
[MMEX official Git repository]:
    https://github.com/moneymanagerex/moneymanagerex
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
[.appveyor.yml]:
    .appveyor.yml
[IDE Team Services]:
    https://www.visualstudio.com/en-us/docs/git/tutorial/clone#clone-a-repo
[Download sources of curl]:
    //curl.haxx.se/latest.cgi?curl=zip
[libcurl]:
    https://curl.haxx.se/libcurl/
[configuring CMake projects]:
    https://blogs.msdn.microsoft.com/vcblog/2016/10/05/cmake-support-in-visual-studio/#configure-cmake
