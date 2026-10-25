@echo off
if "%1"=="--version" (
    echo 0.29.2
    exit /b 0
)
if "%1"=="--exists" (
    exit /b 0
)
if "%1"=="--libs" (
    echo -lpng
    exit /b 0
)
if "%1"=="--modversion" (
    echo 1.9.5
    exit /b 0
)
exit /b 0
