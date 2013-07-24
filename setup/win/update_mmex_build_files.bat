REM --------------------------------------------------------------------------
REM Author : Stefano Giorgio - Copyright (C) 2012 
REM Purpose: To allow the easy collection of support files required for
REM          - testing in the msw-vc-2012e environment.
REM          - providing a release version for others.
REM 
REM Revision of last commit: $Revision$
REM Author   of last commit: $Author$
REM --------------------------------------------------------------------------
@echo off
cls

REM To create a release configuration, create the following directories:
REM - trunk\mmex_release
REM - trunk\mmex_release\mmex_0.9.9.2_win32_portable
REM - When creating actual releases, rename the directory to the correct version number

set mmex_release_destination=..\..\..\mmex_release\mmex_0.9.9.2_win32_portable
set mmex_build_location=..\..\build\msw-vc-2012e
set mmex_release_type=vc-static-u
set mmex_release_source=%mmex_build_location%\%mmex_release_type%

@echo ------------------------------------------------------------------------
@echo MMEX Support Files Updating Facility
@echo.
@set display_message=Update Build Configurations.
if not exist %mmex_release_destination% goto display_config_continue
@set display_message=Update Build and Release Configurations.
@echo.
@echo Release Configuration Setup:
@echo      Source: %mmex_release_source%
@echo Destination: %mmex_release_destination%
:display_config_continue
@echo.
@echo %display_message%
@echo ------------------------------------------------------------------------
pause
cls

REM Starts with Unicode Release
goto update_u

REM The routine: UpdateFiles
REM will collect the files for all configurations to the specified location.
:UpdateFiles
set mmex_build_dir=%mmex_build_location%\%current_location%
if not exist %mmex_build_dir% goto skip_this_location
@echo ------------------------------------------------------------------------
@echo Updating MMEX Files for: %mmex_build_dir%
@echo.
@echo Copying Root files for: %current_location%
@echo ------------------------------------------------------------------------
copy "..\..\doc\*.txt" %mmex_build_dir%
copy "..\..\readme.*"  %mmex_build_dir%
@echo.
@echo ------------------------------------------------------------------------
@echo Copying Help files for: %current_location%
@echo ------------------------------------------------------------------------
REM del "%mmex_build_dir%\help\*.* /S"
REM create the directories if they don't exist
if not exist %mmex_build_dir%\help mkdir %mmex_build_dir%\help
copy "..\..\doc\help\*.*"         "%mmex_build_dir%\help"
@echo.
if not exist %mmex_build_dir%\help\french mkdir %mmex_build_dir%\help\french
copy "..\..\doc\help\french\*.*"  "%mmex_build_dir%\help\french"
@echo.
if not exist %mmex_build_dir%\help\german mkdir %mmex_build_dir%\help\german
copy "..\..\doc\help\german\*.*"  "%mmex_build_dir%\help\german"
@echo.
if not exist %mmex_build_dir%\help\hungarian mkdir %mmex_build_dir%\help\hungarian
copy "..\..\doc\help\hungarian\*.*"  "%mmex_build_dir%\help\hungarian"
@echo.
if not exist %mmex_build_dir%\help\italian mkdir %mmex_build_dir%\help\italian
copy "..\..\doc\help\italian\*.*" "%mmex_build_dir%\help\italian"
@echo.
if not exist %mmex_build_dir%\help\polish mkdir %mmex_build_dir%\help\polish
copy "..\..\doc\help\polish\*.*"  "%mmex_build_dir%\help\polish"
@echo.
if not exist %mmex_build_dir%\help\russian mkdir %mmex_build_dir%\help\russian
copy "..\..\doc\help\russian\*.*" "%mmex_build_dir%\help\russian"
@echo.
if not exist %mmex_build_dir%\help\spanish mkdir %mmex_build_dir%\help\spanish
copy "..\..\doc\help\spanish\*.*" "%mmex_build_dir%\help\spanish"
@echo.
@echo ------------------------------------------------------------------------
@echo Copying Language files for: %current_location%
@echo ------------------------------------------------------------------------
if not exist %mmex_build_dir%\po mkdir %mmex_build_dir%\po
copy "..\..\po\*.mo" "%mmex_build_dir%\po"
@echo.
@echo ------------------------------------------------------------------------
@echo Copying Resources files for: %current_location%
@echo ------------------------------------------------------------------------
if not exist %mmex_build_dir%\res mkdir %mmex_build_dir%\res
copy "..\..\resources\kaching.wav"       "%mmex_build_dir%\res"
copy "..\..\resources\mmex.ico"          "%mmex_build_dir%\res"
@echo ------------------------------------------------------------------------
@echo.
@echo Updated Support Files for: %mmex_build_dir%
@echo.
pause
cls

:skip_this_location
REM Work out where to go next.
if %location%==vc-static-u          goto update_ud
if %location%==vc-static-ud         goto update_ud_tests
goto ScriptEnd
REM -------------------------------------------------------------------------- 

REM Unicode Release
:update_u
set location=vc-static-u
set current_location=%location%
goto UpdateFiles

REM Unicode Debug
:update_ud
set location=vc-static-ud
set current_location=%location%
goto UpdateFiles

REM Unicode Debug Tests
:update_ud_tests
set location=tests\vc-static-ud
set current_location=%location%
goto UpdateFiles

REM Update the release
:update_release
REM Update the exe files first
if not exist %mmex_release_destination% goto ScriptEnd
@echo --------------------------------------------------------------------
@echo Updating MMEX Release
@echo.
@echo To Destination: %mmex_release_destination%
@echo From Source: %mmex_release_source%
@echo --------------------------------------------------------------------
pause
cls

@echo --------------------------------------------------------------------
@echo Updating MMEX Release
@echo.
@echo To Destination: %mmex_release_destination%
@echo From Source: %mmex_release_source%
@echo.
@echo --------------------------------------------------------------------
REM Set up the release location. Create locations if not exist
set location=MoneyManagerEX
set mmex_release_dir=%mmex_release_destination%\%location%
if not exist %mmex_release_dir% mkdir %mmex_release_dir%

set mmex_release_bin_dir=%mmex_release_dir%\bin
if not exist %mmex_release_bin_dir% mkdir %mmex_release_bin_dir%

REM set up the executable files
copy %mmex_release_source%\mmex.exe %mmex_release_bin_dir%
copy "C:\Windows\sysWOW64\msvcp100.dll" %mmex_release_bin_dir%
copy "C:\Windows\sysWOW64\msvcr100.dll" %mmex_release_bin_dir%

REM Set up the support files before ending process.
set mmex_build_location=%mmex_release_destination%
set current_location=%location%
goto UpdateFiles

:ScriptEnd
@echo.
@echo Update completed.
@echo ------------------------------------------------------------------------
pause
