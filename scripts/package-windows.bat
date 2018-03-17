
rem On my machine, I execute the build script from a script with these commands:
rem
rem SET QTDIR=C:\Qt\5.10.0\msvc2017_64
rem call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin\amd64\vcvars64.bat"
rem PATH=%PATH%;C:\Program Files (x86)\Windows Kits\10\bin\10.0.16299.0\x64
rem cd "C:\Users\Jarle Aase\src\f-crm\scripts"
rem call .\package-windows

echo on

IF NOT DEFINED DIST_DIR (set DIST_DIR=%cd%\dist\windows)
IF NOT DEFINED BUILD_DIR (set BUILD_DIR=%DIST_DIR%\build)
IF NOT DEFINED SRC_DIR (set SRC_DIR=%cd%\..)
IF NOT DEFINED OUT_DIR (set OUT_DIR=%DIST_DIR%\f-crm)

rmdir /S /Q "%DIST_DIR%"
mkdir "%DIST_DIR%"
mkdir "%BUILD_DIR%"
mkdir "%OUT_DIR%"

pushd "%BUILD_DIR%"

%QTDIR%\bin\qmake.exe ^
  -spec win32-msvc ^
  "CONFIG += release" ^
  "%SRC_DIR%\f-crm.pro"

nmake

popd

echo "Copying: %BUILD_DIR%\release\f_crm.exe" "%OUT_DIR%"
copy "%BUILD_DIR%\release\f-crm.exe" "%OUT_DIR%"
copy "%SRC_DIR%\res\icons\f_crm.ico" "%OUT_DIR%"

%QTDIR%\bin\windeployqt "%OUT_DIR%\f-crm.exe"

echo "The prepared package is in: "%OUT_DIR%"
