@echo off
chcp 65001
set log=%temp%\update_cmake_build_for_msv.log
REM --------------------------------------------------------------------------
REM Author : Stefano Giorgio [stef145g] - Copyright (C) 2017
REM 
REM Purpose: To collect all the MMEX support files required to run MMEX.EXE
REM          within MS Visual Studio IDE on completion of an MSVC build.
REM --------------------------------------------------------------------------

REM Current Build Types: Debug, Release, MinSizeRel
set cmake_build_location=.\bin
set current_build_type=Debug

REM --------------------------------------------------------------------------  
REM This will collect the required files for existing build locations.
REM --------------------------------------------------------------------------  
:Update_Files
set mmex_build_dir=%cmake_build_location%\%current_build_type%
if not exist %mmex_build_dir% goto Next_Update
@echo ======================================================================== >>%log%
@echo Collect MMEX Support files for: %mmex_build_dir% >>%log%
@echo ======================================================================== >>%log%
@echo. >>%log%
@echo Copy Root files for: %current_build_type% >>%log%
@echo ------------------------------------------------------------------------ >>%log%
copy ".\doc\*.txt" %mmex_build_dir% >>%log%
copy ".\readme.*"  %mmex_build_dir% >>%log%
@echo. >>%log%
@echo ------------------------------------------------------------------------ >>%log%
@echo Updating Help files for: %current_build_type% to: %mmex_build_dir%\help >>%log%
@echo ------------------------------------------------------------------------ >>%log%
if not exist %mmex_build_dir%\help mkdir %mmex_build_dir%\help
xcopy /Y /I /S ".\docs\*.*"          "%mmex_build_dir%\help" >>%log%
@echo. >>%log%
@echo ------------------------------------------------------------------------ >>%log%
@echo Copying Language files for: %current_build_type% to: "%mmex_build_dir%\po\en" >>%log%
@echo ------------------------------------------------------------------------ >>%log%
if not exist %mmex_build_dir%\po mkdir %mmex_build_dir%\po
if not exist %mmex_build_dir%\po\en mkdir %mmex_build_dir%\po\en
copy ".\po\*.mo" "%mmex_build_dir%\po\en" >>%log%
rem copy ".\po\*.po" "%mmex_build_dir%\po"
@echo. >>%log%
@echo ------------------------------------------------------------------------ >>%log%
@echo Copying Resources files for: %current_build_type% to: "%mmex_build_dir%\res" >>%log%
@echo ------------------------------------------------------------------------ >>%log%
if not exist %mmex_build_dir%\res mkdir %mmex_build_dir%\res
@echo copy ".\resources\kaching.wav"   "%mmex_build_dir%\res" >>%log%
copy ".\resources\kaching.wav"         "%mmex_build_dir%\res" >>%log%
@echo copy ".\resources\mmex.ico"      "%mmex_build_dir%\res" >>%log%
copy ".\resources\mmex.ico"            "%mmex_build_dir%\res" >>%log%
@echo copy ".\resources\master.css"    "%mmex_build_dir%\res" >>%log%
copy ".\resources\master.css"          "%mmex_build_dir%\res" >>%log%
@echo copy ".\resources\home_page.htt" "%mmex_build_dir%\res" >>%log%
copy ".\resources\home_page.htt"       "%mmex_build_dir%\res" >>%log%
@echo copy ".\3rd\ChartNew.js\ChartNew.js"      "%mmex_build_dir%\res" >>%log%
copy ".\3rd\ChartNew.js\ChartNew.js"            "%mmex_build_dir%\res" >>%log%
@echo copy ".\3rd\sorttable.js\sorttable.js"    "%mmex_build_dir%\res" >>%log%
copy ".\3rd\sorttable.js\sorttable.js"          "%mmex_build_dir%\res" >>%log%
@echo ------------------------------------------------------------------------ >>%log%
@echo Completed for: %current_build_type% >>%log% 
@echo ======================================================================= >>%log%
@echo. >>%log%
@echo. >>%log%
@echo. >>%log%

REM Runtime files for x64 - MSCV_2015 - Not needed in a build environment
rem copy "C:\Windows\system32\msvcp140.dll"     %mmex_build_dir%
rem copy "C:\Windows\system32\vcruntime140.dll" %mmex_build_dir%

REM Work out what to do next.
:Next_Update
REM Continue from already processed, Debug
if %current_build_type%==Debug      goto Update_Release
if %current_build_type%==Release    goto Update_MinSizeRel
goto Script_End

:Update_Release
set current_build_type=Release
goto Update_Files

:Update_MinSizeRel
set current_build_type=MinSizeRel
goto Update_Files

:Script_End
@echo. >>%log%
@echo Total update: Completed. >>%log%
@echo ======================================================================= >>%log%
notepad %log%
del %log%
