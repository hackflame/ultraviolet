:: =============================================================================
:: Ultraviolet
::
:: Copyright Kristian Garnét.
:: -----------------------------------------------------------------------------

@echo off
cd /d "%~dp0"

if exist build (
  rmdir /s /q build
)

if exist converted (
  rmdir /s /q converted
)

del /q utf.exe
