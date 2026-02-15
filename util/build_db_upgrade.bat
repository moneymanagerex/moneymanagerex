@echo off
REM --------------------------------------------------------------------------
REM Author : Gabriele-V
REM Purpose: To allow easy installation of _TableUpgrade.h
REM --------------------------------------------------------------------------
@echo.
set buildfile_name=sqliteupgrade2cpp.py
set buildfile_location=../database/incremental_upgrade

set actual_location=%buildfile_location%
if not exist %actual_location% set actual_location=../%buildfile_location%
@echo on
python %buildfile_name% %actual_location%
@echo off
if not %buildfile_location%==%actual_location% goto continue
@echo copy _TableUpgrade.h files to src/table
pause
copy _TableUpgrade.h ..\src\table
del _TableUpgrade.h
@echo -------------------------------------------
:continue
@echo Build Completed for %buildfile_name%.
pause
