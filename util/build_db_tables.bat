@echo off
REM --------------------------------------------------------------------------
REM Author : Stefano Giorgio
REM Purpose: To allow easy installation of *Table.* files
REM --------------------------------------------------------------------------
set buildfile_name=sqlite2cpp.py
set buildfile_location=../database/tables.sql
@echo Generated table files for MMEX
@echo.
python %buildfile_name% %buildfile_location% 
@echo.
@echo Confirm moving the generated files to correct the location
@echo.
@pause
@cls
@echo Generated files moved to correct location
@echo.
copy *Table.* ..\src\table
@echo.
copy patch*.sql ..\database
del *Table.*
del patch*.sql
@echo -------------------------------------------
@echo %buildfile_name% installation completed.
@echo.
@pause
