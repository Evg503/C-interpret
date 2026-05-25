::@echo off

setlocal

REM Check if first parameter is "clean"
if "%1"=="clean" (
    echo Cleaning build directory...
    if exist build rmdir /s /q build
)

REM Create build directory if it doesn't exist
if not exist build (
    echo Creating build directory...
    mkdir build
)

REM Run cmake in build directory
cd build
cmake ..
::cmake -DPLATFORM_WINDOWS=ON ..
