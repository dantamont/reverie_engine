REM \todo Preserve previous python paths in a file so that installation does not break.
REM Currently, these are: C:\Users\dante\Anaconda3\Lib and C:\Users\dante\Anaconda3\Lib\site-packages
REM site-packages like numpy

rem Take current dir
set "crt_dir=%~dp0"

rem Go up one level
for %%I in ("%crt_dir%\..") do set "root=%%~fI"

rem Construct python paths
set "py_version=%2"
set "arch=%1"
set "pypath=%root%\python\%py_version%\%arch%"
set "pylibpath=%pypath%\libs"

rem Set PYTHON_PATH environmental variable permanently
REM setx PYTHONPATH %pypath%;%pylibpath%;%pypath%\Lib;%pypath%\Lib\site-packages;%pypath%\DLLs
setx PYTHONPATH %pypath%\Lib;%pypath%\Lib\site-packages;

REM cmd /k