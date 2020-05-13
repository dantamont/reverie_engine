REM Check for new source files
call utils\generate_pri.bat

REM Build main app with visual studio make and qmake
call C:\"Program Files (x86)"\"Microsoft Visual Studio"\2019\Community\VC\Auxiliary\Build\vcvarsall.bat x86  && cd C:\Users\dante\Documents\Projects\grand-blue-engine\grand_blue && call c:\Qt\5.12.2\msvc2017\bin\qmake -r -tp vc || echo qmake failed, Exit Code is %errorlevel% )

echo /////////////////////////////////////////////////////////////////////////////////////////////////////////////
echo Built main project
echo /////////////////////////////////////////////////////////////////////////////////////////////////////////////

REM Build tests with visual studio make and qmake
cd C:\Users\dante\Documents\Projects\grand-blue-engine\tests
call run_qmake_tests_msvc2019.bat || echo test qmake failed, Exit Code is %errorlevel% 

echo /////////////////////////////////////////////////////////////////////////////////////////////////////////////
echo Built test project
echo /////////////////////////////////////////////////////////////////////////////////////////////////////////////

PAUSE

REM See https://doc.qt.io/qtvstools/qtvstools-managing-projects.html