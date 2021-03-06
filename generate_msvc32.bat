REM Run cmake with the appropriate generator
:: https://cmake.org/cmake/help/git-stage/generator/Visual%20Studio%2016%202019.html
call cmake -B"./bin/x86" -S"./" -G "Visual Studio 16 2019" -A Win32 || echo qmake failed, Exit Code is %errorlevel% 

echo ////////////////////////////////////////////////////////////////////////
echo Built main project
echo ////////////////////////////////////////////////////////////////////////

cmd /k