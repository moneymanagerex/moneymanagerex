Building Money Manager Ex from sources
======================================

Jump to [Windows] | [Linux] | [macOS] instructions.


Microsoft Windows
-----------------

### Prerequisites

1. Install free [Microsoft Visual Studio] Community 2017. Select at least the following
   components during install:
   * Visual Studio Core Editor (already selected)
   * Windows: Desktop development with C++ options
         * VC++ 2017 v141 toolset (x86,x64)
         * Windows 10 SDK (10.0.x.x) for Desktop C++ x86 and x64
         * Visual C++ tools for CMake

   Older versions of Visual Studio should work but some settings must be
   adjusted. Please remember that different VS versions use different toolsets.

   | Name               | Version | Toolset |
   |--------------------|:-------:|:-------:|
   | Visual Studio 2008 |   9.0   |    90   |
   | Visual Studio 2010 |  10.0   |   100   |
   | Visual Studio 2012 |  11.0   |   110   |
   | Visual Studio 2013 |  12.0   |   120   |
   | Visual Studio 2015 |  14.0   |   140   |
   | Visual Studio 2017 |  15.0   |   141   |

2. If you use Visual Studio older than 2013 Update 1, download and install
   [Git for Windows] with default options.

3. Download and install [gettext pre-compiled binaries] with default options.

4. Download [wxWidgets 3.x binaries] for the architecture that you're going
   to build for (32-Bit: x86, 64-Bit: x64):

   | x86 | x64 |
   | --- | --- |
   | `wxWidgets-3.*.*_Headers.7z` | `wxWidgets-3.*.*_Headers.7z` |
   | `wxMSW-3.*.*-vc141_Dev.7z` | `wxMSW-3.*.*-vc141_x64_Dev.7z` |
   | `wxMSW-3.*.*-vc141_ReleaseDLL.7z` | `wxMSW-3.*.*-vc141_x64_ReleaseDLL.7z` |
   
   | arch | files |
   | --- | --- |
   | x86 | `wxWidgets-3.*.*_Headers.7z`, `wxMSW-3.*.*-vc141_Dev.7z`, `wxMSW-3.*.*-vc141_ReleaseDLL.7z` |
   | x64 | `wxWidgets-3.*.*_Headers.7z`, `wxMSW-3.*.*-vc141_x64_Dev.7z`, `wxMSW-3.*.*-vc141_x64_ReleaseDLL.7z` |
   
   Unpack the archives to `C:\wxWidgets\` or `C:\Program Files\wxWidgets\`.

   Set the environment variable pointing to the wxWidgets folder containing the `include` and `lib` folders:
   
       setx WXWIN c:\path\to\wxWidgets\

5. Developer Command Prompt

   Start it from the *Start Menu* using *x64 Native Tools Command Prompt for VS 2017*
   or *x86 Native Tools Command Prompt for VS 2017* (names my vary in different VS
   versions).

   Or start the following command from the start menu:
   
   | arch | command line |
   | --- | --- |
   | x86 | `%comspec% /k "%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x86` |
   | x64 | `%comspec% /k "%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" amd64` |
   
   The following command can be used with older VS versions (change 14.0 with the
   version number that applies in your case):

   | arch | command line |
   | --- | --- |
   | x86 | `%comspec% /k "%ProgramFiles(x86)%\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x86` |
   | x64 | `%comspec% /k "%ProgramFiles(x86)%\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64` |


**IMPORTANT**  
__All the following commands must be run from this command prompt!__

