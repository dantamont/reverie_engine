rem Take current dir
set "crt_dir=%~dp0"

rem Go up one level
rem for %%I in ("%crt_dir%\..") do set "root=%%~fI"

rem Construct  paths
set "win32=%crt_dir%\bin\Win32"
set "win64=%pypath%\bin\Win64"

rem Add to system path (note, does not set in current session, need to close terminal to see changes)
setx PATH %PATH%;%win32%;%win64%

rem see if setting system path works for finding DLLs
rem https://stackoverflow.com/questions/17240725/setx-doesnt-append-path-to-system-path-variable
rem https://stackoverflow.com/questions/19335004/how-to-run-a-powershell-script-from-a-batch-file

rem Append dll directories to system PATH environmental variable
rem Note that %path% combines the system and user paths, so there will be duplication :(
rem set "pythonqt_dll_path=%pythonqtpath%\lib"
rem echo %path%; | find /i "%pythonqt_dll_path%;" >NUL || set /m "temp_path=%path%;%pythonqt_dll_path%"
rem echo %path%; | find /i "%pypath%;" >NUL || setx path %temp_path%;%pypath%
cmd /k