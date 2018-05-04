@echo off
REM --------------------------------------------------------------------------
REM Author : Gabriele-V
REM Purpose: To allow easy installation of DB_Upgrade.h
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
@echo copy DB_Upgrade.h files to src/db
pause
copy DB_Upgrade.h ..\src\db
del DB_Upgrade.h
@echo -------------------------------------------
:continue
@echo Build Completed for %buildfile_name%.
pause
