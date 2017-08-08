:: =============================================================================
:: Ultraviolet Windows build scripts.
::
:: Clean up the build and testing environment.
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

if exist test (
  del "test\8\*.txt"
  del "test\8\16\*.txt"
  rmdir test\8\16
  del "test\8\32\*.txt"
  rmdir test\8\32
  rmdir test\8

  del "test\16\*.txt"
  del "test\16\8\*.txt"
  rmdir test\16\8
  del "test\16\32\*.txt"
  rmdir test\16\32
  rmdir test\16

  del "test\32\*.txt"
  del "test\32\8\*.txt"
  rmdir test\32\8
  del "test\32\16\*.txt"
  rmdir test\32\16
  rmdir test\32

  rmdir test
)

rem Uncomment to remove the built executable as well
rem del "utf.exe"
