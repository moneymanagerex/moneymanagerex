		*** Build instructions for MS Windows platform ***


You can build MMEx on Windows using Visual C++ 2003 or later, or MinGW with GCC.
Firstly you can run mmex/bootstrap.bat which will refresh makefiles and projects.
Bakefile must be installed to do that. But you can skip this step if you just want
to build sources.


WARNING: all files in this directory generated from bakefile's scripts (..\bakefiles).
         Do not modify these files and never commit them to svn. All changes will be lost
         if bootstrap.bat will rerun!!! These files should be commited only if you changed
         some bakefile scripts and run bootstrap.bat to refresh content of this directory.



	*** Prerequisites ***


1. Install wxWidgets 2.8.10 and above in a directory like C:\wxWidgets-2.8.10
   and then create an environment variable WXWIN to point to that directory.

2. Follow the instructions given by WxWidgets install on how to build it.
   Build these configurations: Unicode Release, Unicode Debug, Release, Debug
   (or only those you need).

3. Install Boost C++ Libraries and create an environment variable BOOSTDIR
   that points to directory where Boost were installed.

4. Install GNU GetText for Windows, http://gnuwin32.sourceforge.net/packages/gettext.htm.
   This software requires to generate binary translation files from .po files.
   Append path to bin folder of installed software to PATH environment variable
   (usually "C:\Program Files\GnuWin32\bin").

5. To build installation (setup) of MMEX download and install InnoSetup Quick Start Pack
   (which includes InnoSetup itself and Inno Setup Preprocessor). Append path to folder
   of installed software (something like a "C:\Program Files\Inno Setup 5")
   to PATH environment variable.



	*** Compiling using Visual C++ command prompt ***


1.Open "Visual Studio 200X Command Prompt".
2.Change working directory to mmex\build\msw.
3.Edit config.vc to select build options. You must always set these options:
  BOOST_DIR, WX_DIR, VSTUDIO_VER (for Visual Studio only).
4.Type in command prompt
  nmake -f makefile.vc
  or if you don't like to edit config.vc, type something like that
  nmake -f makefile.vc WX_DEBUG=1 CPU=AMD64
5.Run tests (you can skip this step)
  nmake -f makefile.vc sub_check
6.nmake -f makefile.vc install (optionally)
7.Build installation (if InnoSetup installed)
  nmake -f makefile.vc sub_setup

  This is the most preferable way to build MMEX using VC++.


  	*** Compiling using Microsoft Visual C++ 2010 Express IDE ***

    Use the pre built files in the mmex/build/msw-vc-2010e directory.
    To use this directory, continue...
    
1. Ensure Prerequisites are complete with a build of wxWidgets 2.8.11 or greater
   using "Unicode Release" and/or "Unicode Debug" depending on preferences below.
   
2  Copy the mmex/build/msw directory to mmex/build/msw-vc-2010e

3  Open the project file mmex/build/msw-vc-2010e/mmex.sln.
   MS VC++ 2010e will convert the project files to the new compiler version.
   
4  In the Solution Explorer set the mmex project as the Startup Project
   Add any missing files to the projects header and source structure.
   
3. Using the Build ->configurations Manager...
   Select one of following configurations to build (accordingly to wxWidgets builds):
   "Unicode Release Multilib WX_PYTHON_0" <-- choice number one for most people
   "Unicode Debug Multilib WX_PYTHON_0"

4. Build the solution...


	*** Compiling using VisualStudio IDE ***


1. Open build\msw\mmex.sln under mmex directory.
   Select one of following configurations to build (accordingly to wxWidgets builds):

   Unicode builds, for Windows 2000 or later
   "Unicode Release Multilib WX_PYTHON_0" <-- choice number one for most people
   "Unicode Debug Multilib WX_PYTHON_0"

   Ansi builds, for Windows 95/98/Me.
   "Release Multilib WX_PYTHON_0"
   "Debug Multilib WX_PYTHON_0"

   To build unit tests use solution build\msw\tests\mmex_tests.sln. Build tests,
   copy mmex_tests.exe to mmex\runtime and run one.

2. IDE cannot translate files from mmex\runtime\en\*.po. You should use one of makefiles
   in that directory to build translations. For example, run Visual Studio Command Prompt,
   change dir to mmex\runtime\en and type "nmake -f makefile.vc".
   The same you should do to build setup in mmex\setup\win32.



	*** Compiling using MinGW ***


1.Install the latest version of MinGW, GCC compiler and mingw-make for it.
  Add environment variable MINGWDIR which points to directory of MinGW.

2.Build wxWidgets library using gcc from MinGW. Use these options in config.gcc:
  SHARED ?= 0
  MONOLITHIC ?= 1 
  USE_QA ?= 1

3.You can create mingw.bat file to run gcc with next content (assume MinGW installed in "C:\bin\MinGW")

  set MINGWDIR=C:\bin\MinGW
  set PATH=%MINGWDIR%\bin;C:\Program Files\Inno Setup 5
  mingw32-make -f makefile.gcc %* 2>mingw.log

4.Edit config.gcc to select build options (Ansi\Unicode, Debug\Release, Static\Shared, etc.).
5.mingw.bat
6.mingw.bat check (executes some tests)
7.mingw.bat install (optionally)
8.mingw.bat setup (if InnoSetup installed)


	
	*** Building 64-bit MMEX using Visual C++ *** 


  64-bit tools are not available on Visual C++ Express Edition by default. To enable 64-bit tools 
  on VC++ Express, install the Windows Software Development Kit (SDK) in addition to VC++ Express.
  Otherwise, an error occurs when you attempt to configure a project to target a 64-bit platform.

1.As usually, firstly you should build wxWidgets. 
  Run Visual Studio Command Prompt x64 (or IA-64), change dir to wxWidgets\build\msw.
  Edit config.vc and set TARGET_CPU to your system's CPU (AMD64 in most cases).
  Run nmake -f makefile.vc 
  and you will build libraries at wxWidgets\lib\vc_amd64_lib.

2.Change dir to trunk\mmex\build\msw, set TARGET_CPU in config.vc and type
  nmake -f makefile.vc again. After this your will get 64-bit MMEX :-)



	*** Your compiler do not present in this document *** 


This means someone should modify bakefile build scripts to add output for your compiler.
Run bakefile --help to list makefiles\projects that one can generate.

The mmex build scripts resides in mmex\build\bakefiles. 
Firstly modify Bakefiles.bkgen to add missing output formats.
Append new format to tag <add-formats> and add flag to makefile's output directory.
If you are lucky, you will get makefile\project for your compiler after running mmex\bootstrap[.bat].
