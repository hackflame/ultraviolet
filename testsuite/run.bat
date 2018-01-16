:: =============================================================================
:: Ultraviolet Windows build scripts.
::
:: Test the UTF converter program and the library.
::
:: Copyright Kristian Garnét.
:: -----------------------------------------------------------------------------

@echo off
cd /d "%~dp0"

:: Create directories

mkdir test

mkdir test\8
mkdir test\8\16
mkdir test\8\32

mkdir test\16
mkdir test\16\8
mkdir test\16\32

mkdir test\32
mkdir test\32\8
mkdir test\32\16

:: Convert everything

for %%F in (..\samples\*.txt) do (
  rem Convert to UTF-8
  utf -out=utf-8 < %%F > test\8\%%~nxF

  rem Convert from UTF-8 to UTF-16
  utf -out=utf-16 < test\8\%%~nxF > test\8\16\%%~nxF

  rem Convert from UTF-8 to UTF-32
  utf -out=utf-32 < test\8\%%~nxF > test\8\32\%%~nxF

  rem Convert to UTF-16
  utf -out=utf-16 < %%F > test\16\%%~nxF

  rem Convert from UTF-16 to UTF-8
  utf -out=utf-8 < test\16\%%~nxF > test\16\8\%%~nxF

  rem Convert from UTF-16 to UTF-32
  utf -out=utf-32 < test\16\%%~nxF > test\16\32\%%~nxF

  rem Convert to UTF-32
  utf -out=utf-32 < %%F > test\32\%%~nxF

  rem Convert from UTF-32 to UTF-8
  utf -out=utf-8 < test\32\%%~nxF > test\32\8\%%~nxF

  rem Convert from UTF-32 to UTF-16
  utf -out=utf-16 < test\32\%%~nxF > test\32\16\%%~nxF
)
