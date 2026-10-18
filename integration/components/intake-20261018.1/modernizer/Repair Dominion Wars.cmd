@echo off
title Repair Dominion Wars Modernizer
powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%~dp0DominionWarsModernizer.ps1" Repair
set "DW_EXIT=%ERRORLEVEL%"
echo.
if not "%DW_EXIT%"=="0" (
    echo Repair did not complete. Review the error above.
) else (
    echo Repair completed successfully.
)
pause
exit /b %DW_EXIT%
