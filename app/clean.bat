:: =============================================================================
:: Ultraviolet Windows build scripts.
::
:: Clean up the build environment.
::
:: Copyright Kristian Garnét.
:: -----------------------------------------------------------------------------

@echo off
cd /d "%~dp0"

if exist build (
  del "build\*.o"
  del "build\*.log"

  rmdir build
)

rem Uncomment to remove the built executable as well
rem del "utf.exe"
