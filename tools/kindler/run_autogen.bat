REM Call auto-gen from virtual environment
set "arch=%1"
set "py_version=%2"
set "target_name=%3"
set "output_dir=%4"
echo Autogenerating for architecture: %arch%
echo Python version: %py_version%
echo Target name: %target_name%
echo Output directory: %output_dir%

REM Activate virtual environment
set "kindler_dir=%REVERIE_DEV_PATH%/tools/kindler"
set "env_dir=%kindler_dir%/venvs/envs"
call "%env_dir%/deployment_%py_version%_%arch%/Scripts/activate"

REM Need to add directory to PYTHONPATH so that autogen is found
REM export PYTHONPATH="${PYTHONPATH}:/path/to/your/project/"
REM For Windows, update python path with path to project
set PYTHONPATH=%PYTHONPATH%;%kindler_dir%;%kindler_dir%/autogen

python -m autogen %target_name% %output_dir%

REM Deactivate virtual environment
REM call "%env_dir%/deployment_%py_version%_%arch%/Scripts/deactivate.bat"

REM Will break cmake's execute_process if left in, so only uncomment for debugging
REM cmd /k