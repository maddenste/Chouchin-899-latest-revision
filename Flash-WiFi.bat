@echo off
rem Copyright (C) 2026 Steve Madden
rem SPDX-License-Identifier: GPL-3.0-or-later
setlocal
set "CLOCK_POWERSHELL=%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe"
if exist "%SystemRoot%\Sysnative\WindowsPowerShell\v1.0\powershell.exe" set "CLOCK_POWERSHELL=%SystemRoot%\Sysnative\WindowsPowerShell\v1.0\powershell.exe"
if not exist "%~dp0tools\Start-WiFiFlash.ps1" (
  echo Extract the complete project ZIP first. Keep this file beside the tools folder.
  pause
  exit /b 1
)
"%CLOCK_POWERSHELL%" -NoLogo -NoProfile -STA -ExecutionPolicy Bypass -File "%~dp0tools\Start-WiFiFlash.ps1"
set "CLOCK_RESULT=%ERRORLEVEL%"
pause
exit /b %CLOCK_RESULT%
