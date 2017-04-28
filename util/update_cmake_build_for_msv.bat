@echo off
chcp 65001
set log=%temp%\update_cmake_build_for_msv.log
REM --------------------------------------------------------------------------
REM Author : Stefano Giorgio [stef145g] - Copyright (C) 2017
REM 
REM Purpose: Collection of MMEX support files in CMake build locations.
REM          Allows MMEX to be run successfilly within MSVC 2015 IDE
REM --------------------------------------------------------------------------

REM --------------------------------------------------------------------------  
REM This will collect the required files for existing build locations.
REM --------------------------------------------------------------------------  

set cmake_build_location=..\build

REM Current Build Types: Debug, Release
for %%x in (Release Debug) do (
if not exist %cmake_build_location%\%%x mkdir %cmake_build_location%\%%x 
@echo. >>%log%
@echo ======================================================================= >>%log%
@echo. >>%log%
@echo ------------------------------------------------------------------------ >>%log%
@echo Collect MMEX Support files for: %cmake_build_location%\%%x >>%log%
@echo ------------------------------------------------------------------------ >>%log%
@echo Copy Root files for: %%x >>%log%
for /r %%f in (..\*.txt ..\README.*) do (
  @echo Copy %%f to %cmake_build_location%\%%x\ >>%log%
  copy %%f %cmake_build_location%\%%x >NUL
) 

rem copy "..\doc\*.txt" %cmake_build_location%\%%x>NUL
rem copy "..\readme.*"  %cmake_build_location%\%%x>NUL
@echo. >>%log%
@echo ------------------------------------------------------------------------ >>%log%
@echo Updating Help files for: %%x >>%log%
@echo ------------------------------------------------------------------------ >>%log%
(if not exist %cmake_build_location%\%%x\help mkdir %cmake_build_location%\%%x\help)
echo Copy "..\docs\*.*" to "%cmake_build_location%\%%x\help\" >>%log%
xcopy /Y /I /S "..\docs\*.*" "%cmake_build_location%\%%x\help\" >>%log%
@echo. >>%log%
@echo ------------------------------------------------------------------------ >>%log%
@echo Copying Language files for: %%x >>%log%
@echo ------------------------------------------------------------------------ >>%log%
if not exist %cmake_build_location%\%%x\po mkdir %cmake_build_location%\%%x\po
if not exist %cmake_build_location%\%%x\po\en mkdir %cmake_build_location%\%%x\po\en
@echo Copy ".\po\*.mo" to "%cmake_build_location%\%%x\po\en\" >>%log%
copy "..\po\*.mo" "%cmake_build_location%\%%x\po\en" >>%log%
rem copy ".\po\*.po" "%cmake_build_location%\%%x\po"

@echo. >>%log%
@echo ------------------------------------------------------------------------ >>%log%
@echo Copying Resources files for: %%x >>%log%
@echo ------------------------------------------------------------------------ >>%log%
if not exist %cmake_build_location%\%%x\res mkdir %cmake_build_location%\%%x\res
for %%f in (.\resources\kaching.wav .\resources\mmex.ico .\resources\master.css .\resources\home_page.htt .\3rd\ChartNew.js\ChartNew.js .\3rd\sorttable.js\sorttable.js) DO (
@echo Copy "%%f" to "%cmake_build_location%\%%x\res\" >>%log%
copy "%%f"  "%cmake_build_location%\%%x\res" >NUL 
)

@echo. >>%log%
@echo ------------------------------------------------------------------------ >>%log%
@echo Creating CMake batch file for: %%x >>%log%
@echo ------------------------------------------------------------------------ >>%log%
@echo cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=%%x ../../ > %cmake_build_location%\%%x\cmake_init.bat
@echo cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=%%x ../../ >>%log%
@echo Done >>%log%
@echo ------------------------------------------------------------------------ >>%log%

REM Runtime files for x64 - MSCV_2015
rem copy "C:\Windows\system32\msvcp140.dll"     %cmake_build_location%\%%x
rem copy "C:\Windows\system32\vcruntime140.dll" %cmake_build_location%\%%x
)

@echo. >>%log%
@echo Update completed. >>%log%
@echo ======================================================================= >>%log%
notepad %log%
del %log%
