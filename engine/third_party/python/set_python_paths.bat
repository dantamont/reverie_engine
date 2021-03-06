REM TODO: Make this an explicit part of setup process. PYTHONPATH is where python looks for
REM site-packages like numpy

rem Take current dir
set "crt_dir=%~dp0"

rem Go up one level
for %%I in ("%crt_dir%\..") do set "root=%%~fI"

rem Construct python paths
set "py_version=38"
set "arch=%1"
set "pypath=%root%\python\%py_version%\%arch%"
set "pylibpath=%pypath%\libs"

rem Set PYTHON_PATH environmental variable permanently
REM setx PYTHONPATH %pypath%;%pylibpath%;%pypath%\Lib;%pypath%\Lib\site-packages;%pypath%\DLLs
setx PYTHONPATH %pypath%\Lib;%pypath%\Lib\site-packages;

REM cmd /k