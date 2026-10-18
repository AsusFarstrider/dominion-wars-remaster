@echo off
powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%~dp0Restore-DominionWarsDesktop.ps1" -Manual
set "DW_EXIT=%ERRORLEVEL%"
pause
exit /b %DW_EXIT%
