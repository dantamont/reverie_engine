REM Activate environment and update requirements.txt with the packages it uses

set "kindler_dir=%REVERIE_DEV_PATH%/tools/kindler"
set "env_dir=%kindler_dir%/venvs/envs"
set "reqs_dir=%kindler_dir%/venvs/requirements"
set "py_version=%2"
set "arch=%1"
set "venv_name=%3"

cd "%env_dir%/%venv_name%_%py_version%_%arch%/Scripts"
. activate
pip freeze > "%reqs_dir%/%venv_name%_%py_version%_%arch%.txt"
deactivate


cmd /k