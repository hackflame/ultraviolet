:: =============================================================================
:: Ultraviolet
::
:: Copyright Kristian Garnét.
:: -----------------------------------------------------------------------------

@echo off
cd /d "%~dp0"

if exist build\CMakeFiles (
  rmdir /s /q build
)

if exist converted (
  rmdir /s /q converted
)

del /q bin\utf.exe

rmdir /q bin
