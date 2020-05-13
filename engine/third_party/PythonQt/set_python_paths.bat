rem Take current dir
set "crt_dir=%~dp0"

rem Go up one level
for %%I in ("%crt_dir%\..") do set "root=%%~fI"

rem Construct python paths
set "pypath=%root%\Python38-32"
set "pylibpath=%pypath%\libs"
set "pythonqtpath=%root%\PythonQt"

rem Set PYTHON_PATH environmental variable permanently
rem It is currently unclear to me if the below line is necessary, but it breaks other python-path dependent applications
setx PYTHONPATH %pypath%;%pylibpath%;%pypath%\Lib;%pypath%\Lib\site-packages;%pypath%\DLLs
setx PYTHON_PATH %pypath%
setx PYTHON_DLL_PATH %pypath%\DLLs
setx PYTHON_LIB %pylibpath%
setx PYTHON_QT_PATH %pythonqtpath%
setx PYTHON_VERSION 38

rem see if setting system path works for finding DLLs
rem https://stackoverflow.com/questions/17240725/setx-doesnt-append-path-to-system-path-variable
rem https://stackoverflow.com/questions/19335004/how-to-run-a-powershell-script-from-a-batch-file

rem Append dll directories to system PATH environmental variable
rem Note that %path% combines the system and user paths, so there will be duplication :(
rem set "pythonqt_dll_path=%pythonqtpath%\lib"
rem echo %path%; | find /i "%pythonqt_dll_path%;" >NUL || set /m "temp_path=%path%;%pythonqt_dll_path%"
rem echo %path%; | find /i "%pypath%;" >NUL || setx path %temp_path%;%pypath%
cmd /k