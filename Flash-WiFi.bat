@echo off
rem Copyright (C) 2026 Steve Madden
rem SPDX-License-Identifier: GPL-3.0-or-later
setlocal
set "CLOCK_PWSH="
where pwsh.exe >nul 2>nul
if not errorlevel 1 set "CLOCK_PWSH=pwsh.exe"
if not defined CLOCK_PWSH if exist "%ProgramFiles%\PowerShell\7\pwsh.exe" set "CLOCK_PWSH=%ProgramFiles%\PowerShell\7\pwsh.exe"
if not defined CLOCK_PWSH (
  echo PowerShell 7 is required. Install it, then double-click this file again.
  echo https://learn.microsoft.com/en-us/powershell/scripting/install/install-powershell-on-windows
  pause
  exit /b 1
)
if not exist "%~dp0tools\Start-WiFiFlash.ps1" (
  echo Extract the complete project ZIP first. Keep this file beside the tools folder.
  pause
  exit /b 1
)
"%CLOCK_PWSH%" -NoLogo -NoProfile -STA -ExecutionPolicy Bypass -File "%~dp0tools\Start-WiFiFlash.ps1"
set "CLOCK_RESULT=%ERRORLEVEL%"
pause
exit /b %CLOCK_RESULT%
