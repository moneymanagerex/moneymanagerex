REM --------------------------------------------------------------------------
REM Author : Stefano Giorgio - Copyright (C) 2012 
REM Purpose: To allow the easy collection of support files required for
REM          - testing in the msw-vc-2013e environment.
REM          - providing a release version for others.
REM 
REM Revision of last commit: $Revision$
REM Author   of last commit: $Author$
REM --------------------------------------------------------------------------
@echo off
cls

REM - When creating subsequent releases, rename the variable: mmex_release_version
REM   to reflect the correct version location
set mmex_release_version=mmex_1.0.0.0_win32_portable
set mmex_release_type=release

set mmex_release_destination=..\..\mmex_release\%mmex_release_version%
set mmex_build_location=..\..\build\msw-vc-2013e
set mmex_release_source=%mmex_build_location%\%mmex_release_type%

@echo ------------------------------------------------------------------------
@echo MMEX Support Files Updating Facility
@echo.
if exist %mmex_release_destination% goto continue_intro
@echo To collect appropriate files for a System Release,
@echo create the directorys:-
@echo.
@echo - trunk\mmex_release
@echo - trunk\mmex_release\%mmex_release_version%

:continue_intro
@set display_message=Update IDE Build Configurations.
if not exist %mmex_release_destination% goto display_config_continue
@set display_message=Update IDE Build Configurations and MMEX Release Location.
@echo.
@echo MMEX Release Configuration Setup:
@echo      Source: %mmex_release_source%
@echo Destination: %mmex_release_destination%
:display_config_continue
@echo.
@echo %display_message%
@echo ------------------------------------------------------------------------
pause
cls

REM Starts with Local release
goto update_release

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
if not exist %mmex_build_dir%\po\en mkdir %mmex_build_dir%\po\en
copy "..\..\po\*.mo" "%mmex_build_dir%\po\en"
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
if %location%==release              goto update_debug
if %location%==debug                goto update_debug_tests
if %location%==tests\debug          goto update_main_release
goto ScriptEnd
REM -------------------------------------------------------------------------- 

REM Unicode Release
:update_release
set location=release
set current_location=%location%
goto UpdateFiles

REM Unicode Debug
:update_debug
set location=debug
set current_location=%location%
goto UpdateFiles

REM Unicode Debug Tests
:update_debug_tests
set location=tests\debug
set current_location=%location%
goto UpdateFiles

REM Update the release
:update_main_release
REM Update the exe files first
if not exist %mmex_release_destination% goto ScriptEnd
@echo --------------------------------------------------------------------
@echo Updating MMEX Release Location
@echo.
@echo Destination: %mmex_release_destination%
@echo From Source: %mmex_release_source%
@echo --------------------------------------------------------------------
pause
cls

@echo --------------------------------------------------------------------
@echo Updating MMEX System Release
@echo.
@echo Destination: %mmex_release_destination%
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
copy "C:\Windows\sysWOW64\msvcp120.dll" %mmex_release_bin_dir%
copy "C:\Windows\sysWOW64\msvcr120.dll" %mmex_release_bin_dir%

REM Set up the support files before ending process.
set mmex_build_location=%mmex_release_destination%
set current_location=%location%
goto UpdateFiles

:ScriptEnd
@echo.
@echo Update completed.
@echo ------------------------------------------------------------------------
pause
