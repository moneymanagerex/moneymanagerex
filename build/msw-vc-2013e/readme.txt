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
   
2. Create environment variables for wxWidgets. (See table below)
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

   The IDE build locations will be updated with the required support files to
   successfully run MMEX within the MS VC++ 2013e IDE.

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

Obtaining the Source Code from https://github.com
=================================================
This can be done in 2 ways.
1. Directly from https://github.com/moneymanagerex/moneymanagerex and using the 
   Download Zip link
2. Using tools such as Git for Windows or TortoiseGit.
   For this you will need to create an account on GitHub if you don't have one.
   Using this option allows the user to contribute source code changes directly.   

Downloading source code using GitHub and TortoiseGit
====================================================
1. Install GitHub for Windows and/or Git
   Both versions have a Git implementation which is required for TortoiseGit. 

2. Install TortoiseGit.
   TortoiseGit will need the location of Git if it cannot determine where it is.

3. Create a directory/folder where the MMEX repository will reside.

4. Using Windows Explorer, right-mouse-click on your new folder and
   use the TortoiseGit Commmand:
   Git Clone...
   
   Dialog box info:
   URL: https://github.com/moneymanagerex/moneymanagerex.git
   Directory: This will be automatically provided by TortoiseGit
   Check Box Select: Recursive
   Select button: OK
   
   This will now download the repository to your machine, including all submodules
   used by moneymanagerex.
   
Using GitHub to submit changes.
===============================
For this you will need your active GitHub account.

1. Using the GitHub MoneyManagerEx page, via your Internet Web browser,
   Select the option: Fork

   This will create a copy of the repository to your own local account.
  
2. Change the settings of your local MMEX repository as follows:
   Use the TortoiseGit Commmand:
   Settings...
  
3. In the dialog navigation tree, select: Git -> Remote
   You should see a remote location: **origin**
   Select it to reveal the URL as set up above.

4. Add a new remote location: upstream
   This will point to the same URL
   
5. Change the URL for origin to your username:
   origin: https://github.com/YOUR_GITHUB_USERNAME/moneymanagerex.git
   upstream: https://github.com/moneymanagerex/moneymanagerex.git    

Using Git terminology, you will now be able to pull changes from upstream,
and push the changes to your origin repository on Github. Using the web interface,
you can now create a pull request. This will submit your proposed changes to MMEX.
------------------------------------------------------------------------------------
End of File.
