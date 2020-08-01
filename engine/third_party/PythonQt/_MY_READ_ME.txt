https://doc.qt.io/archives/qq/qq23-pythonqt.html
https://mevislab.github.io/pythonqt/Features.html
https://sourceforge.net/p/pythonqt/discussion/631393/thread/4d476292dc/

MAKE SURE TO USE THE CORRECT STK (10+)
Need to change to static library (General -> Configuration Type)
Need to change configuration to MDd (C/C++ -> Code Generation -> Runtime library)
https://stackoverflow.com/questions/42801276/error-lnk-2019-unresolved-external-symbol-imp-crtdbgreportw-in-visual-studio
Need to run setup.bat after creating solution
Make sure there are not multiple python3 installations

Error: Fatal Python error: failed to get the Python codec of the filesystem encoding
Solution: PythonPath variable must be set to C:\Users\dante\Documents\Projects\grand-blue-engine\third_party\Python38-32;
C:\Users\dante\Documents\Projects\grand-blue-engine\third_party\Python38-32\DLLs;
C:\Users\dante\Documents\Projects\grand-blue-engine\third_party\Python38-32\Lib;
C:\Users\dante\Documents\Projects\grand-blue-engine\third_party\Python38-32\Lib\site-packages