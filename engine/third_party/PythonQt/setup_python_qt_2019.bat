rem Build PythonQt
rem Fairly sure this is the same command as vcvars32.bat
rem https://docs.microsoft.com/en-us/cpp/build/building-on-the-command-line?view=vs-2019
rem call createSolution.bat
call C:\"Program Files (x86)"\"Microsoft Visual Studio"\2019\Community\VC\Auxiliary\Build\vcvarsall.bat x86 && call c:\Qt\5.12.2\msvc2017\bin\qmake && call C:\"Program Files (x86)"\"Microsoft Visual Studio"\2019\Community\VC\Tools\MSVC\14.16.27023\bin\Hostx86\x86\nmake /f Makefile || echo Build of PythonQt failed, Exit Code is %errorlevel% 

cmd /k