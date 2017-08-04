@echo off
REM --------------------------------------------------------------------------
REM Author : Stefano Giorgio
REM Purpose: To allow easy installation of DB_Table_xxx.h files
REM --------------------------------------------------------------------------
set buildfile_name=sqlite2cpp.py
set buildfile_location=../database/tables_v1.sql
@echo Generated DB_Tables files for MMEX
@echo.
python %buildfile_name% %buildfile_location% 
@echo.
@echo Confirm moving the generated files to correct the location
@echo.
@pause
@cls
@echo Generated files moved to correct location
@echo.
copy DB_Table*.* ..\src\db
@echo.
copy currency_table_*.* ..\database
del DB_Table*.*
del currency_table_*.*
@echo -------------------------------------------
@echo %buildfile_name% installation completed.
@echo.
@pause
