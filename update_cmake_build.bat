@echo off
REM --------------------------------------------------------------------------
REM Author : Stefano Giorgio [stef145g] - Copyright (C) 2017
REM 
REM Purpose: Collection of MMEX support files in CMake build locations.
REM          Allows MMEX to be run successfilly within MSVC 2015 IDE
REM --------------------------------------------------------------------------

REM Current Build Types: Debug, Release
set cmake_build_location=.\build
set current_build_type=Debug
set pause_delay=60

REM --------------------------------------------------------------------------  
REM This will collect the required files for existing build locations.
REM --------------------------------------------------------------------------  
:Update_Files
set mmex_build_dir=%cmake_build_location%\%current_build_type%
if not exist %mmex_build_dir% mkdir %mmex_build_dir%
@echo.
@echo ------------------------------------------------------------------------
@echo Collect MMEX Support files for: %mmex_build_dir%
timeout /t %pause_delay%
@echo.
@echo Copy Root files for: %current_build_type%
@echo ------------------------------------------------------------------------
copy ".\doc\*.txt" %mmex_build_dir%
copy ".\readme.*"  %mmex_build_dir%
@echo.
@echo ------------------------------------------------------------------------
@echo Updating Help files for: %current_build_type%
@echo ------------------------------------------------------------------------
if not exist %mmex_build_dir%\help mkdir %mmex_build_dir%\help
xcopy /s ".\docs\*.*"                   "%mmex_build_dir%\help"
@echo.
@echo ------------------------------------------------------------------------
@echo Copying Language files for: %current_build_type%
@echo ------------------------------------------------------------------------
if not exist %mmex_build_dir%\po mkdir %mmex_build_dir%\po
if not exist %mmex_build_dir%\po\en mkdir %mmex_build_dir%\po\en
copy ".\po\*.mo" "%mmex_build_dir%\po\en"
rem copy ".\po\*.po" "%mmex_build_dir%\po"
@echo.
@echo ------------------------------------------------------------------------
@echo Copying Resources files for: %current_build_type%
@echo ------------------------------------------------------------------------
if not exist %mmex_build_dir%\res mkdir %mmex_build_dir%\res
copy ".\resources\kaching.wav"         "%mmex_build_dir%\res"
copy ".\resources\mmex.ico"            "%mmex_build_dir%\res"
copy ".\resources\master.css"          "%mmex_build_dir%\res"
copy ".\resources\home_page.htt"       "%mmex_build_dir%\res"
copy ".\3rd\ChartNew.js\ChartNew.js"   "%mmex_build_dir%\res"
copy ".\3rd\sorttable.js\sorttable.js" "%mmex_build_dir%\res"

@echo ------------------------------------------------------------------------
@echo Creating CMake files for: %current_build_type%
@echo ------------------------------------------------------------------------
@echo cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=%current_build_type% ../../ > %mmex_build_dir%\cmake_init.bat

REM Runtime files for x64 - MSCV_2015
rem copy "C:\Windows\system32\msvcp140.dll"     %mmex_build_dir%
rem copy "C:\Windows\system32\vcruntime140.dll" %mmex_build_dir%
timeout /t %pause_delay%
cls

REM Work out what to do next.
:Next_Update
REM Continue from already processed, Debug
if %current_build_type%==Debug  goto Update_Release
goto Script_End

:Update_Release
set current_build_type=Release
goto Update_Files

:Script_End
@echo.
@echo Update completed.
@echo ------------------------------------------------------------------------
timeout /t %pause_delay%
