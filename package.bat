@echo off
setlocal

set DIST_ROOT=dist
set APP_VERSION=dev
if exist VERSION (
    set /p APP_VERSION=<VERSION
)
set PACKAGE_NAME=tetris-win-%APP_VERSION%
set PACKAGE_DIR=%DIST_ROOT%\%PACKAGE_NAME%
set ZIP_FILE=%DIST_ROOT%\%PACKAGE_NAME%.zip
set LATEST_ZIP=%DIST_ROOT%\tetris-win-latest.zip
set CHECKSUM_FILE=%DIST_ROOT%\SHA256SUMS.txt

call build.bat
if errorlevel 1 exit /b 1

if exist "%PACKAGE_DIR%" rmdir /s /q "%PACKAGE_DIR%"
if not exist "%DIST_ROOT%" mkdir "%DIST_ROOT%"
mkdir "%PACKAGE_DIR%"

copy /Y main.exe "%PACKAGE_DIR%\main.exe" >NUL
copy /Y README.md "%PACKAGE_DIR%\README.md" >NUL
copy /Y VERSION "%PACKAGE_DIR%\VERSION" >NUL
copy /Y DEVLOG.md "%PACKAGE_DIR%\DEVLOG.md" >NUL
copy /Y ROADMAP.md "%PACKAGE_DIR%\ROADMAP.md" >NUL

if exist docs (
    xcopy /E /I /Y docs "%PACKAGE_DIR%\docs" >NUL
)

if exist Font (
    xcopy /E /I /Y Font "%PACKAGE_DIR%\Font" >NUL
)

if exist lib\*.dll (
    copy /Y lib\*.dll "%PACKAGE_DIR%\" >NUL
)

if exist "%ZIP_FILE%" del /q "%ZIP_FILE%"
if exist "%LATEST_ZIP%" del /q "%LATEST_ZIP%"
if exist "%CHECKSUM_FILE%" del /q "%CHECKSUM_FILE%"

(
    echo C++ Tetris %APP_VERSION%
    echo.
    echo Build folder: %PACKAGE_NAME%
    echo Executable: main.exe
    echo.
    echo Files:
    dir /B "%PACKAGE_DIR%"
) > "%PACKAGE_DIR%\RELEASE_NOTES.txt"

where powershell >NUL 2>NUL
if not errorlevel 1 (
    powershell -NoProfile -ExecutionPolicy Bypass -Command "Compress-Archive -Path '%PACKAGE_DIR%\*' -DestinationPath '%ZIP_FILE%' -Force"
    if errorlevel 1 exit /b 1
    copy /Y "%ZIP_FILE%" "%LATEST_ZIP%" >NUL
    where certutil >NUL 2>NUL
    if not errorlevel 1 (
        certutil -hashfile "%ZIP_FILE%" SHA256 > "%CHECKSUM_FILE%"
    )
    echo Package finished: %ZIP_FILE%
    echo Latest copy: %LATEST_ZIP%
    exit /b 0
)

echo Package folder finished: %PACKAGE_DIR%
echo Install PowerShell to also create %ZIP_FILE%.
