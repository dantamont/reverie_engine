Python distributions are obtained by installing with the windows installer for both 32 and 64 bit versions.

See: https://docs.python.org/3/using/windows.html#embedded-distribution

Also see todo-python-embeded.md in this folder: https://gist.github.com/rgl/72b60fc12f91c93c0d286d56ae19e324

I already do this with setup_python.bat (woo)
See this for more context:
https://stackoverflow.com/questions/49737721/adding-packages-to-python-embedded-installation-for-windows

IN CASE OF ERROR:  ModuleNotFoundError: No module named 'pip._vendor'

-Delete the ./38/my_arch/Lib/site-packages folder
- run .\38\<my_arch>\python.exe -m ensurepip
- run .\38\<my_arch>\python.exe -m pip install upgrade pip