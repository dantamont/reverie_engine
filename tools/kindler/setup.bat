REM Set environmental variable to find grand_blue_engine
REM Using regular set so value is used in current cmd window session,
REM and setx so it is set permanently 
set REVERIE_DEV_PATH C:/Users/dante/Documents/Projects/grand-blue-engine
setx REVERIE_DEV_PATH C:/Users/dante/Documents/Projects/grand-blue-engine

# REM Create a deployment virtual environment for windows x64 with Python 3.8
cd "./scripts"
call "./create_and_initialize_env.bat" x64 38 deployment

cmd /k