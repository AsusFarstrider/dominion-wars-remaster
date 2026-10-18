@echo off
powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%~dp0Launch-DominionWars.ps1" -FocusMode Experimental %*
set "DW_EXIT=%ERRORLEVEL%"
if not "%DW_EXIT%"=="0" pause
exit /b %DW_EXIT%
