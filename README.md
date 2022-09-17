# Reverie Engine

## Prerequisites
- CMake 3.20+
	- To run cmake, add cmake/bin directory to system path

## Setup
To set things up properly when first pulling the repo:

- Run `C:\Users\dante\Documents\Projects\grand-blue-engine\extern\python\setup_python.bat`
- Run `C:\Users\dante\Documents\Projects\grand-blue-engine\tools\kindler\setup.bat`


For deployment info, see:
https://doc.qt.io/qt-5/windows-deployment.html

For PDBs (very annoying to find), see: https://download.qt.io/online/qtsdkrepository/windows_x86/desktop/qt5_5152/qt.qt5.5152.debug_info.win64_msvc2019_64/

Resolutions to known issues:

1) Missing dlls: https://stackoverflow.com/questions/28732602/qt-example-executables-wont-run-missing-qt5cored-dll
	Can 1) Add the missing dlls using the windows deployment tool, 2) move them manually, or 3) move the dll directory to the system PATH environmental variable