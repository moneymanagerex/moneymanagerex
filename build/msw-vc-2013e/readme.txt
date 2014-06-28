Build instructions for MS Windows platform
===================================================================================
Microsoft Visual C++ 2013 Express compiler
-----------------------------------------------------------------------------------

NOTE:
    The directory msw-vc-2013e has the project files for:
    Microsoft Visual C++ 2013 Express.
    For other compilers, see the readme file in the mmex/build/msw directory.

Prerequisites
-----------------------------------------------------------------------------------
1. Installation of the following:
   MS-VC 2013e  http://www.microsoft.com/visualstudio/eng/downloads#d-2013-express
   wxWidgets    http://www.wxwidgets.org/downloads/
   Source code  https://github.com/moneymanagerex/moneymanagerex
   
2. Create environment variables for wxWidgets. (See below)
-----------------------------------------------------------------------------------

Optional software installation:
===============================
   GitHub Windows   https://windows.github.com/

   Git              http://git-scm.com/downloads
   TortoiseGit      http://code.google.com/p/tortoisegit/
                    A Windows interface to GitHub to download the source code.
  
   Doxygen          http://www.stack.nl/~dimitri/doxygen/download.html
                    Allows documentation to be obtained fromthe source code.
   PoEdit           http://www.poedit.net/
                    Allows the generation of .po and .mo files
                    required for language translations. 

   GNU GetText for Windows: http://gnuwin32.sourceforge.net/packages/gettext.htm.
   This package may also be required on systems to allow translations to work.
   Append path to bin folder of installed software to PATH environment variable
   (usually "C:\Program Files\GnuWin32\bin").

To build the setup installation, download and install InnoSetup Quick Start Pack.
   This includes InnoSetup itself and Inno Setup Preprocessor. Append the folder
   path to the Windows Environment Variable: PATH for the installed software.
   (something like a "C:\Program Files\Inno Setup 5")

Compiling using Microsoft Visual C++ 2013 Express IDE
-----------------------------------------------------------------------------------
Note: wxWidgets must be built using MS VC++ 2013e, to ensure a successful build.

1. Ensure the prerequisites are complete with an installation of wxWidgets 3.0.1
   or greater, compiled using MS VC++ 2013e

2. Using MS VC++ 2013e, select the project file in the wxWidgets 3.0.1 folder.
   (.../wxWidgets-3.0.1/build/msw/wx_vc12.sln)

   Using MS VC++ 2013e, build the appropriate configurations for your needs.
   Recommended configurations for mmex: Release, Debug

   Aditional information on how to install and build wxWidgets can be found here:
   http://wiki.wxwidgets.org/Microsoft_Visual_C%2B%2B_Guide#Visual_Studio_C.2B.2B_2013

3  Locate the directory: moneymanagerex\build\msw-vc-2013e
   Open the project file: moneymanagerex\build\msw-vc-2013e\mmex.sln
   MS VC++ 2013e will set up the project files to the user's environment.
   
4  In the Solution Explorer set the mmex project as the Startup Project
   
5. Using the Build ->configurations Manager...
   Select one of following configurations to build accordingly to wxWidgets builds:
   Release  <-- Best choice for most people.
   Debug    <-- Extra debug code added for debugging and may be slower.

4. Build the mmex solution...

5. Locate and Run the batch file: update_mmex_build_files.bat
   location: moneymanagerex\setup\win\update_mmex_build_files.bat

   This will allow MS VC++ 2013e IDE to run mmex.exe successfully.

Windows Configuration
=====================

Environment variables need to access wxWidgets builds on a Windows system.
------------------------------------------------------------------------------------
Variable  | Value                    | Comment
----------|--------------------------|----------------------------------------------
WXWIN     | C:\CPP\wxWidgets-3.0.1   | Location of wxWidgets 3.0.1


Setting up Environment variables using Windows 7
------------------------------------------------------------------------------------
Control Panel ---> System and Security ---> System
Advanced system settings        (Located on left navigation panel)

System Properties       (Dialog) ---> Advanced  (Tab)
Environment Variables   (Button)

------------------------------------------------------------------------------------
End of File.
