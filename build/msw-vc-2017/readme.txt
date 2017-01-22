Build instructions for MS Windows platform
===================================================================================
Microsoft Visual Studio Community 2017 C++ compiler
-----------------------------------------------------------------------------------

NOTE:
    The directory msw-vc-2017 has the project files for:
    Microsoft Visual Studio Community C++ 2017.
    For other compilers, see the readme file in the mmex/build/msw directory.

Prerequisites
-----------------------------------------------------------------------------------
1. Installation of the following:
   MS-VC 2017         https://www.visualstudio.com/downloads/
   wxWidgets source   https://github.com/wxWidgets/wxWidgets
   Source code        https://github.com/moneymanagerex/moneymanagerex
   
2. Create environment variables for wxWidgets. (See table below)
-----------------------------------------------------------------------------------

Optional software installation:
===============================
   GitHub Windows   https://windows.github.com/

   Git              https://git-for-windows.github.io/
   TortoiseGit      https://tortoisegit.org/download/
                    A Windows interface to GitHub to download the source code.
  
   Doxygen          http://www.stack.nl/~dimitri/doxygen/download.html
                    Allows documentation to be obtained from the source code.
   PoEdit           http://www.poedit.net/
                    Allows the generation of .po and .mo files
                    required for language translations. 

To build the setup installation, download and install InnoSetup Quick Start Pack.
   This includes InnoSetup itself and Inno Setup Preprocessor. Append the folder
   path to the Windows Environment Variable: PATH for the installed software.
   (something like a "C:\Program Files\Inno Setup 5")

Compiling using Microsoft Visual Studio Community C++ 2017 IDE
-----------------------------------------------------------------------------------
Note: wxWidgets must be built using MSVS 2017, to ensure a successful build use:
      * wxWidgets Configuration	wx_vc15.sln or
	  * wxWidgets Configuration	wx_vc14.sln after project files upgraded by VC 2017

1. Set up the windows configuration as per the table below
	  
2. With Git and TortoiseGit installed, using Windows Explorer
   a) Create a folder to contain wxWidgets.
      * eg: C:\Git
   b) Select this folder and use: Git clone
      * URL: https://github.com/wxwidgets/wxwidgets.git
	    This will place wxWidgets in C:\Git\wxWidgets
	  * Rename \wxWidgets to \wxWidgets_2017 if necessary to identify the build
   c) Using Git Show Log, create a branch at tag: v3.1.0
      * b_v3.1.0
      * Switch to this branch. (Master may also be used)
   d) Set up setup.h
      * cmd: copy include\wx\msw\setup0.h include\wx\msw\setup.h

3. Using MSVS 2017, select the project file in the wxWidgets folder.
   (.../build/msw/wx_vc15.sln) - Allow to upgrade project files 

   Build the appropriate configurations for your needs.
   Recommended configurations for mmex: Release, Debug

4. With Git and TortoiseGit installed, using Windows Explorer,
   a) Using the same c:\Git location use Git Clone to obtain moneymanagerex
      * Dialog box info:
      * URL: https://github.com/moneymanagerex/moneymanagerex.git
        Directory: This will be automatically provided by TortoiseGit
      * Check Box Select: Recursive
      * Select button: OK
5. Using MSVS 2017, locate the directory: moneymanagerex\build\msw-vc-2017
   a) Open the project file: moneymanagerex\build\msw-vc-2017\mmex.sln
      MSVS 2017 will set up the project files to the user's environment.
   
6  In the Solution Explorer set the mmex project as the Startup Project
   
7. Using the Build ->configurations Manager...
   Select one of following configurations to build as per wxWidgets builds:
   Release  <-- Best choice for most people.
   Debug    <-- Extra debug code added for debugging and may be slower.

8. On build completion, locate and Run the batch file: update_mmex_build_files.bat
   (Only need to be run once to install help files)
   location: moneymanagerex\setup\win\update_mmex_build_files.bat

   The IDE build locations will be updated with the required support files to
   successfully run MMEX within the MSVS 2017 IDE.

If you wish to contribute to the project, you will need to:
   a) Create GitHub account if you don't have one already
   b) Using GitHub commands, use FORK to obtain a copy of the master project
   C) This allows the user to contribute source code changes via a Pull Request.
   
Windows Configuration
=====================

Environment variables need to access wxWidgets builds on a Windows system.
------------------------------------------------------------------------------------
Variable  | Value                    | Comment
----------|--------------------------|----------------------------------------------
WXWIN     | C:\Git\wxWidgets_2017    | Location of wxWidgets repository


Setting up Environment variables using Windows 7
------------------------------------------------------------------------------------
Control Panel ---> System and Security ---> System
Advanced system settings        (Located on left navigation panel)

System Properties       (Dialog) ---> Advanced  (Tab)
Environment Variables   (Button)

------------------------------------------------------------------------------------


Downloading GitHub and TortoiseGit
====================================================
1. Install GitHub for Windows and/or Git
   Both versions have a Git implementation which is required for TortoiseGit. 

2. Install TortoiseGit.
   TortoiseGit will need the location of Git if it cannot determine where it is.

   
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

Git stores changes as node points using a commit command. These commit points
generally form a single chain. When 2 different commits made from the same initial
node point, we end up in a split in the chain.

When using multiple branches and user repositories the command:
Revision Graph shows the relationship between branches.

To synchronise we can either use a Pull command from the other branch or
we can use a Fetch and Rebase command.

The Pull command will bring the two node points back to a single node creating a
merge commit in the process. The Fetch and Rebase will collect all changes from
our branch and place them on top of the changes from the remote node without a
merge commit.
------------------------------------------------------------------------------------
End of File.
