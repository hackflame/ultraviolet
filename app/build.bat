:: =============================================================================
:: Ultraviolet Windows build scripts.
::
:: Build the UTF transcoder.
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

:: Build the object files

gcc -O3 %MARCH% -c -I%QUANTUM% -I%ULTRAVIOLET% ^
  %ULTRAVIOLET%\utf\utf.c -o build\utf.o 2> build\utf.log
if %ERRORLEVEL% neq 0 (echo "Build error: utf\utf.c" && exit /b %ERRORLEVEL%)

gcc -O3 %MARCH% -c -I%QUANTUM% -I%ULTRAVIOLET% ^
  %ULTRAVIOLET%\app\utf.c -o build\app.o 2> build\app.log
if %ERRORLEVEL% neq 0 (echo "Build error: app\app.c" && exit /b %ERRORLEVEL%)

:: Link the executable

gcc -mconsole ^
  build\utf.o build\app.o ^
  -o utf.exe -lmingw32 -lmsvcr120
