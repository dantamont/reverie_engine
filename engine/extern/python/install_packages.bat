REM Takes in architecture as a command line argument, e.g. x86 or x64

REM Ensure that the correct environmental variables are set
call .\set_python_paths.bat %1 %2

REM Ensure that pip runs for the correct python instance
REM See: https://stackoverflow.com/questions/43434028/how-to-pip-install-64-bit-packages-while-having-both-64-bit-and-32-bit-versions
.\%2\%1\python.exe -m pip install -r requirements.txt

cmd /k