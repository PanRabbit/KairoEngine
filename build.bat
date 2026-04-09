@echo off
echo Configuring project...
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
echo.
echo Building project...
cmake --build build
echo.
echo Done! Executable is in build/Debug/
pause