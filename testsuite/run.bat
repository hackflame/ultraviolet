:: =============================================================================
:: Ultraviolet
::
:: Copyright Kristian Garnét.
:: -----------------------------------------------------------------------------

@echo off
cd /d "%~dp0"

:: Create directories

mkdir converted

mkdir converted\8
mkdir converted\8\16
mkdir converted\8\32

mkdir converted\16
mkdir converted\16\8
mkdir converted\16\32

mkdir converted\32
mkdir converted\32\8
mkdir converted\32\16

:: Convert everything

for %%F in (samples\*.txt) do (
  rem Convert to UTF-8
  utf.exe -out=utf-8 < %%F > converted\8\%%~nxF

  rem Convert from UTF-8 to UTF-16
  utf.exe -out=utf-16 < converted\8\%%~nxF > converted\8\16\%%~nxF

  rem Convert from UTF-8 to UTF-32
  utf.exe -out=utf-32 < converted\8\%%~nxF > converted\8\32\%%~nxF

  rem Convert to UTF-16
  utf.exe -out=utf-16 < %%F > converted\16\%%~nxF

  rem Convert from UTF-16 to UTF-8
  utf.exe -out=utf-8 < converted\16\%%~nxF > converted\16\8\%%~nxF

  rem Convert from UTF-16 to UTF-32
  utf.exe -out=utf-32 < converted\16\%%~nxF > converted\16\32\%%~nxF

  rem Convert to UTF-32
  utf.exe -out=utf-32 < %%F > converted\32\%%~nxF

  rem Convert from UTF-32 to UTF-8
  utf.exe -out=utf-8 < converted\32\%%~nxF > converted\32\8\%%~nxF

  rem Convert from UTF-32 to UTF-16
  utf.exe -out=utf-16 < converted\32\%%~nxF > converted\32\16\%%~nxF
)
