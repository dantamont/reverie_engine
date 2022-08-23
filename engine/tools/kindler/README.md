# Kindler

## Overview
This project contains all of the tools necessary to generate a python virtual environment suitable for the autogeneration of code as well as any other functions performed by Python during the deployment process.

## Installation
Simply run setup.bat, which will create a Python 3.8 virtual environment for x64 windows.

### VS Code setup
See https://techinscribed.com/python-virtual-environment-in-vscode/

## Updates
To update a virtual environment's requirements.txt with any local changes, run scripts/freeze_requirements.bat with the appropriate arguments. Or, simply run the update.bat wrapper script

```
python -m doit -f my_dodo.py
```