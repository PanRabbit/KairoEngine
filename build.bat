@echo off
setlocal

:: 1. Only configure the project if the build folder doesn't exist yet
if not exist "build\" (
    echo Configuring project for the first time...
    cmake -S . -B build
    echo Configuration complete!
    echo.
)

echo Building project...
:: 2. This command natively detects exactly what changed and compiles incrementally
cmake --build build

:: Check if the build succeeded
if %errorlevel% equ 0 (
    echo.
    echo Done! Launching KairoEngine...
    echo --------------------------------
    :: Adjust the executable path if your build generator puts it in a subdirectory (like Debug/Release)
    .\build\Debug\KairoEngine.exe
) else (
    echo.
    echo Build failed. Check the errors above.
)

endlocal