@echo off
call :treeProcess
pause
goto :eof

:treeProcess
rem Do whatever you want here over the files of this subdir, for example:
for %%f in (..\src\*.cpp ..\src\*.h) do cscript //nologo checkEOL.vbs %%f
for /D %%d in (*) do (
    cd %%d
    call :treeProcess
    cd ..
)
exit /b
