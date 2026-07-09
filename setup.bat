@echo off
setlocal
cd /d "%~dp0"

echo === Updating submodules ===
git submodule update --init --recursive
if errorlevel 1 goto :error

echo.
echo === Configuring CMake ===
cmake -S . -B build
if errorlevel 1 goto :error

echo.
echo === Building pixey-sample ===
cmake --build build --config Debug --target pixey-sample
if errorlevel 1 goto :error

echo.
echo Done. Run: build\pixey-sample\Debug\pixey-sample.exe
echo.
pause
endlocal
exit /b 0

:error
echo.
echo Setup failed.
echo.
pause
endlocal
exit /b 1
