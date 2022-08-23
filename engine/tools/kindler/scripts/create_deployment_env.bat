REM Create a virtual environment
REM Takes in architecture, python version, and name of venv, so x64, 38, deployment, for example
REM https://newbedev.com/determine-if-python-is-running-inside-virtualenv
REM https://stackoverflow.com/questions/59838/how-can-i-check-if-a-directory-exists-in-a-bash-shell-script

REM Create directory for virtual environments if it does not exist
set "env_dir=../venvs/envs"
set "reqs_dir=../venvs/requirements"
set "py_version=%2"
set "arch=%1"

IF NOT EXIST "%env_dir%/" (
    mkdir "%env_dir%/"
)

IF EXIST "%env_dir%/deployment_%py_version%_%arch%/" ( 
    echo Deployment environment already exists 
) ELSE ( 
    REM Create environment
    call ./create_env.bat %arch% %py_version% deployment 
    
    REM Install pip within virtual environment
    cd "%env_dir%/deployment_%py_version%_%arch%/Scripts"
    . activate
    python -m pip install --upgrade pip
    
    REM Use pip to install required packages
    python -m pip install -r "%reqs_dir%/requirements_%py_version%_%arch%.txt"
    deactivate
)


cmd /k