@echo off
powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%~dp0Launch-DominionWars.ps1" -DiagnoseOnly
set "DW_EXIT=%ERRORLEVEL%"
pause
exit /b %DW_EXIT%
