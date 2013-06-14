Build instructions for MS Windows platform
===================================================================================
Compiler: Microsoft Visual C++ 2010 Express
-----------------------------------------------------------------------------------

**NOTE:**
    The directory mmex/build/msw-vc-2010e has the project files for:
    Microsoft Visual C++ 2010 Express.
    For other compilers, see the readme file in the mmex/build/msw directory.

Prerequisites
-----------------------------------------------------------------------------------

1. Installation of the following software:
   MS-VC 2010e  http://www.microsoft.com/visualstudio/eng/downloads#d-2010-express
   wxWidgets    http://www.wxwidgets.org/downloads/
   -
2. Create environment variables for wxWidgets. (See table below)

**Optional Software**
    
    Doxygen      http://www.stack.nl/~dimitri/doxygen/download.html
                 Allows documentation to be obtained from the source code.
                 
    PoEdit       http://www.poedit.net/
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

Compiling using Microsoft Visual C++ 2010 Express IDE
-----------------------------------------------------------------------------------

1. Ensure Prerequisites are complete with an installation of wxWidgets 2.9.4
   or greater
   -    
2. Using MS VC++ 2010e, select the project file in the wxWidgets 2.9.4 folder.
   **(.../wxWidgets-2.9.4/build/msw/wx_vc9.sln)**
   -
   This will allow MS VC++ 2010e to convert the appropriate existing project
   files to the new environment. Build the appropriate configurations for your
   needs: Recommended configurations for mmex: **Release, Debug**
   -
   Additional information on how to install and build wxWidgets can be found here:
   http://wiki.wxwidgets.org/Microsoft_Visual_C%2B%2B_Guide#Visual_Studio_C.2B.2B_2010
   -
3. Locate the directory: mmex\build\msw-vc-2010e.
   Open the project file: mmex\build\msw-vc-2010e\mmex.sln
   MS VC++ 2010e will set up the project files to the user's environment.
   -
4. In the Solution Explorer set the mmex project as the Startup Project
   -
5. Using the Build ->configurations Manager...
   Select one of following configurations to build accordingly to wxWidgets builds:
   "Unicode Release Multilib WX_PYTHON_0" <-- choice number one for most people
   "Unicode Debug Multilib WX_PYTHON_0"
   -
6. Build the mmex solution...
   -
7. Locate and Run the batch file: update_mmex_build_files.bat
   location: mmex\trunk\mmex\setup\win\update_mmex_build_files.bat
   -
   This will allow MS VC++ 2010e IDE to run mmex.exe successfully.

Allowing Configuration Switching
--------------------------------
**New configurations to allow multiple wxWidgets builds.**

Variable  | wxWidgets Build  |  Configurations
----------|------------------|------------------------------------------------------
WXWIN     | wxWidgets 2.x.xx |  set for generic Windows configurations.
WXWIN_29  | wxWidgets 2.9.xx |  wx29 Debug Unicode Multilib WX_PYTHON_0
          |                  |  wx29 Release Unicode Multilib WX_PYTHON_0

**Environment variables need to access the different builds on Windows.**

Variable  | Value                    | Comment
----------|--------------------------|----------------------------------------------
WXWIN     | C:\CPP\wxWidgets-2.9.xx  | Location of wxWidgets 2.9.xx
WXWIN_29  | C:\CPP\wxWidgets-2.9.4   | Location of wxWidgets 2.9.4

Setting up Environment variables using Windows 7
------------------------------------------------------------------------------------
Control Panel ---> System and Security ---> System
Advanced system settings        (Located on left navigation panel)

System Properties       (Dialog) ---> Advanced  (Tab)
Environment Variables   (Button)

------------------------------------------------------------------------------------
End of File.
