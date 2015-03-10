@echo off
REM --------------------------------------------------------------------------
REM Author : Stefano Giorgio
REM Purpose: To allow easy installation of DB_Table_xxx.h files
REM --------------------------------------------------------------------------
@echo.
set buildfile_name=sqlite2cpp.py
set buildfile_location=../database/tables_v1.sql

set actual_location=%buildfile_location%
if not exist %actual_location% set actual_location=../%buildfile_location%
@echo on
python %buildfile_name% %actual_location%
@echo off
if not %buildfile_location%==%actual_location% goto continue
@echo copy DB_*.* files to src/db
pause
copy DB_Table*.* ..\src\db
del DB_Table*.*
@echo -------------------------------------------
:continue
@echo Build Completed for %buildfile_name%.
pause
