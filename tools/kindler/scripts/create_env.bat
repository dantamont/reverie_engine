REM Create a virtual environment
REM Takes in architecture, python version, and name of venv, so x64, 38, deployment, for example
REM cd ../../extern/python
"%REVERIE_DEV_PATH%/extern/python/%2/%1/python.exe" -m venv %3_%2_%1