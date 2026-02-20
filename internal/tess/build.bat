@echo off
setlocal

set VCPKG_ROOT=C:\Users\Alex\source\repos\vcpkg
set VCPKG_MAX_CONCURRENCY=4

echo [1/2] Configuring CMake (vcpkg will install dependencies automatically)...
cmake -B build -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" -DVCPKG_TARGET_TRIPLET=x64-windows-static -DVCPKG_HOST_TRIPLET=x64-windows-static
if %errorlevel% neq 0 (
    echo ERROR: CMake configure failed
    exit /b 1
)

echo [2/2] Building...
cmake --build build --config Release
if %errorlevel% neq 0 (
    echo ERROR: Build failed
    exit /b 1
)

echo.
echo Build succeeded! Run: build\Release\webcam_ocr.exe
