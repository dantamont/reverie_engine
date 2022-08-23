The gist is:

1. install the normal python from python.org
2. install everything into a venv
3. copy the venv into python embed.

# A raw example

```bash
unzip python-3.8.0-embed-amd64.zip
cd python-3.8.0-embed-amd64

cat >python38._pth <<EOF
python38.zip
.
# run site.main() automatically:
import site
EOF

mkdir -p Lib/site-packages/example
cat >Lib/site-packages/example/__init__.py <<EOF
def greet(name):
  print(f"Hello {name}!")
EOF

./python.exe -c 'import example; example.greet("Rui")'
```

# A complex example

This will package cloudbase-init into a stand-alone application.

Install python:

```powershell
choco install -y python3 --version 3.8.0
python -m pip install --upgrade pip
```

Create a temporary directory to hold everything:

```powershell
mkdir test
cd test
```

Create an empty venv:

```powershell
python -m venv cbi
.\cbi\Scripts\Activate.ps1
```

Install a package from source (it also includes a native module, for which you need to have VS Build Tools already installed):

```powershell
python -m pip install python-certifi-win32
git clone -b pbr https://github.com/petrutlucian94/PyMI.git
cd PyMI
python -m pip install -r requirements.txt
python setup.py install
python -c 'import mi'
cd ..
```

Install cloudinit-base:

```powershell
git clone -b add-no-cloud https://github.com/rgl/cloudbase-init.git
cd cloudbase-init
python -m pip install -r requirements.txt
python setup.py install
cd ..
```

Now package everything up:

```powershell
cd cbi
# compile all files.
# NB in theory everything is already compiled.
python -m compileall .
# remove unneeded files
dir -Recurse '*.chm' | Remove-Item
# save all the used versions.
python -m pip freeze >requirements.txt
# package into zip files.
pushd Lib\site-packages
rm -ErrorAction SilentlyContinue ..\..\..\cloudbase-init-site-packages.zip
rm -ErrorAction SilentlyContinue ..\..\..\cloudbase-init-site-packages-pyd.zip
7z a -tzip ..\..\..\cloudbase-init-site-packages.zip '-xr!*.pyd' '-x!pip*'
7z l ..\..\..\cloudbase-init-site-packages.zip
7z a -tzip ..\..\..\cloudbase-init-site-packages-pyd.zip '-ir!*.pyd'
7z l ..\..\..\cloudbase-init-site-packages-pyd.zip
popd
cd ..
exit
```

Create the python embedded environment:

```powershell
$url = 'https://www.python.org/ftp/python/3.8.0/python-3.8.0-embed-amd64.zip'
$filename = Split-Path -Leaf $url
(New-Object System.Net.WebClient).DownloadFile($url, "$PWD\$filename")
mkdir dist
cd dist
7z x "..\$filename"
cp ..\cloudbase-init-site-packages.zip site-packages.zip
7z x ..\cloudbase-init-site-packages-pyd.zip -osite-packages
Set-Content -Encoding ascii python38._pth @"
python38.zip
site-packages.zip
site-packages
.
#import site
"@
7z a -tzip ..\dist.zip
cd ..
.\dist\python -c "import sys; print(f'sys.path={sys.path}')"
.\dist\python -m cloudbaseinit.shell --help # XXX this does not work unless the files are outside of site-packages.zip... why?
```
