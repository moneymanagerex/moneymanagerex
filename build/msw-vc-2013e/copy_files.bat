xcopy /Y /E /I ..\..\resources\*.wav %1res
xcopy /Y /E /I ..\..\resources\*.css %1res
xcopy /Y /E /I ..\..\resources\*.htt %1res
copy /Y ..\..\3rd\Chart.js\Chart.js %1res
copy /Y ..\..\3rd\sorttable.js\sorttable.js %1res
xcopy /Y /E /I ..\..\po\*.mo %1po\en
copy /Y ..\..\doc\contrib.txt %1
rem xcopy /Y /E /I ..\..\doc\help\*.* %1help
