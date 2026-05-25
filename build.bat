@echo off
setlocal

where g++ >NUL 2>NUL
if not errorlevel 1 (
    g++ -std=c++17 -Wall -Wextra src\*.cpp -mwindows -lwinmm -lgdi32 -luser32 -o main.exe
    if errorlevel 1 exit /b 1
    echo Build finished with g++: main.exe
    exit /b 0
)

where cl >NUL 2>NUL
if not errorlevel 1 (
    cl /EHsc /W4 /std:c++17 /Fe:main.exe src\*.cpp user32.lib gdi32.lib winmm.lib /link /SUBSYSTEM:WINDOWS
    if errorlevel 1 exit /b 1
    echo Build finished with cl: main.exe
    exit /b 0
)

echo Build failed. Install MinGW-w64 g++ or run this from a Visual Studio Developer Command Prompt.
exit /b 1
