REM Create a virtual environment
REM Takes in architecture, python version, and name of venv, so x64, 38, deployment, for example
REM https://newbedev.com/determine-if-python-is-running-inside-virtualenv
REM https://stackoverflow.com/questions/59838/how-can-i-check-if-a-directory-exists-in-a-bash-shell-script

REM Define paths for convenience
set "kindler_dir=%REVERIE_DEV_PATH%/tools/kindler"
set "env_dir=%kindler_dir%/venvs/envs"
set "reqs_dir=%kindler_dir%/venvs/requirements"
set "py_version=%2"
set "arch=%1"
set "venv_name=%3"
set "reqs_file=%reqs_dir%/%venv_name%_%py_version%_%arch%.txt"

REM Create directory for virtual environments if it does not exist
IF NOT EXIST "%env_dir%/" (
    mkdir "%env_dir%/"
)

IF EXIST "%env_dir%/%venv_name%_%py_version%_%arch%/" ( 
    echo %venv_name% environment already exists, checking package dependencies
) ELSE ( 
    REM Create environment
    cd "%env_dir%"
    call "%kindler_dir%/scripts/create_env.bat" %arch% %py_version% %venv_name% 
)

REM Install pip within virtual environment
cd "%env_dir%/%venv_name%_%py_version%_%arch%/Scripts"
. activate
python -m pip install --upgrade pip

REM Use pip to install required packages, if a config exists
if EXIST "%reqs_file%" python -m pip install -r "%reqs_file%"
. deactivate

cmd /k