xcopy /Y /E /I ..\..\resources\*.wav %1res
xcopy /Y /E /I ..\..\resources\*.css %1res
xcopy /Y /E /I ..\..\resources\*.htt %1res
copy /Y ..\..\3rd\ChartNew.js\ChartNew.js %1res
copy /Y ..\..\3rd\sorttable.js\sorttable.js %1res
IF NOT EXIST %1po mkdir %1po
xcopy /Y /E /I ..\..\po\*.mo %1po\en
copy /Y ..\..\doc\contrib.txt %1
IF NOT EXIST %1help xcopy /Y /E /I ..\..\doc\help\*.* %1help
If errorlevel 1 @exit 0
