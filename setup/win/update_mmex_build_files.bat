REM --------------------------------------------------------------------------
REM Author : Stefano Giorgio [stef145g] - Copyright (C) 2012..2014
REM 		 Updates by:
REM          Lisheng [guanlisheng] (C) 2013
REM          Nikolay [vomikan]     (C) 2013
REM          James [siena123]      (c) 2014
REM          gabriele-v            (c) 2014
REM 
REM Purpose: To allow the easy collection of support files required for
REM          - testing in the msw-vc-2013e environment.
REM          - providing a release version for others.
REM 
REM Revision of last commit: $Revision$
REM Author   of last commit: $Author$
REM --------------------------------------------------------------------------
@echo off
cls

REM Set the variable: mmex_release_version
REM to reflect the correct version.

rem set mmex_release_version=mmex_1.1.0
set mmex_release_version=mmex_1.1.0-rc2

set mmex_system_name=MoneyManagerEX
set mmex_build_location=..\..\build\msw-vc-2013e
set mmex_release_location=..\..\mmex_release
if NOT EXIST %mmex_release_location% md %mmex_release_location%

REM mmex_win_system_type=x64   ... updated later in script.
set mmex_win_system_type=win32
set mmex_build_type=release

@echo ------------------------------------------------------------------------
@echo MMEX Support Files Updating Facility
@echo.

set display_message=Update IDE Build Configurations and MMEX Release Locations.

REM Create a compressed version of the output file for distribution
if exist .\mpress.219\mpress.exe goto display_config_continue
@echo.
@echo The program 'mpress.exe' has not been found in dir: .\mpress.219
@echo.
@echo To distribute compressed versions of: mmex.exe
@echo include the package version of: mpress.219 with this batch file.
@echo.
@echo available from: http://www.matcode.com/mpress.htm

:display_config_continue
@echo.
@echo %display_message%
@echo ------------------------------------------------------------------------
pause
cls

REM Starts with Win32 Release
goto start_update_process

REM --------------------------------------------------------------------------  
REM The routine: UpdateFiles
REM will collect the files for all configurations to the specified location.
REM --------------------------------------------------------------------------  
:UpdateFiles
REM Initially set the location to the mmex_release location
set mmex_build_dir=%mmex_release_location%\%mmex_release_version%_%mmex_win_system_type%_portable\%location%
if %location%==%mmex_system_name% goto UpdateFiles_Continue

rem Reset the build location to the IDE Build location
set mmex_build_dir=%mmex_build_location%\%mmex_win_system_type%\%location%

:UpdateFiles_Continue
set current_location=%mmex_win_system_type%\%location%
if %current_location% == %mmex_win_system_type%\tests\debug set current_location=..\msw_tests-vc-2013e\%mmex_win_system_type%\debug
if %current_location% == ..\msw_tests-vc-2013e\%mmex_win_system_type%\debug set mmex_build_dir=%mmex_build_location%\%current_location%

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
for /f %%d IN ('dir /A:D /B ..\..\doc\help') do (
    if not exist "%mmex_build_dir%\doc\help\%%d" mkdir "%mmex_build_dir%\help\%%d"
    copy "..\..\doc\help\%%d\*.*"  "%mmex_build_dir%\help\%%d"
)
@echo ------------------------------------------------------------------------
@echo Copying Language files for: %current_location%
@echo ------------------------------------------------------------------------
if not exist %mmex_build_dir%\po mkdir %mmex_build_dir%\po
if not exist %mmex_build_dir%\po\en mkdir %mmex_build_dir%\po\en
copy "..\..\po\*.mo" "%mmex_build_dir%\po\en"
copy "..\..\po\*.po" "%mmex_build_dir%\po"
@echo.
@echo ------------------------------------------------------------------------
@echo Copying Resources files for: %current_location%
@echo ------------------------------------------------------------------------
if not exist %mmex_build_dir%\res mkdir %mmex_build_dir%\res
copy "..\..\resources\kaching.wav"     "%mmex_build_dir%\res"
copy "..\..\resources\mmex.ico"        "%mmex_build_dir%\res"
copy "..\..\resources\master.css"      "%mmex_build_dir%\res"
copy "..\..\resources\home_page.htt"   "%mmex_build_dir%\res"
copy "..\..\3rd\Chart.js\Chart.js"     "%mmex_build_dir%\res"
@echo ------------------------------------------------------------------------
@echo.
@echo Updated Support Files for: %mmex_build_dir%
@echo.
pause
cls

:skip_this_location
REM Work out what to do next. Continue from already processed, win32\release
if %current_location%==%mmex_win_system_type%\release       goto update_debug
if %current_location%==%mmex_win_system_type%\debug         goto update_tests_debug
if %current_location%==..\msw_tests-vc-2013e\%mmex_win_system_type%\debug   goto update_release
if %current_location%==win32\%mmex_system_name%             goto system_change_x64
goto ScriptEnd
REM -------------------------------------------------------------------------- 

REM x64 Release
:system_change_x64
set mmex_win_system_type=x64
set location=release
goto UpdateFiles

REM -------------------------------------------------------------------------- 
REM win32 Release - Initial start.
REM -------------------------------------------------------------------------- 
:start_update_process
set location=release
goto UpdateFiles

REM win32/x64 Debug
:update_debug
set location=debug
goto UpdateFiles

REM win32/x64 Tests\Debug
:update_tests_debug
set location=tests\debug
goto UpdateFiles

REM Update the release
:update_release
REM Update the exe files first
set location=%mmex_system_name%
set mmex_release_source=%mmex_build_location%\%mmex_win_system_type%\%mmex_build_type%
set mmex_release_destination=%mmex_release_location%\%mmex_release_version%_%mmex_win_system_type%_portable
if not exist %mmex_release_destination% mkdir %mmex_release_destination%
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
set mmex_release_dir=%mmex_release_destination%\%location%
if not exist %mmex_release_dir% mkdir %mmex_release_dir%

set mmex_release_bin_dir=%mmex_release_dir%\bin
if not exist %mmex_release_bin_dir% mkdir %mmex_release_bin_dir%

REM Create a zero length file for portable version
copy nul %mmex_release_dir%\mmexini.db3

if %mmex_win_system_type%==x64 goto get_x64_dll_files
REM set up the executable files for Win32
copy %mmex_release_source%\mmex.exe %mmex_release_bin_dir%
copy "C:\Windows\sysWOW64\msvcp120.dll" %mmex_release_bin_dir%
copy "C:\Windows\sysWOW64\msvcr120.dll" %mmex_release_bin_dir%
goto update_release_continue

:get_x64_dll_files
REM set up the executable files for x64
copy %mmex_release_source%\mmex.exe %mmex_release_bin_dir%
copy "C:\Windows\system32\msvcp120.dll" %mmex_release_bin_dir%
copy "C:\Windows\system32\msvcr120.dll" %mmex_release_bin_dir%

:update_release_continue
REM Create a compressed version of the output file for distribution
if not exist .\mpress.219\mpress.exe goto UpdateFiles
.\mpress.219\mpress %mmex_release_bin_dir%\mmex.exe

REM Set up the support files before ending process.
goto UpdateFiles

:ScriptEnd
@echo.
@echo Update completed.
@echo ------------------------------------------------------------------------
pause
