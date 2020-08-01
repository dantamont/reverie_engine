REM call MSVC for 32 bit with SDK 10.0.17134.0 to set up environment, then call qmake
call C:\"Program Files (x86)"\"Microsoft Visual Studio"\2017\Community\VC\Auxiliary\Build\vcvarsall.bat x86 10.0.17763.0 && call c:\Qt\5.12.2\msvc2017\bin\qmake -tp vc -r PythonQt.pro|| echo qmake on PythonQt failed, Exit Code is %errorlevel% 
cmd /k