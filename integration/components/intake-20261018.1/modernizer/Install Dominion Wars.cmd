@echo off
title Dominion Wars Modernizer
set "DW_RESULT=%TEMP%\dw-install-%RANDOM%-%RANDOM%-%RANDOM%.success"
if exist "%DW_RESULT%" (
    echo Cannot reserve an installation result file. Please try again.
    pause
    exit /b 1
)
powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%~dp0DominionWarsModernizer.ps1" Install -CompletionMarker "%DW_RESULT%"
set "DW_EXIT=%ERRORLEVEL%"
if not exist "%DW_RESULT%" set "DW_EXIT=1"
if exist "%DW_RESULT%" (
    "%SystemRoot%\System32\findstr.exe" /l /x /c:"Dominion Wars installation completed successfully." "%DW_RESULT%" >nul 2>&1
    if errorlevel 1 set "DW_EXIT=1"
)
if exist "%DW_RESULT%" del /q "%DW_RESULT%"
echo.
if not "%DW_EXIT%"=="0" (
    echo Installation did not complete. A game directory may remain if later compatibility setup failed.
    echo Installation logs: "%LOCALAPPDATA%\Dominion Wars Modernizer\InstallLogs"
) else (
    echo Installation completed successfully.
)
pause
exit /b %DW_EXIT%
