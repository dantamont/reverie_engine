REM Requires a cmake install that is on the system path
REM Also requires qt 5.15 at the correct directory, which is tricky to find! This link may or may not work:
REM https://www.qt.io/download-qt-installer?hsCtaTracking=99d9dd4f-5681-48d2-b096-470725510d34%7C074ddad0-fdef-4e53-8aa8-5e8a876d6ab4
REM Run cmake with the appropriate generator
REM Switch may not be necessary with generator variable set:
REM https://stackoverflow.com/questions/28350214/how-to-build-x86-and-or-x64-on-windows-from-command-line-with-cmake
:: https://cmake.org/cmake/help/git-stage/generator/Visual%20Studio%2016%202019.html
call cmake -B"./builds/x64" -S"./" -A x64 || echo CMake build may have failed (sorry), Exit Code is %errorlevel% .

echo ////////////////////////////////////////////////////////////////////////
echo Built main project
echo ////////////////////////////////////////////////////////////////////////

cmd /k