@echo off
title Remove Dominion Wars Modernizer
echo This removes the compatibility layer and desktop shortcut.
echo The game installation, saves, and user-modified files will remain.
echo.
choice /M "Continue"
if errorlevel 2 exit /b 0
powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%~dp0DominionWarsModernizer.ps1" Uninstall
set "DW_EXIT=%ERRORLEVEL%"
echo.
if not "%DW_EXIT%"=="0" (
    echo Removal did not complete. Review the error above.
) else (
    echo Dominion Wars Modernizer was removed. The game files remain installed.
)
pause
exit /b %DW_EXIT%
