:: =============================================================================
:: Ultraviolet Windows build scripts.
::
:: Copyright Kristian Garnét.
:: -----------------------------------------------------------------------------

@echo off
cd /d "%~dp0"

:: Variables

set MARCH=-march=native
set QUANTUM=%CREATIVE%\QUANTUM
set ULTRAVIOLET=%CREATIVE%\Ultraviolet

:: Create the build directory

if not exist build (
  mkdir build

  if %ERRORLEVEL% neq 0 (
    echo "Build error: couldn't create the build directory."
    exit /b %ERRORLEVEL%
  )
)

:: C++ compatibility test

gcc %MARCH% -I%QUANTUM% -I%ULTRAVIOLET% ^
  -c compat.cpp -o build\compat.o 2> build\compat.log
if %ERRORLEVEL% neq 0 (echo "Build error: compat.cpp" && exit /b %ERRORLEVEL%)
