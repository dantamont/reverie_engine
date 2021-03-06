:: This is an example file to generate binaries using Windows Operating System
:: This script is configured to be executed from the source directory

:: Compiled binaries will be placed in BINARIES_DIR\code\CONFIG

:: NOTE
:: The build process will generate a config.h file that is placed in BINARIES_DIR\include
:: This file must be merged with SOURCE_DIR\include
:: You should write yourself a script that copies the files where you want them.
:: Also see: https://github.com/assimp/assimp/pull/2646

:: Code ends up in \BINARIES\Win32\code\Debug and \BINARIES\Win32\code\Release

SET SOURCE_DIR=.

:: For generators see "cmake --help"
:: SET GENERATOR=Visual Studio 15 2017
SET GENERATOR=Visual Studio 16 2019

:: https://stackoverflow.com/questions/25159635/how-to-set-a-cmake-path-from-command-line
SET BINARIES_DIR="./BINARIES/Win32"
cmake CMakeLists.txt -G "%GENERATOR%" -A Win32 -S %SOURCE_DIR% -B %BINARIES_DIR% -D CMAKE_PREFIX_PATH="C:\\Qt\\5.15\\5.15.2\\msvc2019"
cmake --build %BINARIES_DIR% --config debug
cmake --build %BINARIES_DIR% --config release

SET BINARIES_DIR="./BINARIES/x64"
cmake CMakeLists.txt -G "%GENERATOR%" -A x64  -S %SOURCE_DIR% -B %BINARIES_DIR% -D CMAKE_PREFIX_PATH="C:\\Qt\\5.15\\5.15.2\\msvc2019_64"
cmake --build %BINARIES_DIR% --config debug
cmake --build %BINARIES_DIR% --config release

PAUSE