6. Clone [MMEX's official Git repository] with its submodules using this command-line:

       git clone --recursive https://github.com/moneymanagerex/moneymanagerex c:\path\to\your\projects\mmex

   You can select a specific MMEX version by adding for example the `--branch v1.4.0` parameter.

   If the git command is not recognized, you can either use VS' minimal git client.
   First find out where git.exe is located and then add its directory to the `PATH` variable
   (otherwise it will report [missing DLLs]), before trying to run the above line again:

       cd \
       dir /n/b/s git.exe
       set "PATH=%PATH%;c:\path\to\gits\cmd\dir"

   Or use the built-in Visual Studio [IDE Team Services] to clone MMEX's repository:
   - Open Team Explorer using `Team`->`Manage Connections...`
   - Select `Clone` under `Local Git Repositories`
   - Put `https://github.com/moneymanagerex/moneymanagerex.git` into the URL field
   - Specify the `c:\path\to\your\projects\mmex` directory in the field below the URL
   - Select the `Recursively Clone Submodule` check-box
   - Click the `Clone` button below

7. Apply patches from the `util` directory to the CMake modules
   (some might not apply and will be omitted automatically).

       cd "%DevEnvDir%CommonExtensions\Microsoft\CMake\CMake\share\"
       dir cmake*
       cd cmake-<version>\Modules
       for %p in (c:\path\to\your\projects\mmex\util\*.cmake-*.patch) do git apply --ignore-space-change --ignore-whitespace --whitespace=nowarn %p

   See the previous step for instructions if the git command is not recognized.

8. [Download sources of curl], unpack them to `c:\` and build the [libcurl]
   library with the following commands:

       mkdir c:\curl-<version>\build
       cd c:\curl-<version>\build
       set "PATH=%PATH%;%DevEnvDir%CommonExtensions\Microsoft\CMake\CMake\bin"
       
   | arch | command line |
   | --- | --- |
   | x86 | `cmake -G "Visual Studio 15 2017" -DBUILD_CURL_EXE=OFF -DHTTP_ONLY=ON ^` <br> `-DENABLE_MANUAL=OFF -DBUILD_TESTING=OFF -DCURL_STATICLIB=ON ^` <br> `-DCMAKE_USE_WINSSL=ON -DCMAKE_INSTALL_PREFIX=c:\libcurl ..`
   | x64 | `cmake -G "Visual Studio 15 2017 Win64" -DBUILD_CURL_EXE=OFF -DHTTP_ONLY=ON ^` <br> `-DENABLE_MANUAL=OFF -DBUILD_TESTING=OFF -DCURL_STATICLIB=ON ^` <br> `-DCMAKE_USE_WINSSL=ON -DCMAKE_INSTALL_PREFIX=c:\libcurl ..`
       
       set "CL=/MP"
       cmake --build . --target install --config Release --clean-first ^
         -- /maxcpucount /verbosity:minimal /nologo /p:PreferredToolArchitecture=x64

9. Then you should follow one of  
   [Visual Studio project] | [Visual Studio CLI] | [Visual Studio CMake]

### Visual Studio GUI with project file

This should work with different versions of Visual Studio and uses native
tools to manage projects in VS IDE.

1. Generate build environment using [CMake]

       mkdir c:\path\to\your\projects\mmex\build
       cd c:\path\to\your\projects\mmex\build
       set "PATH=%PATH%;%DevEnvDir%CommonExtensions\Microsoft\CMake\CMake\bin"

   | arch | command line |
   | --- | --- |
   | x86 | `cmake -G "Visual Studio 15 2017" -DCMAKE_PREFIX_PATH=c:\libcurl ..`
   | x64 | `cmake -G "Visual Studio 15 2017 Win64" -DCMAKE_PREFIX_PATH=c:\libcurl ..`
   
   Among other files this produces the VS solution file `c:\path\to\your\projects\mmex\build\MMEX.sln`.

2. In the Visual Studio GUI, open the above solution file with the `File`->`Open`->`Project/Solution...` menu
   command (or hit `Ctrl+Shift+O`).

3. Run the `Build`->`Build Solution` menu command (or hit `Ctrl+Shift+B`). This will
   compile MMEX and propagate needed files into the right directories.

4. In order to debug the application, you need to set the path to wxWidgets'
   development DLLs. Go to `mmex Properties \ Debugging \ Environment` and set
   it to `PATH=%PATH%;C:\wxwidgets\lib\vc141_x64_dll`.
   Otherwise the application will [report missing DLLs].

4. Now you can run MMEX with the `Debug`->`Start Without Debugging` menu command
   (`Ctrl+F5`) or start a debugging session with `Debug`->`Start Debugging` (`F5`).

5. To create a binary package (you need to have [NSIS] installed for this), build the
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


### Visual Studio 2017 GUI with native CMake support

This will work in Visual Studio 2017 or newer with _Visual C++ tools for
CMake_ option installed (gets selected by default).

1. Open `c:\path\to\your\projects\mmex` with the `File`->`Open`->`Folder...` menu command
   (or hit `Ctrl+Shift+Alt+O`).

2. The very first time VS opens the directory, it will start to run CMake
   config immediately, which might fail with errors. Ignore this and proceed
   to the next steps to set up CMake correctly.

3. Select a target like `x64-Debug` in the `Project Settings` drop-down.

4. Set the path to the libcurl library using the menu item `CMake`->`Change CMake Settings`,
   then in _CMakeSettings.json_, supplement each of the target configurations
   that you're going to run with the following variable definition (see the paragraph 
   _CMakeSettings.json example_ in [this blog post]:

       "variables": [
         {
           "name": "CMAKE_PREFIX_PATH",
           "value": "c:\libcurl"
         }

5. Run the `CMake`->`Install`->`Project MMEX` menu command before starting a
   debugging session with `CMake`->`Debug`->`src\mmex.exe`.

6. Select `src\mmex.exe` in the `Select Startup Item` drop-down to unlock commands
   in the Debug menu.


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

       /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"

3. Install required packages. You can choose which compiler you want to use:

       brew update && brew install wxmac gettext cmake
       brew link --force gettext

#### 2. Download Sources

    git clone --recursive https://github.com/moneymanagerex/moneymanagerex

You can select MMEX version by adding `--branch v1.4.0` parameter.

#### 3. Compile and Create Package

    mkdir moneymanagerex/build
    cd moneymanagerex/build
    export MAKEFLAGS=-j4
    cmake -DCMAKE_BUILD_TYPE=Release ..
    cmake --build . --target package

If you want build the project with for debugging proposes replace CMake flag
`-DCMAKE_BUILD_TYPE=Release` with `-DCMAKE_BUILD_TYPE=Debug`.

You could tune `-j4` option to different number to use all processor cores
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

Same as for [macOS](#2-download-sources)
	
#### 4. Compile and Create Package

Same as for [macOS](#3-compile-and-create-package)

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
[this blog post]:
    https://blogs.msdn.microsoft.com/vcblog/2016/10/05/cmake-support-in-visual-studio/#configure-cmake
[missing DLLs]: https://developercommunity.visualstudio.com/content/problem/26539/vs2017-deployed-gitexe-not-usable.html
[report missing DLLs]: https://github.com/moneymanagerex/moneymanagerex/issues/1676
