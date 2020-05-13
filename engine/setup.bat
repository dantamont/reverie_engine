REM Check for new source files
call utils\generate_pri.bat

REM Set environment variables for dependent libraries
REM call C:\Users\dante\Documents\Projects\grand-blue-engine\third_party\PythonQt\set_python_paths.bat || echo Failed to set environmental variables for PythonQt, Exit Code is %errorlevel%

REM Build main app with visual studio make and qmake
REM && Runs second command when first is successful, and || runs the second command when the previous is unsuccessful
REM x86 specifies a 32-bit build architecture, can target for x86 | amd64 | x86_amd64 | x86_arm | x86_arm64 | amd64_x86 | amd64_arm | amd64_arm64
call C:\"Program Files (x86)"\"Microsoft Visual Studio"\2017\Community\VC\Auxiliary\Build\vcvarsall.bat x86 && cd C:\Users\dante\Documents\Projects\grand-blue-engine\grand_blue && call c:\Qt\5.12.2\msvc2017\bin\qmake -r -tp vc || echo qmake failed, Exit Code is %errorlevel% 

echo /////////////////////////////////////////////////////////////////////////////////////////////////////////////
echo Built main project
echo /////////////////////////////////////////////////////////////////////////////////////////////////////////////

REM Build tests with visual studio make and qmake
cd C:\Users\dante\Documents\Projects\grand-blue-engine\tests
call run_qmake_tests.bat

echo /////////////////////////////////////////////////////////////////////////////////////////////////////////////
echo Built test project
echo /////////////////////////////////////////////////////////////////////////////////////////////////////////////

cmd /k

REM See https://doc.qt.io/qtvstools/qtvstools-managing-projects.html